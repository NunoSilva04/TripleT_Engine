#ifndef __TRIPLET_GRAPHICS_H__
#define __TRIPLET_GRAPHICS_H__

#include "TripleT_Window.h"
#include "TripleT_Utils.h"
#include <stdbool.h>

typedef enum TripleT_Graphics_Errors{
    TRIPLET_GRAPHICS_ERROR_NONE = 0,
    TRIPLET_GRAPHICS_ERROR_INSTANCE,
    TRIPLET_GRAPHICS_ERROR_SURFACE,
    TRIPLET_GRAPHICS_ERROR_DEVICE,
    TRIPLET_GRAPHICS_ERROR_COMMANDS,
    TRIPLET_GRAPHICS_ERROR_SWAPCHAIN,
    TRIPLET_GRAPHICS_ERROR_IMAGE_VIEW,
    TRIPLET_GRAPHICS_ERROR_SYNC_OBJECTS,
    TRIPLET_GRAPHICS_ERROR_GRAPHICS_PIPELINE,
    TRIPLET_GRAPHICS_ERROR_RESOURCE_MANAGER,
}TripleT_Graphics_Errors;

typedef enum TripleT_Graphics_Image_Type{
    TRIPLET_GRAPHICS_IMAGE_TYPE_UNDEFINED,
    TRIPLET_GRAPHICS_IMAGE_TYPE_COLOR_ATTACHMENTE_OPTIONAL,
    TRIPLET_GRAPHICS_IMAGE_TYPE_PRESENT_SRC,
}TripleT_Graphics_Image_Type;

typedef struct TripleT_Graphics_t TripleT_Graphics;

extern TripleT_Graphics *t3_init_graphics_ex(const TripleT_Window *t3_window, TripleT_Graphics_Errors *t3_graphics_error, bool debug_enabled); 
#define t3_init_graphics(t3_graphics, t3_graphics_error) t3_init_graphics_ex(t3_graphics, t3_graphics_error, false)
#define t3_init_graphics_debug(t3_graphics, t3_graphics_error) t3_init_graphics_ex(t3_graphics, t3_graphics_error, true)
extern void t3_start_synchronization(TripleT_Window *t3_window, TripleT_Graphics *t3_graphics);
extern void t3_barrier_transition(TripleT_Graphics *t3_graphics, const TripleT_Graphics_Image_Type old_image_type, const TripleT_Graphics_Image_Type new_image_type);
extern void t3_begin_rendering(TripleT_Graphics *t3_graphics);
extern void t3_clear_background(TripleT_Graphics *t3_graphics, const TripleT_RGB background_colour);
extern void t3_render_triangle_temp(TripleT_Graphics *t3_graphics);
extern void t3_finish_rendering(TripleT_Graphics *t3_graphics);
extern void t3_present_graphics(TripleT_Graphics *t3_graphics);
extern void t3_destroy_graphics(TripleT_Graphics *t3_graphics);

#endif // __TRIPLET_GRAPHICS_H__
