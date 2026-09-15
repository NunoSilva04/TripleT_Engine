#ifndef __TRIPLET_ENGINE_GRAPHICS_X11_INTERNALS_H__
#define __TRIPLET_ENGINE_GRAPHICS_X11_INTERNALS_H__

#include "TripleT_Graphics.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

typedef struct{
    VkDevice logical_device;
    VkQueue logical_device_queue;
    VkPhysicalDevice physical_device;
    unsigned int queue_index;
    unsigned int num_queues;
    unsigned int num_queues_in_use;
    struct{
	char **names;
	unsigned int count;
    }extensions;
    VkPhysicalDeviceFeatures features;
}Device_Info;

typedef struct{
    unsigned int num_command_pools;
    VkCommandPool *command_pools;
    unsigned int num_command_buffers;
    VkCommandBuffer *command_buffers;
}Commands_Info;

typedef struct{
    VkSwapchainKHR swapchain;
    // Device Surface Capabilites
    unsigned int min_image_count;
    unsigned int max_image_count;
    unsigned int num_images_used;
    VkExtent2D min_image_extent;
    VkExtent2D max_image_extent;
    VkExtent2D curr_image_extend;
    unsigned int max_image_array_layers;
    unsigned int num_image_array_layers_used;
    VkSurfaceTransformFlagsKHR supported_transforms;
    VkSurfaceTransformFlagBitsKHR chosen_transform;
    VkCompositeAlphaFlagsKHR supported_alpha_flags;
    VkCompositeAlphaFlagBitsKHR chosen_alpha_flag;
    VkImageUsageFlags supported_usage_flags;
    VkImageUsageFlags chosen_usage_flags;
    // Device Surface Formats
    VkFormat format;
    VkColorSpaceKHR color_space;
    // Device Surface Present Mode
    VkPresentModeKHR present_mode;
}Swapchain_Info;

typedef struct{
    unsigned int num_images;
    unsigned int num_frames;
    VkImage *images;
    VkImageView *image_view;
    unsigned int image_index;
    unsigned int frame_index;
}Image_Info; 

typedef struct{
    VkFence *fences;
    VkSemaphore *image_available_semaphores;
    VkSemaphore *render_finished_semaphores;
}Sync_Objects_Info;

typedef enum{
    VERTEX_SHADER,
    FRAGMENT_SHADER,
}Shader_Type;

typedef struct{
   struct{
	unsigned int num_shaders;
	struct Shader_Data{
	    VkShaderModule shader_module;
	    VkPipelineShaderStageCreateInfo shader_stage_create_info;
	    Shader_Type shader_type;
	}*shader_data;
    }Shader_Info;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
}Graphics_Pipeline_Info;

typedef struct TripleT_Graphics_t{
    bool debug_enabled;
    VkInstance instance;
    VkSurfaceKHR surface;
    Device_Info device_info; 
    Commands_Info commands_info;
    Swapchain_Info swapchain_info;
    Image_Info image_info;
    Sync_Objects_Info sync_objects_info;
    Graphics_Pipeline_Info graphics_pipeline_info;
}TripleT_Graphics;

#endif // __TRIPLET_ENGINE_GRAPHICS_X11_INTERNALS_H__
