#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

Display* display;
Window window;

#include "volumecontrol.h"
#include "link.h"


int main(int argc, char **argv) {

	daemon(0,0);

	if( argc != 3 ) {
		print_help(argc, argv);
		exit(-1);
	}

	display = XOpenDisplay(0);
	window = RootWindow(display, 0);
	
	char *hostname;
	uint16_t port;

	hostname = strdup(argv[1]);
	port = atoi(argv[2]);

	init_link(hostname, port);
	
	static const int global_modifiers[] = { 0, LockMask, LockMask | Mod2Mask, Mod2Mask };
	int i;

	for(i = 0; i < sizeof(global_modifiers) / sizeof(global_modifiers[0]); ++i)
	{
		int gmod = global_modifiers[i];

		XGrabKey(display, XKeysymToKeycode(display, XK_KP_Add), ControlMask | gmod, window, False, GrabModeAsync, GrabModeAsync);
		XGrabKey(display, XKeysymToKeycode(display, XK_KP_Subtract), ControlMask | gmod, window, False, GrabModeAsync, GrabModeAsync);
		XGrabKey(display, XKeysymToKeycode(display, XK_KP_Multiply), ControlMask | gmod, window, False, GrabModeAsync, GrabModeAsync);
		XGrabKey(display, XKeysymToKeycode(display, XK_KP_Divide), ControlMask | gmod, window, False, GrabModeAsync, GrabModeAsync);
	}

	XEvent event;

	while(0 == XNextEvent(display, &event))
	{
		switch(event.type)
		{
			case KeyPress:

				{
					KeySym key_sym = XLookupKeysym(&event.xkey, 0);

					if(!(event.xkey.state & ControlMask))
						break;
					if(key_sym == XK_KP_Add)
						link_send(VOL_UP);

					if(key_sym == XK_KP_Subtract)
						link_send(VOL_DN);

					if(key_sym == XK_KP_Multiply)
						link_send(MUTE);
					if(key_sym == XK_KP_Divide)
						link_send(NOMUTE);
				}
		}
	}


	cleanup_link();
	free( hostname );

	return 0;
}


void print_help(int argc, char **argv) {
	fprintf(stderr, "USAGE: %s <hostname> <port>\n", argv[0]);
}

/* if( (state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK) */
