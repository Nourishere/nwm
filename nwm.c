 // SPDX-License-Identifier: GPL-2.0-only
 //
 // nwm: A naive X tiling window manager
 // Authored by Nour Nawar <nournawar5@gmail.com>

#include "nwm.h"
#include <stdio.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <stdlib.h>

#define WM_WINDOW_HEIGHT 10

static unsigned int border_width = 1;
static unsigned int window_count = 0;
static Window wm_window;
XWindowAttributes *rootw_attr;
XWindowChanges *w_changes;
Display *main_display;

typedef struct client {
	struct client *next;
	struct client *prev;
	Window window;
	int x, y;
	int width, height;
} client;

client *clients, *head;

// Prototypes
void handle_events(XEvent * next_event);
void map(XMapRequestEvent * map_event);
void unmap(XUnmapEvent * unmap_event);
void configure(XConfigureRequestEvent * configure_event);
void circulate(XCirculateRequestEvent * circulate_event);
int other_wm_handler(Display * disp, XErrorEvent * err);
int main_error_handler(Display * disp, XErrorEvent * err);
void add_client_window(Window window);
void tile(void);
static void map_primary(void);
static void reconfigure(client * client);

int main(int argc, char *argv[])
{
#ifdef DEBUG
	_Xdebug = 1;
#endif
	if (argc > 1) {
		printf("%s: command-line arguemnts currently not supported\n",
		       argv[0]);
		return 1;
	}

	main_display = XOpenDisplay(NULL);

	if (main_display == NULL) {
		puts("nwm: cannot open display");
		return 1;
	}

	check_other_wm();

	setup();

	map_wm_window();

	run();

	if (!XCloseDisplay(main_display))
		return 1;

	return 0;
}

void map_wm_window(void)
{
	int whiteColor = WhitePixel(main_display, DefaultScreen(main_display));
	XGetWindowAttributes(main_display, XDefaultRootWindow(main_display),
			     rootw_attr);

	wm_window =
	    XCreateSimpleWindow(main_display, XDefaultRootWindow(main_display),
				0, 0, rootw_attr->width, WM_WINDOW_HEIGHT, 0, whiteColor,
				whiteColor);
	XSelectInput(main_display, wm_window, StructureNotifyMask);

	XMapRaised(main_display, wm_window);
}

// Loop forever handling events
// Loop around the event queue
void run(void)
{
	XSetErrorHandler(main_error_handler);
	XEvent next_event;

	for (;;) {
		XNextEvent(main_display, &next_event);
		if (next_event.type != 0)
			handle_events(&next_event);
	}
}

void handle_events(XEvent *next_event)
{
	int event = next_event->type;

	if (event == CirculateRequest) {
		circulate(&next_event->xcirculaterequest);
		return;
	} else if (event == ConfigureRequest) {
		configure(&next_event->xconfigurerequest);
		return;
	} else if (event == MapRequest) {	/* Handle mapping requests */
		map(&next_event->xmaprequest);
	} else if (event == UnmapNotify) {
		unmap(&next_event->xunmap);
	} else {
		return;
	}
}

// Map windows in a tiling fashon
void map(XMapRequestEvent *map_event)
{
	add_client_window(map_event->window);
	window_count++;

	if (window_count > 1)
		tile();
	else {			// Map the first window full screen
		clients->x = w_changes->x = 0;
		clients->y = w_changes->y = WM_WINDOW_HEIGHT;
		clients->width = w_changes->width = rootw_attr->width;
		clients->height = w_changes->height = rootw_attr->height - WM_WINDOW_HEIGHT;
		XConfigureWindow(map_event->display, map_event->window,
				 CWX | CWY | CWWidth | CWHeight, w_changes);

		XMapRaised(map_event->display, map_event->window);
	}

	XSync(map_event->display, false);

	return;
}

void unmap(XUnmapEvent *unmap_event)
{
	window_count--;
}

void add_client_window(Window window)
{
	client *new_client = (client *) malloc(sizeof(client));
	if (!new_client)
		return;
	new_client->window = window;
	new_client->next = 0;
	new_client->prev = clients;
	clients->next = new_client;

	clients = new_client;
}

void tile(void)
{
	map_primary();

	client *temp = clients->prev;
	// Reconfigure other windows
	while (temp->prev) {
		reconfigure(temp);
		temp = temp->prev;
	}
}

// Map the newest window on the left side as primary
static void map_primary(void)
{
	int total_width = rootw_attr->width;

	clients->x = w_changes->x = 0;
	clients->y = w_changes->y = WM_WINDOW_HEIGHT;
	clients->width = w_changes->width = total_width >> 1;
	clients->height = w_changes->height = rootw_attr->height - WM_WINDOW_HEIGHT;

	XConfigureWindow(main_display, clients->window,
			 CWX | CWY | CWWidth | CWHeight, w_changes);

	XMapRaised(main_display, clients->window);
}

static void reconfigure(client *client)
{
	int x, y;
	int height, width;

	height = rootw_attr->height / (window_count - 1);
	x = width = rootw_attr->width >> 1;

	// The one right before the primary
	if (client->next->next == 0) {
		y = WM_WINDOW_HEIGHT;
	} else {
		y = client->next->y + height;
	}
	// Update coordinates
	client->y = y;

	XMoveResizeWindow(main_display, client->window, x, y, width, height);
}

void check_other_wm(void)
{
	/* Attempt to acquire the control mask which would fail if there is
	 * another WM running
	 */
	// There is a possiblity we might need ResizeRequestMask, ButtonPressMask
	long mask = SubstructureRedirectMask;

	XSetErrorHandler(other_wm_handler);
	XSelectInput(main_display, XDefaultRootWindow(main_display), mask);
}

int other_wm_handler(Display *disp, XErrorEvent *err)
{
	if (err->error_code == BadAccess) {
		puts("nwm: another WM is running..");
		exit(1);
	}
	return err->error_code;
}

void configure(XConfigureRequestEvent *configure)
{
	Display *disp = configure->display;
	XWindowChanges *changes =
	    (XWindowChanges *) malloc(sizeof(XWindowChanges));

	changes->x = configure->x;
	changes->y = configure->y;
	changes->width = configure->width;
	changes->height = configure->height;
	changes->border_width = border_width;
	changes->sibling = configure->above;
	changes->stack_mode = configure->detail;

	XConfigureWindow(disp, configure->window, configure->value_mask,
			 changes);
	free(changes);
}

// Control the window stack
void circulate(XCirculateRequestEvent *circulate)
{

}

/* This handler should take care of all possible errors we can
 * get from the `run` and `handle` functions
 */
int main_error_handler(Display *disp, XErrorEvent *err)
{
	unsigned char error = err->error_code;

	return 1;
}

void setup(void)
{
	clients = (client *) malloc(sizeof(client));
	clients->next = 0;
	clients->prev = 0;
	head = clients;

	rootw_attr = (XWindowAttributes *) malloc(sizeof(XWindowAttributes));

	w_changes = (XWindowChanges *) malloc(sizeof(XWindowAttributes));

	return;
}

void die(void)
{
	free(w_changes);
	free(rootw_attr);
	client *temp;
	while (head->next != 0) {
		temp = head;
		head = head->next;
		free(temp);
	}

	return;
}
