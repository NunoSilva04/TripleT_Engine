#include "../Internals/TripleT_Engine_X11_Internal.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

static void set_atoms_as_events(const TripleT_Window t3_window){
    Atom wm_delete_window = XInternAtom(t3_window.display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(t3_window.display, t3_window.window, &wm_delete_window, 1);

    return;
}

TripleT_Window *t3_create_window(const TripleT_Window_Properties t3_properties){
    Display *display = XOpenDisplay(NULL);
    if(display == NULL){
	fprintf(stderr, "Couldn't open display\n");
	return NULL;
    }

    Window root_window = DefaultRootWindow(display);
    if(root_window == None){
	fprintf(stderr, "Couldn't get root window\n");
	XCloseDisplay(display);
	return NULL;
    }

    Window window = XCreateSimpleWindow(display, root_window, t3_properties.x, t3_properties.y, t3_properties.width, t3_properties.height, t3_properties.border_width, t3_properties.border_color, t3_properties.background_color);

    if(window == None){
	fprintf(stderr, "Couldn't create simple window\n");
	XCloseDisplay(display);
	return NULL;
    }

    TripleT_Window *t3_window = (TripleT_Window *) malloc(sizeof(TripleT_Window));
    if(t3_window == NULL)
	return NULL;

    t3_window->display = display;
    t3_window->window = window;
    t3_window->properties = t3_properties;
    t3_window->screen_number = DefaultScreen(display);

    XSelectInput(t3_window->display, t3_window->window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | StructureNotifyMask | FocusChangeMask | ExposureMask);
    XMapWindow(t3_window->display, t3_window->window);
    set_atoms_as_events(*t3_window);

    return t3_window;
}

int t3_get_main_screen_width(void){
    Display *display = XOpenDisplay(NULL);
    int screen_number = DefaultScreen(display);

    return DisplayWidth(display, screen_number);
}

int t3_get_main_screen_height(void){
    Display *display = XOpenDisplay(NULL);
    int screen_number = DefaultScreen(display);

    return DisplayHeight(display, screen_number);
}

void t3_print_window_properties(const TripleT_Window *t3_window){
    fprintf(stdout, "Window name: %s\nWindow (X, Y): (%d, %d)\nWindow (Width, Height): (%d, %d)\n", 
	    t3_window->properties.window_name, t3_window->properties.x, t3_window->properties.y, t3_window->properties.width, t3_window->properties.height);    

    return;
}

void t3_destroy_window(TripleT_Window *t3_window){
    XDestroyWindow(t3_window->display, t3_window->window);
    XCloseDisplay(t3_window->display);
    free(t3_window);

    return;
}
