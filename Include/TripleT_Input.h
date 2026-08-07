#ifndef __TRIPLE_T_INPUT_H__
#define __TRIPLE_T_INPUT_H__

#include "TripleT_Window.h"

typedef enum TripleT_Input_Event_Type{
    TRIPLET_INPUT_EVENT_TYPE_NONE = 0,
    // Client Events
    TRIPLET_INPUT_EVENT_TYPE_CLIENT_MESSAGE, 

    // Structure Control Events
    TRIPLET_INPUT_EVENT_TYPE_CONFIGURE_NOTIFY,

    // Keyboard Events
    TRIPLET_INPUT_EVENT_TYPE_KEY_PRESSED,

    // Mouse Events
    TRIPLET_INPUT_EVENT_TYPE_LEFT_MOUSE_BUTTON_PRESSED,
    TRIPLET_INPUT_EVENT_TYPE_MOUSE_MOTION,
}TripleT_Input_Event_Type;

enum{
    TRIPLET_NONE = 0,
    // Client Values
    TRIPLET_DELETE_WINDOW,

    // Keyboard Values
    TRIPLET_A_KEY,
    TRIPLET_B_KEY,
    TRIPLET_C_KEY,
    TRIPLET_D_KEY,
    TRIPLET_E_KEY,

    // Mouse Values
    TRIPLET_LEFT_MOUSE_BUTTON,
    TRIPLET_RIGHT_MOUSE_BUTTON,
    TRIPLET_MOUSE_MOTION,
};

typedef struct TripleT_Client_t{
    int value;
}TripleT_Client;

typedef struct TripleT_Configure_t{
    int new_x, new_y;
    int new_width, new_height;
}TripleT_Configure;

typedef struct TripleT_Keyboard_t{
    int value;
}TripleT_Keyboard;

typedef struct TripleT_Mouse_t{
    int value;
    struct{
	int screen_x, screen_y;
	int window_x, window_y;
    }position;
}TripleT_Mouse;

typedef struct{
    TripleT_Input_Event_Type event_type;
    union{
	TripleT_Client client;
	TripleT_Keyboard keyboard;
	TripleT_Mouse mouse;
	TripleT_Configure configure;
    };
}TripleT_Input;

extern void t3_get_mouse_position_window(const TripleT_Window *t3_window, TripleT_Input *t3_input);
extern void t3_get_mouse_position_screen(const TripleT_Window *t3_window, TripleT_Input *t3_input);
extern void t3_get_tripleT_input(const TripleT_Window *t3_window, TripleT_Input *t3_input);
extern void t3_resize_window(TripleT_Window *t3_window, const TripleT_Configure configure);

#endif // __TRIPLE_T_INPUT_H__
