#include "../../Include/TripleT_Window.h"
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct TripleT_Window_t{
    Display *display;
    Window window;
    Triple_T_Window_Properties properties;
}TripleT_Window;

TripleT_Window *create_window(const Triple_T_Window_Properties t3_properties){
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

    XMapWindow(t3_window->display, t3_window->window);

    return t3_window;
}

void destroy_window(TripleT_Window *t3_window){
    XDestroyWindow(t3_window->display, t3_window->window);
    XCloseDisplay(t3_window->display);
    free(t3_window);

    return;
}
