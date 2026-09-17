#include "TripleT_Resource_Manager.h"
#include "../Internals/TripleT_Engine_Resource_Manager_Internals.h"
#include "../Internals/TripleT_Engine_Graphics_X11_Internals.h"
#include "TripleT_Graphics.h"
#include "TripleT_Utils.h"
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

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
    unsigned int objects_index;
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

void t3_close_resource_manager(TripleT_Graphics *t3_graphics){
    for(unsigned int i = 0; i < t3_resource_manager.max_num_objects; i++){
	if(t3_resource_manager.objects[i].is_created == true){
	    vkDestroyBuffer(t3_graphics->device_info.logical_device, t3_resource_manager.objects[i].object_data.vertex_buffer, NULL);
	    vkFreeMemory(t3_graphics->device_info.logical_device, t3_resource_manager.objects[i].object_data.vertex_buffer_memory, NULL);
	}
    }
    free(t3_resource_manager.objects);

    return;
}

/*
 *
 * RESOURCE MANAGER STATIC FUNCTIONS
 *
 * */

static TripleT_Object_Handle t3_create_triangle_object(const TripleT_Graphics *t3_graphics, const TripleT_Object_Description t3_object_handle_desc, TripleT_Object_Handle_Error *t3_object_handle_error){
    VkBufferCreateInfo vertex_buffer_info = {
	.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	.pNext = NULL,
	.flags = 0,
	.size = sizeof(TripleT_Triangle_3D),
	.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    
    VkResult result = vkCreateBuffer(t3_graphics->device_info.logical_device, &vertex_buffer_info, NULL, &t3_resource_manager.objects[t3_resource_manager.objects_index].object_data.vertex_buffer);
    if(result != VK_SUCCESS){
	*t3_object_handle_error = TRIPLET_OBJECT_HANDLE_ERROR_INVALID_GRAPHICS;
	return TRIPLET_OBJECT_HANDLE_INVALID;
    }

    VkMemoryRequirements memory_requirements = {0};
    vkGetBufferMemoryRequirements(t3_graphics->device_info.logical_device, t3_resource_manager.objects[t3_resource_manager.objects_index].object_data.vertex_buffer, &memory_requirements);
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties = {0};
    vkGetPhysicalDeviceMemoryProperties(t3_graphics->device_info.physical_device, &physical_device_memory_properties);

    VkMemoryPropertyFlags desired_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    unsigned int memory_type_index = 0;
    for(unsigned int i = 0; i < physical_device_memory_properties.memoryTypeCount; i++){
	bool is_compatible = memory_requirements.memoryTypeBits & (1 << i);
	bool has_properties = (physical_device_memory_properties.memoryTypes[i].propertyFlags & desired_flags) == desired_flags;
	if(is_compatible && has_properties){
	    memory_type_index = i;
	    break;
	}
    }

    VkMemoryAllocateInfo memory_allocate_info = {
	.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	.pNext = NULL,
	.allocationSize = memory_requirements.size,
	.memoryTypeIndex = memory_type_index,
    };
    result = vkAllocateMemory(t3_graphics->device_info.logical_device, &memory_allocate_info, NULL, &t3_resource_manager.objects[t3_resource_manager.objects_index].object_data.vertex_buffer_memory);
    if(result != VK_SUCCESS){
	*t3_object_handle_error = TRIPLET_OBJECT_HANDLE_ERROR_INVALID_GRAPHICS;
	return TRIPLET_OBJECT_HANDLE_INVALID;
    }

    vkBindBufferMemory(t3_graphics->device_info.logical_device, t3_resource_manager.objects[t3_resource_manager.objects_index].object_data.vertex_buffer, t3_resource_manager.objects[t3_resource_manager.objects_index].object_data.vertex_buffer_memory, 0);

    void *data;
    vkMapMemory(t3_graphics->device_info.logical_device, t3_resource_manager.objects[t3_resource_manager.objects_index].object_data.vertex_buffer_memory, 0, vertex_buffer_info.size, 0, &data);
    memcpy(data, t3_object_handle_desc.triangle_3d.vertices, sizeof(TripleT_Triangle_3D));
    vkUnmapMemory(t3_graphics->device_info.logical_device, t3_resource_manager.objects[t3_resource_manager.objects_index].object_data.vertex_buffer_memory);

    TripleT_Object_Handle handle = t3_resource_manager.objects_index;
    t3_resource_manager.objects[handle].object_data.vertex_count = 3;
    t3_resource_manager.objects[handle].is_created = true;
    t3_resource_manager.objects_index++;
    t3_resource_manager.num_objects++;
    *t3_object_handle_error = TRIPLET_OBJECT_HANDLE_ERROR_NONE;
    return handle;
}

TripleT_Object_Handle t3_create_object_ex(const TripleT_Graphics *t3_graphics, const TripleT_Object_Description t3_object_handle_desc, TripleT_Object_Handle_Error *t3_object_handle_error){
    if(t3_graphics == NULL)    
	return TRIPLET_OBJECT_HANDLE_ERROR_INVALID_GRAPHICS;

    TripleT_Object_Handle object_handle = 0;
    TripleT_Object_Handle_Error error = 0;

    switch(t3_object_handle_desc.type){
	case TRIPLET_OBJECT_TYPE_TRIANGLE_3D:
	    object_handle = t3_create_triangle_object(t3_graphics, t3_object_handle_desc, &error); 
	    break;

	default:
	    error = TRIPLET_OBJECT_HANDLE_ERROR_INVALID_OBJECT_TYPE; 
	    object_handle = TRIPLET_OBJECT_HANDLE_INVALID;
	    break;
    }

    if(t3_object_handle_error != NULL)
	*t3_object_handle_error = error;
    return object_handle;
}

extern void t3_render_object(const TripleT_Graphics *t3_graphics, TripleT_Object_Handle t3_handle){
    vkCmdBindPipeline(t3_graphics->commands_info.command_buffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, t3_graphics->graphics_pipeline_info.graphics_pipeline); 

    VkViewport viewport = {0};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float) t3_graphics->swapchain_info.curr_image_extend.width;
    viewport.height = (float) t3_graphics->swapchain_info.curr_image_extend.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(t3_graphics->commands_info.command_buffers[0], 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.extent = t3_graphics->swapchain_info.curr_image_extend;
    vkCmdSetScissor(t3_graphics->commands_info.command_buffers[0], 0, 1, &scissor);

    VkBuffer buffer[] = {t3_resource_manager.objects[t3_handle].object_data.vertex_buffer};
    VkDeviceSize offset[] = {0};
    vkCmdBindVertexBuffers(t3_graphics->commands_info.command_buffers[0], 0, 1, buffer, offset);
    vkCmdDraw(t3_graphics->commands_info.command_buffers[0], t3_resource_manager.objects[t3_handle].object_data.vertex_count, 1, 0, 0);

    return;
}
