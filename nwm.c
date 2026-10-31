 // SPDX-License-Identifier: GPL-2.0-only
 //
 // nwm: A naive X tiling window manager
 // Authored by Nour Nawar <nournawar5@gmail.com>

#include "nwm.h"
#include <stdio.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
#ifdef DEBUG
	_Xdebug = 1;
#endif
	if (argc > 1) {
		puts("nwm: command-line arguemnts currently not supported");
		return 1;
	}

	Display *main_display = XOpenDisplay(NULL);

	if (main_display == NULL) {
		puts("nwm: cannot open display");
		return 1;
	}

	check_other_wm(main_display);

	map_wm_window(main_display);

	run(main_display);

	if (!XCloseDisplay(main_display))
		return 1;

	return 0;
}

void map_wm_window(Display *disp)
{
	XEvent e;
	//int blackColor = BlackPixel(disp, DefaultScreen(disp));
	int whiteColor = WhitePixel(disp, DefaultScreen(disp));
	XWindowAttributes *w_attr =
	    (XWindowAttributes *) malloc(sizeof(XWindowAttributes));
	XGetWindowAttributes(disp, XDefaultRootWindow(disp), w_attr);

	Window wm_window =
	    XCreateSimpleWindow(disp, XDefaultRootWindow(disp), 0, 0,
				w_attr->width, 10, 0,
				whiteColor, whiteColor);
	XSelectInput(disp, wm_window, StructureNotifyMask);

	XMapRaised(disp, wm_window);

	free(w_attr);
}

// Loop forever handling events
// Loop around the event queue
void run(Display *disp)
{
	XSetErrorHandler(main_error_handler);
	XEvent next_event;

	for (;;) {
		XNextEvent(disp, &next_event);
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
	} else if (event == MapRequest) {
		XMapWindow(next_event->xmaprequest.display,
			   next_event->xmaprequest.window);
		XSync(next_event->xmaprequest.display, false);
	} else {
		return;
	}
}

void check_other_wm(Display *disp)
{
	/* Attempt to acquire the control mask which would fail if there is
	 * another WM running
	 */
	// There is a possiblity we might need ResizeRequestMask, ButtonPressMask
	long mask = SubstructureRedirectMask;

	XSetErrorHandler(other_wm_handler);
	XSelectInput(disp, XDefaultRootWindow(disp), mask);
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

