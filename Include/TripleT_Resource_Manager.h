#ifndef __TRIPLET_RESOURCE_MANGAER_H__
#define __TRIPLET_RESOURCE_MANGAER_H__

#include "TripleT_Graphics.h"

typedef int TripleT_Object_Handle;
#define TRIPLET_OBJECT_HANDLE_INVALID -1

typedef enum TripleT_Object_Handle_Error{
    TRIPLET_OBJECT_HANDLE_ERROR_NONE = 0,
    TRIPLET_OBJECT_HANDLE_ERROR_INVALID_GRAPHICS = -1,
    TRIPLET_OBJECT_HANDLE_ERROR_INVALID_OBJECT_TYPE = -2,
    TRIPLET_OBJECT_HANDLE_ERROR_MAX_ENUM = 0x7FFFFFFF,
}TripleT_Object_Handle_Error;

typedef enum TripleT_Object_Type{
    TRIPLET_OBJECT_TYPE_TRIANGLE_3D,
}TripleT_Object_Type;

typedef struct TripleT_Object_Description_t{
    TripleT_Object_Type type;
    union{
	TripleT_Triangle_3D triangle_3d;
    };
}TripleT_Object_Description;

extern TripleT_Object_Handle t3_create_object_ex(const TripleT_Graphics *t3_graphics, const TripleT_Object_Description t3_object_handle_desc, TripleT_Object_Handle_Error *t3_object_handle_error);
#define t3_create_object(t3_graphics, t3_object_handle_desc) \
    t3_create_object_ex(t3_graphics, t3_object_handle_desc, NULL)
extern void t3_render_object(const TripleT_Graphics *t3_graphics, TripleT_Object_Handle t3_handle);

#endif // __TRIPLET_RESOURCE_MANGAER_H__
