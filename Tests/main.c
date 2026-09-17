#include <TripleT_Engine/TripleT_Utils.h>
#include <TripleT_Engine/TripleT_Window.h>
#include <TripleT_Engine/TripleT_Input.h>
#include <TripleT_Engine/TripleT_Graphics.h>
#include <TripleT_Engine/TripleT_Resource_Manager.h>
#include <stdint.h>
#include <stdlib.h>
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

    TripleT_Graphics_Errors t3_graphics_error;
    TripleT_Graphics *t3_graphics = t3_init_graphics_debug(t3_window, &t3_graphics_error);
    if(t3_graphics == NULL)
	printf("Graphics Error = %d", t3_graphics_error);

    TripleT_Vertex_3D vertices[3] = {
	[0] = {
	    .position = {.x = 0.0f, .y = -0.5f, .z = 0.0f},
	    .color = {.r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f},
	},
	[1] = {
	    .position = {.x = 0.5f, .y = 0.5f, .z = 0.0f},
	    .color = {.r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f},
	},
	[2] = {
	    .position = {.x = -0.5f, .y = 0.5f, .z = 0.0f},
	    .color = {.r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f},
	}
    };
    TripleT_Triangle_3D triangle = {
	.vertice_1 = vertices[0],
	.vertice_2 = vertices[1],
	.vertice_3 = vertices[2],
    };
    TripleT_Object_Description triangle_object_description = {
	.type = TRIPLET_OBJECT_TYPE_TRIANGLE_3D,
	.triangle_3d = triangle,
    };
    TripleT_Object_Handle_Error error = 0;
    TripleT_Object_Handle triangle_handle = t3_create_object_ex(t3_graphics, triangle_object_description, &error);
    if(triangle_handle == TRIPLET_OBJECT_HANDLE_INVALID || error == TRIPLET_OBJECT_HANDLE_ERROR_INVALID_GRAPHICS){
	printf("Error");
	exit(EXIT_FAILURE);
    };

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
	t3_start_synchronization(t3_window, t3_graphics);
	t3_barrier_transition(t3_graphics, TRIPLET_GRAPHICS_IMAGE_TYPE_UNDEFINED, TRIPLET_GRAPHICS_IMAGE_TYPE_COLOR_ATTACHMENTE_OPTIONAL);
	t3_begin_rendering(t3_graphics);
	t3_clear_background(t3_graphics, (TripleT_RGB){.r = 0.1f, .g = 0.2f, .b = 0.3f, .a = 0.0f});
	t3_render_object(t3_graphics, triangle_handle);
	// t3_render_triangle_temp(t3_graphics);
	t3_finish_rendering(t3_graphics);
	t3_barrier_transition(t3_graphics, TRIPLET_GRAPHICS_IMAGE_TYPE_COLOR_ATTACHMENTE_OPTIONAL, TRIPLET_GRAPHICS_IMAGE_TYPE_PRESENT_SRC);
	t3_present_graphics(t3_graphics);
    }

    t3_destroy_graphics(t3_graphics);
    t3_destroy_window(t3_window);
    printf("Done with loop");
    return 0;
}
