#ifndef __TRIPLET_GRAPHICS_H__
#define __TRIPLET_GRAPHICS_H__

#include "TripleT_Window.h"
#include <stdbool.h>

typedef enum TripleT_Graphics_Errors{
    TRIPLET_GRAPHICS_ERROR_NONE = 0,
    TRIPLET_GRAPHICS_ERROR_INSTANCE,
    TRIPLET_GRAPHICS_ERROR_SURFACE,
    TRIPLET_GRAPHICS_ERROR_DEVICE,
    TRIPLET_GRAPHICS_ERROR_COMMANDS,
    TRIPLET_GRAPHICS_ERROR_SWAPCHAIN,
    TRIPLET_GRAPHICS_ERROR_RENDER_PASS,
    TRIPLET_GRAPHICS_ERROR_IMAGE_VIEW,
    TRIPLET_GRAPHICS_ERROR_FRAME_BUFFER, 
    TRIPLET_GRAPHICS_ERROR_SYNC_OBJECTS,
}TripleT_Graphics_Errors;

typedef struct TripleT_Graphics_t TripleT_Graphics;

extern TripleT_Graphics *t3_init_graphics(const TripleT_Window *t3_window, TripleT_Graphics_Errors *t3_graphics_error); 
extern void t3_temp_render_fn(TripleT_Graphics *t3_graphics);
extern void t3_destroy_graphics(TripleT_Graphics *t3_graphics);

#endif // __TRIPLET_GRAPHICS_H__
