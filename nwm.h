 // SPDX-License-Identifier: GPL-2.0-only
 //
 // nwm: A naive X tiling window manager
 // Authored by Nour Nawar <nournawar5@gmail.com>

#ifndef __NWM_H
#define __NWM_H

#include <X11/Xlib.h>

// Prototypes
void check_other_wm(Display * disp);
void map_wm_window(Display * disp);
void run(Display * disp);
void handle_events(XEvent * next_event);
void configure(XConfigureRequestEvent * configure);
void circulate(XCirculateRequestEvent * circulate);

// Handlers
int other_wm_handler(Display * disp, XErrorEvent * err);
int main_error_handler(Display * disp, XErrorEvent * err);

#endif				// __NWM_H
