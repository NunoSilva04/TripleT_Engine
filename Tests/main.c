#include <TripleT_Engine/TripleT_Window.h>
#include <TripleT_Engine/TripleT_Input.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

int main(void){
    TripleT_Window_Properties properties = {0};
    strcpy(properties.window_name, "First Window");
    properties.x = 0;
    properties.y = 0;
    properties.width = 800;
    properties.height = 600;
    properties.border_width = 30;
    properties.border_color = 0;
    properties.background_color = 0xFFFFFFFF;

    TripleT_Window *t3_window = t3_create_window(properties);
    int number = 0;
    for(int i = 0; i < 1000000000; i++)
	number++;
    printf("Done\n");

    bool running = true;
    while(running){
	TripleT_Input t3_input = {0};
	t3_get_tripleT_input(t3_window, &t3_input);
	switch(t3_input.event_type){
	    case TRIPLET_INPUT_EVENT_TYPE_CLIENT_MESSAGE:
		if(t3_input.client.value == TRIPLET_DELETE_WINDOW)
		    running = false;
		break;

	    case TRIPLET_INPUT_EVENT_TYPE_KEY_PRESSED:
		if(t3_input.keyboard.value == TRIPLET_A_KEY)
		    printf("Pressed A key\n");
		break;

	    case TRIPLET_INPUT_EVENT_TYPE_MOUSE_MOTION:
		printf("Mouse coordinates: (%d, %d)\n", t3_input.mouse.position.window_x, t3_input.mouse.position.window_y);
		break;

	    case TRIPLET_INPUT_EVENT_TYPE_CONFIGURE_NOTIFY:
		printf("Configure Notify\n");
		t3_resize_window(t3_window, t3_input.configure);
		break;
	}
    }

    t3_print_window_properties(t3_window);      
    printf("Done with loop");
    t3_destroy_window(t3_window);
    return 0;
}
