#ifndef __TRIPLET_RESOURCE_MANGAER_H__
#define __TRIPLET_RESOURCE_MANGAER_H__

#include "TripleT_Graphics.h"
#include "TripleT_Utils.h"

typedef int TripleT_Object_Handle;
#define TRIPLET_OBJECT_HANDLE_INVALID -1

typedef enum TripleT_Object_Handle_Error{
    TRIPLET_OBJECT_HANDLE_ERROR_NONE = 0,
    TRIPLET_OBJECT_HANDLE_ERROR_INVALID_GRAPHICS = -1,
    TRIPLET_OBJECT_HANDLE_ERROR_INVALID_OBJECT_TYPE = -2,
    TRIPLET_OBJECT_HANDLE_ERROR_MAX_OBJECTS_CREATED = -3,
    TRIPLET_OBJECT_HANDLE_ERROR_MAX_ENUM = 0x7FFFFFFF,
}TripleT_Object_Handle_Error;

typedef enum TripleT_Object_Type{
    TRIPLET_OBJECT_TYPE_TRIANGLE_3D,
    TRIPLET_OBJECT_TYPE_CAMERA_3D,
}TripleT_Object_Type;

typedef struct TripleT_Object_Camera_Description{
    TripleT_Point_3D position;
    TripleT_Point_3D target;
    float FOV;
    float aspect_ratio;
    float near_plane;
    float far_plane;
    TripleT_Matrix_4D mat_view;
    TripleT_Matrix_4D mat_projection;
}TripleT_Object_Camera_Description;

typedef struct TripleT_Object_Description_t{
    TripleT_Object_Type type;
    union{
	TripleT_Triangle_3D triangle_3D;
	TripleT_Object_Camera_Description camera_3D;
    };
}TripleT_Object_Description;

// Object Creation Helper Functions
extern TripleT_Matrix_4D make_mat_view_collumn_major(const TripleT_Object_Camera_Description camera_description);
extern TripleT_Matrix_4D make_mat_projection_collumn_major(const TripleT_Object_Camera_Description camera_description);

extern TripleT_Object_Handle t3_create_object_ex(const TripleT_Graphics *t3_graphics, const TripleT_Object_Description t3_object_handle_desc, TripleT_Object_Handle_Error *t3_object_handle_error);
#define t3_create_object(t3_graphics, t3_object_handle_desc) \
    t3_create_object_ex(t3_graphics, t3_object_handle_desc, NULL)
extern void t3_render_object(const TripleT_Graphics *t3_graphics, TripleT_Object_Handle t3_handle);

#endif // __TRIPLET_RESOURCE_MANGAER_H__
