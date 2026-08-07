#include "../../Include/TripleT_Input.h"
#include "../../Include/Internals/TripleT_X11_Internal.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <string.h>

static int t3_get_keyboard_value(char *keycode_string){
    if(strcmp(keycode_string, "a") == 0){
	return TRIPLET_A_KEY;
    }
    else if(strcmp(keycode_string, "b") == 0){
	return TRIPLET_B_KEY;
    }
    else if(strcmp(keycode_string, "c") == 0){
	return TRIPLET_C_KEY;
    }
    else if(strcmp(keycode_string, "d") == 0){
	return TRIPLET_D_KEY;
    }
    else if(strcmp(keycode_string, "e") == 0){
	return TRIPLET_E_KEY;
    }

    return TRIPLET_NONE;
}

void t3_get_mouse_position_window(const TripleT_Window *t3_window, TripleT_Input *t3_input){
    Window window_returned;
    int root_x, root_y, wind_x, wind_y;
    unsigned int return_mask;

    XQueryPointer(t3_window->display, t3_window->window, &window_returned, &window_returned, &root_x, &root_y, &wind_x, &wind_y, &return_mask); 

    t3_input->mouse.position.window_x = wind_x;
    t3_input->mouse.position.window_y = wind_y;
    t3_input->mouse.position.screen_x = root_x;
    t3_input->mouse.position.screen_y = root_y;

    return;
}

void t3_get_mouse_position_screen(const TripleT_Window *t3_window, TripleT_Input *t3_input){
    Window window_returned;
    int root_x, root_y, wind_x, wind_y;
    unsigned int return_mask;
    Window root_window = XRootWindow(t3_window->display, t3_window->screen_number);

    XQueryPointer(t3_window->display, root_window, &window_returned, &window_returned, &root_x, &root_y, &wind_x, &wind_y, &return_mask); 

    t3_input->mouse.position.window_x = wind_x;
    t3_input->mouse.position.window_y = wind_y;
    t3_input->mouse.position.screen_x = root_x;
    t3_input->mouse.position.screen_y = root_y;

    return;
}

void t3_get_tripleT_input(const TripleT_Window *t3_window, TripleT_Input *t3_input){
    XEvent event;
    XNextEvent(t3_window->display, &event);

    switch(event.type){
	case ClientMessage:
	    t3_input->event_type = TRIPLET_INPUT_EVENT_TYPE_CLIENT_MESSAGE;
	    t3_input->client.value = TRIPLET_DELETE_WINDOW;

	    break;

	case KeyPress:
	    t3_input->event_type = TRIPLET_INPUT_EVENT_TYPE_KEY_PRESSED;
	    KeySym key = XkbKeycodeToKeysym(t3_window->display, event.xkey.keycode, 0, 0); 
	    t3_input->keyboard.value = t3_get_keyboard_value(XKeysymToString(key));

	    break;

	case MotionNotify:
	    t3_input->event_type = TRIPLET_INPUT_EVENT_TYPE_MOUSE_MOTION;
	    t3_input->mouse.value = TRIPLET_MOUSE_MOTION;
	    t3_get_mouse_position_window(t3_window, t3_input);

	    break;
	
	case ConfigureNotify:
	    t3_input->event_type = TRIPLET_INPUT_EVENT_TYPE_CONFIGURE_NOTIFY;
	    t3_input->configure.new_x = event.xconfigure.x;
	    t3_input->configure.new_y = event.xconfigure.y;
	    t3_input->configure.new_width = event.xconfigure.width;
	    t3_input->configure.new_height = event.xconfigure.height;

	    break;
    }

    return;
}

void t3_resize_window(TripleT_Window *t3_window, const TripleT_Configure configure){
    t3_window->properties.x = configure.new_x; 
    t3_window->properties.y = configure.new_y;
    t3_window->properties.width = configure.new_width;
    t3_window->properties.height = configure.new_height;
    
    return;
}
