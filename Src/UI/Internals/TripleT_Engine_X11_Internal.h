#ifndef __TRIPLET_X11_INTERNAL_H__
#define __TRIPLET_X11_INTERNAL_H__

#include "TripleT_Window.h"
#include <X11/Xlib.h>

typedef struct TripleT_Window_t{
    Display *display;
    Window window;
    int screen_number;
    TripleT_Window_Properties properties;
}TripleT_Window;

#endif // __TRIPLET_X11_INTERNAL_H__
