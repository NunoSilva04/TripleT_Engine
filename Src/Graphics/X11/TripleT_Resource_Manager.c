#include "TripleT_Resource_Manager.h"
#include "../Internals/TripleT_Engine_Resource_Manager_Internals.h"
#include <vulkan/vulkan_core.h>
#include <stdlib.h>

typedef struct{
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;
    unsigned int vertex_count;
}TripleT_Object_Data;

typedef struct{
    bool is_created;
    TripleT_Object_Data object_data;
}TripleT_Object;

typedef struct TripleT_Resource_Manager_t{
    TripleT_Object *objects;
    unsigned int num_objects;
    unsigned int max_num_objects;
}TripleT_Resource_Manager;
static TripleT_Resource_Manager t3_resource_manager = {0};

/*
 *
 * RESOURCE MANAGER INTERNAL FUNCTIONS
 *
 * */

bool t3_init_resource_manager(void){
    t3_resource_manager.max_num_objects = TRIPLET_RESOURCE_MANAGER_MAX_NUM_OBJECTS;
    t3_resource_manager.objects = (TripleT_Object *) calloc(TRIPLET_RESOURCE_MANAGER_MAX_NUM_OBJECTS, sizeof(TripleT_Object));
    
    return true;
} 

void t3_close_resource_manager(void){
    for(unsigned int i = 0; i < t3_resource_manager.num_objects; i++){
	if(t3_resource_manager.objects[i].is_created == true){
	    // TODO: Free the data;
	}
    }
    free(t3_resource_manager.objects);

    return;
}

TripleT_Object_Handle t3_create_object_ex(const TripleT_Graphics *t3_graphics, const TripleT_Object_Description t3_object_handle_desc, TripleT_Object_Handle_Error *t3_object_handle_error){
    


}
