#include "TripleT_Graphics.h"
#include "../Internals/TripleT_Engine_Graphics_X11_Internals.h"
#include "../../UI/Internals/TripleT_Engine_X11_Internal.h"
#include "TripleT_Utils.h"
#include "TripleT_Window.h"
#include "../Internals/TripleT_Engine_Resource_Manager_Internals.h"
#include "../Internals/T3_Vertex_Shader.h"
#include "../Internals/T3_Fragment_Shader.h"
#include <X11/Xlib.h>
#include <stddef.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_xlib.h>
#include <stdlib.h>
#include <string.h>

static unsigned int t3_x11_num_extensions = 2;
static const char *const t3_x11_extension_names[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
};

// Graphics Creation
static bool t3_init_instance(TripleT_Graphics *t3_graphics);
static bool t3_init_surface(const TripleT_Window *t3_window, TripleT_Graphics *t3_graphics);
static bool t3_get_best_device(const VkInstance instance, Device_Info *device_info);
static bool t3_choose_queue_family(Device_Info *device_info, VkQueueFlags queue_family_flag, unsigned int num_queues_desired);
static bool t3_get_device_extensions(Device_Info *device_info);
static bool t3_is_device_supported(const Device_Info device_info, VkSurfaceKHR surface);
static bool t3_init_device_get_queue_handle(TripleT_Graphics *t3_graphics);
static bool t3_create_command_pool_buffer(TripleT_Graphics *t3_graphics, uint32_t num_command_pools, uint32_t num_command_buffers);
static bool t3_get_swapchain_surface_capabilities(TripleT_Graphics *t3_graphics);
static bool t3_get_swapchain_device_formats(TripleT_Graphics *t3_graphics);
static bool t3_get_swapchain_present_mode(TripleT_Graphics *t3_graphics);
static bool t3_init_swapchain(TripleT_Graphics *t3_graphics);
static bool t3_init_image_views(TripleT_Graphics *t3_graphics);
static bool t3_create_sync_objects(TripleT_Graphics *t3_graphics);
static bool t3_create_shader(VkDevice logical_device, struct Shader_Data *shader_data, unsigned int *code, const unsigned int code_size, Shader_Type shader_type);
static VkPipelineVertexInputStateCreateInfo create_vertex_input_state_info(void);
static VkPipelineInputAssemblyStateCreateInfo create_assembly_state_create_info(VkPrimitiveTopology topology);
static VkPipelineViewportStateCreateInfo create_viewport_state_create_info(const VkExtent2D curr_image_extent);
static VkPipelineRasterizationStateCreateInfo create_rasterizer_state_info(bool enable_clamp, bool enable_rasterizer_discard, VkPolygonMode polygon_mode, VkCullModeFlags cull_flags, VkFrontFace face_orientation, float line_width);
static VkPipelineMultisampleStateCreateInfo create_multisample_state_create_info(uint32_t num_samples, bool enable_shading, float min_sample_shading_fraction);
static VkPipelineColorBlendStateCreateInfo create_color_blend_state_create_info(bool enable_color_blending);
static VkPipelineDynamicStateCreateInfo create_dynamic_state_create_info(bool tesselation_enabled, bool depth_stencil_enabled);
static bool create_graphics_pipeline_layout(const VkDevice logical_device, VkPipelineLayout *pipeline_layout);
static bool t3_create_graphics_pipeline(TripleT_Graphics *t3_graphics);

// Graphics Rendering
static void t3_recreate_swapchain(TripleT_Graphics *t3_graphics);

// Graphics Destruction
static void t3_destroy_graphics_pipeline(TripleT_Graphics *t3_graphics);
static void t3_destroy_sync_objects(TripleT_Graphics *t3_graphics);
static void t3_destroy_image_views(TripleT_Graphics *t3_graphics);
static void t3_destroy_swapchain(TripleT_Graphics *t3_graphics);
static void t3_destroy_command_pool_buffer(TripleT_Graphics *t3_graphics);
static void t3_destroy_device(TripleT_Graphics *t3_graphics);
static void t3_destroy_surface(TripleT_Graphics *t3_graphics);
static void t3_destroy_instance(TripleT_Graphics *t3_graphics);

TripleT_Graphics *t3_init_graphics_ex(const TripleT_Window *t3_window, TripleT_Graphics_Errors *t3_graphics_error, bool debug_enabled){
    TripleT_Graphics *t3_graphics = (TripleT_Graphics *) calloc(1, sizeof(TripleT_Graphics));
    if(t3_graphics == NULL)
	return NULL;
    t3_graphics->debug_enabled = debug_enabled;
    
    // Initalizing Graphics
    if(!t3_init_instance(t3_graphics)){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_INSTANCE;
	return NULL;
    }
    if(!t3_init_surface(t3_window, t3_graphics)){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_SURFACE;
	return NULL;
    }
    if(!t3_init_device_get_queue_handle(t3_graphics)){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_DEVICE;
	return NULL;
    }
    if(!t3_create_command_pool_buffer(t3_graphics, 1, 1)){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_COMMANDS;
	return NULL;
    }
    if(!t3_init_swapchain(t3_graphics)){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_SWAPCHAIN;
	return NULL;
    }
    if(!t3_init_image_views(t3_graphics)){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_IMAGE_VIEW;
	return NULL;
    }
    if(!t3_create_sync_objects(t3_graphics)){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_SYNC_OBJECTS;
	return NULL;
    }
    if(!t3_create_graphics_pipeline(t3_graphics)){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_GRAPHICS_PIPELINE;
	return NULL;
    }
    if(!t3_init_resource_manager()){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_RESOURCE_MANAGER;
    }

    return t3_graphics;
}

void t3_start_synchronization(TripleT_Window *t3_window, TripleT_Graphics *t3_graphics){
    if(t3_window->resized == true){
	t3_recreate_swapchain(t3_graphics);
	t3_window->resized = false;
    }

    vkWaitForFences(t3_graphics->device_info.logical_device, 1, &t3_graphics->sync_objects_info.fences[0], VK_TRUE, UINT64_MAX);

    vkAcquireNextImageKHR(t3_graphics->device_info.logical_device, t3_graphics->swapchain_info.swapchain, UINT64_MAX, t3_graphics->sync_objects_info.image_available_semaphores[t3_graphics->image_info.frame_index], VK_NULL_HANDLE, &t3_graphics->image_info.image_index);

    vkResetFences(t3_graphics->device_info.logical_device, 1, &t3_graphics->sync_objects_info.fences[0]);

    vkResetCommandBuffer(t3_graphics->commands_info.command_buffers[0], 0);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(t3_graphics->commands_info.command_buffers[0], &begin_info);

    return;
}

void t3_barrier_transition(TripleT_Graphics *t3_graphics, const TripleT_Graphics_Image_Type old_image_type, const TripleT_Graphics_Image_Type new_image_type){
    VkImageMemoryBarrier2 image_barrier = {0};
    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    image_barrier.pNext = NULL;
    switch(old_image_type){
	case TRIPLET_GRAPHICS_IMAGE_TYPE_UNDEFINED:
	    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
	    image_barrier.srcAccessMask = VK_ACCESS_2_NONE;
	    break;

	case TRIPLET_GRAPHICS_IMAGE_TYPE_COLOR_ATTACHMENTE_OPTIONAL:
	    image_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	    image_barrier.srcAccessMask = VK_ACCESS_2_NONE;
	    break;

	case TRIPLET_GRAPHICS_IMAGE_TYPE_PRESENT_SRC:
	    image_barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
	    image_barrier.srcAccessMask = VK_ACCESS_2_NONE;
	    break;
    }

    switch(new_image_type){
	case TRIPLET_GRAPHICS_IMAGE_TYPE_COLOR_ATTACHMENTE_OPTIONAL:
	    image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	    image_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	    break;

	case TRIPLET_GRAPHICS_IMAGE_TYPE_PRESENT_SRC:
	    image_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
	    image_barrier.dstAccessMask = VK_ACCESS_2_NONE;
	    break;
    }
    image_barrier.subresourceRange = (VkImageSubresourceRange){
	.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	.baseMipLevel = 0,
	.levelCount = 1,
	.baseArrayLayer = 0,
	.layerCount = 1,
    };
    image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.image = t3_graphics->image_info.images[t3_graphics->image_info.image_index];

    VkDependencyInfo dependency_info = {
	.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
	.pNext = NULL,
	.imageMemoryBarrierCount = 1,
	.pImageMemoryBarriers = &image_barrier,
    };

    vkCmdPipelineBarrier2(t3_graphics->commands_info.command_buffers[0], &dependency_info);

    return;
}

void t3_begin_rendering(TripleT_Graphics *t3_graphics){
    VkRenderingAttachmentInfo color_attachment_info = {
	.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
	.pNext = NULL,
	.imageView = t3_graphics->image_info.image_view[t3_graphics->image_info.image_index],
	.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };

    VkRenderingInfo rendering_info = {
	.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
	.pNext = NULL,
	.renderArea = {
	    .offset = {0, 0},
	    .extent = t3_graphics->swapchain_info.curr_image_extend,
	},
	.layerCount = 1,
	.viewMask = 0,
	.colorAttachmentCount = 1,
	.pColorAttachments = &color_attachment_info,
	.pDepthAttachment = NULL,
	.pStencilAttachment = NULL,
    };

    vkCmdBeginRendering(t3_graphics->commands_info.command_buffers[0], &rendering_info);

    return;
}

void t3_clear_background(TripleT_Graphics *t3_graphics, const TripleT_RGB background_colour){
    VkClearAttachment clear_attachment = {
	.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	.colorAttachment = 0,
	.clearValue = {
	    .color.float32 = {
		background_colour.r,
		background_colour.g,
		background_colour.b,
		background_colour.a,
	    },
	},
    };

    VkClearRect clear_rect = {
	.rect = {
	    .offset = {0, 0},
	    .extent = t3_graphics->swapchain_info.curr_image_extend,
	},
	.baseArrayLayer = 0,
	.layerCount = 1,
    };

    vkCmdClearAttachments(t3_graphics->commands_info.command_buffers[0], 1, &clear_attachment, 1, &clear_rect);

    return;
}

void t3_render_triangle_temp(TripleT_Graphics *t3_graphics){
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

    vkCmdDraw(t3_graphics->commands_info.command_buffers[0], 3, 1, 0, 0);

    return;
}

void t3_finish_rendering(TripleT_Graphics *t3_graphics){
    vkCmdEndRendering(t3_graphics->commands_info.command_buffers[0]);

    return;
}

void t3_present_graphics(TripleT_Graphics *t3_graphics){
    vkEndCommandBuffer(t3_graphics->commands_info.command_buffers[0]);
    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &t3_graphics->sync_objects_info.image_available_semaphores[t3_graphics->image_info.frame_index];
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &t3_graphics->commands_info.command_buffers[0];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &t3_graphics->sync_objects_info.render_finished_semaphores[t3_graphics->image_info.image_index];

    vkQueueSubmit(t3_graphics->device_info.logical_device_queue, 1, &submit_info, t3_graphics->sync_objects_info.fences[0]);

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &t3_graphics->sync_objects_info.render_finished_semaphores[t3_graphics->image_info.image_index];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &t3_graphics->swapchain_info.swapchain;
    present_info.pImageIndices = &t3_graphics->image_info.image_index;

    vkQueuePresentKHR(t3_graphics->device_info.logical_device_queue, &present_info);
    t3_graphics->image_info.frame_index = (t3_graphics->image_info.frame_index + 1) % t3_graphics->image_info.num_frames;

    return;
}

void t3_destroy_graphics(TripleT_Graphics *t3_graphics){
    vkDeviceWaitIdle(t3_graphics->device_info.logical_device);
    t3_close_resource_manager(t3_graphics);
    t3_destroy_graphics_pipeline(t3_graphics);
    t3_destroy_sync_objects(t3_graphics);
    t3_destroy_image_views(t3_graphics);
    t3_destroy_swapchain(t3_graphics);
    t3_destroy_command_pool_buffer(t3_graphics);
    t3_destroy_device(t3_graphics);
    t3_destroy_surface(t3_graphics);
    t3_destroy_instance(t3_graphics);
    free(t3_graphics); 

    return;
}


/*
 *
 * 	HELPER FUNCTIONS
 *
 **/
#include <stdio.h>

static VkDebugUtilsMessengerEXT messenger = NULL;

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
	VkDebugUtilsMessageTypeFlagsEXT             message_types,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void*                                       pUserData)
{
    (void) pUserData;  // This is just because i am getting annoyed at the warning generated when compiled for the unused parameter
    printf("Severity: %d\n", message_severity);
    printf("Type: %d\n", message_types);
    printf("Message Id: %d\n", pCallbackData->messageIdNumber);
    printf("Message: %s\n", pCallbackData->pMessage);
    printf("Object count: %d\n\n", pCallbackData->objectCount);
    for(uint32_t i = 0; i < pCallbackData->objectCount; i++){
	VkDebugUtilsObjectNameInfoEXT object = pCallbackData->pObjects[i];
	if(object.pObjectName == NULL) object.pObjectName = "<unnamed>";
	printf("\tObj[%d]: type = %d, handle = 0x%llu, name = %s\n\n",
		i,
		object.objectType,
		(unsigned long long)object.objectHandle,
		object.pObjectName);
    }

    return VK_FALSE;
}

static void t3_init_debug_messenger(const VkInstance instance){
    VkDebugUtilsMessengerCreateInfoEXT messenger_info = {0};
    messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messenger_info.pNext = NULL;
    messenger_info.flags = 0;
    messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; 
    messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messenger_info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    messenger_info.pfnUserCallback = debug_callback;
    messenger_info.pUserData = NULL;

    PFN_vkCreateDebugUtilsMessengerEXT create_debug_messenger = NULL;
    create_debug_messenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if(!create_debug_messenger){
        printf("Couldn't create debug messenger\n");
        exit(EXIT_FAILURE);
    }
    
    if(create_debug_messenger(instance, &messenger_info, NULL, &messenger) != VK_SUCCESS) 
        exit(EXIT_FAILURE);
    
    //test function 
    PFN_vkSubmitDebugUtilsMessageEXT submit_debug_message = NULL;
    submit_debug_message = (PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(instance, "vkSubmitDebugUtilsMessageEXT");
    
    if(submit_debug_message){
        VkDebugUtilsMessengerCallbackDataEXT data = {0};
        data.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT;
        data.pNext = NULL;
        data.flags = 0;
        data.pMessageIdName = NULL;
        data.pMessage = "Hello from debug messenger";
        submit_debug_message(instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &data);
    }else{
        printf("Couldn't create test debug function\n");
        exit(EXIT_FAILURE);
    }

    return;
}

static void t3_destroy_debug_messenger(const VkInstance instance){
    PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_messenger = NULL;
    destroy_debug_messenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(!destroy_debug_messenger){
        printf("Couldn't create destroy debug messenger\n");
        return;
    }
    destroy_debug_messenger(instance, messenger, NULL);

    return;
}

/*
static bool t3_check_layer(const char *layer_name){
    bool layers_found = false;
    unsigned int num_layers = 0;
    vkEnumerateInstanceLayerProperties(&num_layers, NULL);
    if(num_layers == 0)
        return false;

    VkLayerProperties *layer_properties = (VkLayerProperties *) malloc(num_layers * sizeof(VkLayerProperties));
    if(vkEnumerateInstanceLayerProperties(&num_layers, layer_properties) != VK_SUCCESS){
        free(layer_properties);
        return false;
    }

    for(unsigned int i = 0; i < num_layers && layers_found == false; i++){
        if(strcmp(layer_properties[i].layerName, layer_name) == 0){
            layers_found = true;
        }
    }

    free(layer_properties);
    return layers_found;
}
*/

bool t3_init_instance(TripleT_Graphics *t3_graphics){
    VkApplicationInfo application_info = {0};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext = NULL;
    application_info.pApplicationName = "TripleT Graphics X11";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "No engine";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_3;

    if(t3_graphics->debug_enabled == true){
	VkInstanceCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.pApplicationInfo = &application_info;
	create_info.enabledLayerCount = 1;        
	// LUNARG CRASH DIAGNOSTICS DOES NOT EXITS ON CACHYOS JUST LIKE IN MACOS, WHEN REWRITING THIS SHIT SPAGHETTI CODE REMEMBER THAT
	// TODO
	create_info.ppEnabledLayerNames = (const char *[]){"VK_LAYER_KHRONOS_validation"}; 
	create_info.enabledExtensionCount = 3;
	create_info.ppEnabledExtensionNames = (const char *[]){"VK_KHR_surface", "VK_KHR_xlib_surface", "VK_EXT_debug_utils"};
	VkResult result = vkCreateInstance(&create_info, NULL, &t3_graphics->instance);
	if(result != VK_SUCCESS)
	    return false;

	if(t3_graphics->debug_enabled)
	    t3_init_debug_messenger(t3_graphics->instance);
    }else{
	VkInstanceCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.pApplicationInfo = &application_info;
	create_info.enabledLayerCount = 0;        
	create_info.ppEnabledLayerNames = NULL;
	create_info.enabledExtensionCount = t3_x11_num_extensions;
	create_info.ppEnabledExtensionNames = t3_x11_extension_names;
	VkResult result = vkCreateInstance(&create_info, NULL, &t3_graphics->instance);
	if(result != VK_SUCCESS)
	    return false;
    }


    return true;
}

bool t3_init_surface(const TripleT_Window *t3_window, TripleT_Graphics *t3_graphics){
    VkXlibSurfaceCreateInfoKHR surface_create_info = {0}; 
    surface_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surface_create_info.pNext = NULL;
    surface_create_info.flags = 0;
    surface_create_info.dpy = t3_window->display;
    surface_create_info.window = t3_window->window;

    VkResult result = vkCreateXlibSurfaceKHR(t3_graphics->instance, &surface_create_info, NULL, &t3_graphics->surface);
    if(result != VK_SUCCESS)
	return false;

    return true;
}

bool t3_get_best_device(const VkInstance instance, Device_Info *device_info){
    unsigned int num_physical_devices = 0;
    vkEnumeratePhysicalDevices(instance, &num_physical_devices, NULL);
    if(num_physical_devices == 0)
	return false;
    
    VkPhysicalDevice *physical_devices = (VkPhysicalDevice *) malloc(num_physical_devices * sizeof(num_physical_devices));
    if(physical_devices == NULL)
	return false;
    
    if(vkEnumeratePhysicalDevices(instance, &num_physical_devices, physical_devices) != VK_SUCCESS){
	free(physical_devices);
	return false;
    }

    VkPhysicalDevice temp_device;
    VkPhysicalDeviceProperties physical_device_properties;
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    unsigned long long int max_vram = 0;
    bool has_dgpu = false;
    for(unsigned int i = 0; i < num_physical_devices; i++){
	vkGetPhysicalDeviceProperties(physical_devices[i], &physical_device_properties);
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &physical_device_memory_properties);

        if(physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            has_dgpu = true;
            if(physical_device_memory_properties.memoryHeaps->size > max_vram){
                temp_device = physical_devices[i];
                max_vram = physical_device_memory_properties.memoryHeaps->size;
            }
        }else{
            if(has_dgpu) 
                continue;
            else{
                temp_device = physical_devices[i];
            }
        }
    }

    device_info->physical_device = temp_device;
    free(physical_devices);
    return true;
}

bool t3_choose_queue_family(Device_Info *device_info, VkQueueFlags queue_family_flag, uint32_t num_queues_desired){
    unsigned int num_queue_families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device_info->physical_device, &num_queue_families, NULL);
    if(num_queue_families == 0)
        return false;

    VkQueueFamilyProperties *queue_family_properties = (VkQueueFamilyProperties *) malloc(num_queue_families * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device_info->physical_device, &num_queue_families, queue_family_properties);

    bool has_flag = false;
    for(unsigned int i = 0; i < num_queue_families; i++){
        if((queue_family_properties[i].queueFlags & queue_family_flag) == true){
            device_info->queue_index = i;
            device_info->num_queues = queue_family_properties[i].queueCount;
            if(num_queues_desired > queue_family_properties[i].queueCount)
                device_info->num_queues_in_use = queue_family_properties[i].queueCount;
            else    
                device_info->num_queues_in_use = num_queues_desired;

            has_flag = true;
            break;
        }
    }

    free(queue_family_properties);
    return has_flag;
}

bool t3_get_device_extensions(Device_Info *device_info){
    unsigned int num_device_extensions = 0;
    vkEnumerateDeviceExtensionProperties(device_info->physical_device, NULL, &num_device_extensions, NULL);
    if(num_device_extensions == 0)
        return false;

    VkExtensionProperties *device_extensions = (VkExtensionProperties *) malloc(num_device_extensions * sizeof(VkExtensionProperties));
    if(vkEnumerateDeviceExtensionProperties(device_info->physical_device, NULL, &num_device_extensions, device_extensions) != VK_SUCCESS){
        free(device_extensions);
        return false;
    }

    device_info->extensions.count = 1;
    device_info->extensions.names = (char **) malloc(1 * sizeof(char *));
    device_info->extensions.names[0] = (char *) calloc(128, sizeof(char));

    bool required_extension = false;
    for(unsigned int i = 0; i < num_device_extensions && required_extension == false; i++){
        if(strcmp(device_extensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0){
	    strcpy(device_info->extensions.names[0], device_extensions[i].extensionName);
            required_extension = true;
        }
    }

    free(device_extensions);
    return required_extension;
}

bool t3_is_device_supported(const Device_Info device_info, VkSurfaceKHR surface){
    VkBool32 is_supported;
    if(vkGetPhysicalDeviceSurfaceSupportKHR(device_info.physical_device, device_info.queue_index, surface, &is_supported) != VK_SUCCESS) 
        return false;
    if(is_supported == VK_FALSE)
        return false;

    return true;
}

bool t3_init_device_get_queue_handle(TripleT_Graphics *t3_graphics){
    if(!t3_get_best_device(t3_graphics->instance, &t3_graphics->device_info))
        return false;
    if(!t3_choose_queue_family(&t3_graphics->device_info, VK_QUEUE_GRAPHICS_BIT, 1))
        return false;
    if(!t3_get_device_extensions(&t3_graphics->device_info))
        return false;
    if(!t3_is_device_supported(t3_graphics->device_info, t3_graphics->surface))
        return false;

    float *priority = (float *) calloc(t3_graphics->device_info.num_queues_in_use, sizeof(float));
    float priority_value = 1.0f;
    for(uint32_t i = 0; i < t3_graphics->device_info.num_queues_in_use; i++){
        priority[i] = priority_value;
        priority_value -= 0.1f;
        if(priority_value <= 0.0f)
            priority_value = 0.0f;
    }
    
    VkDeviceQueueCreateInfo queue_create_info = {0};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.pNext = NULL;
    queue_create_info.flags = 0;
    queue_create_info.queueFamilyIndex = t3_graphics->device_info.queue_index;
    queue_create_info.queueCount = t3_graphics->device_info.num_queues_in_use;
    queue_create_info.pQueuePriorities = priority; 

    VkPhysicalDeviceVulkan13Features vulkan_1_3_features = {0};
    vulkan_1_3_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vulkan_1_3_features.dynamicRendering = VK_TRUE;
    vulkan_1_3_features.synchronization2 = VK_TRUE;

    VkDeviceCreateInfo device_create_info = {0};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &vulkan_1_3_features;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.enabledExtensionCount = t3_graphics->device_info.extensions.count;
    device_create_info.ppEnabledExtensionNames = (const char * const *)t3_graphics->device_info.extensions.names;
    device_create_info.pEnabledFeatures = &t3_graphics->device_info.features;

    if(vkCreateDevice(t3_graphics->device_info.physical_device, &device_create_info, NULL, &t3_graphics->device_info.logical_device) != VK_SUCCESS) 
        return false;

    vkGetDeviceQueue(t3_graphics->device_info.logical_device, t3_graphics->device_info.queue_index, 0, &t3_graphics->device_info.logical_device_queue);

    free(priority);
    return true;
}

bool t3_create_command_pool_buffer(TripleT_Graphics *t3_graphics, uint32_t num_command_pools, uint32_t num_command_buffers){
    t3_graphics->commands_info.num_command_pools = num_command_pools;
    t3_graphics->commands_info.num_command_buffers = num_command_buffers;
    t3_graphics->commands_info.command_pools = (VkCommandPool *) malloc(num_command_pools * sizeof(VkCommandPool));
    t3_graphics->commands_info.command_buffers = (VkCommandBuffer *) malloc(num_command_buffers * sizeof(VkCommandBuffer));

    VkCommandPoolCreateInfo command_pool_create_info = {0};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = NULL;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = t3_graphics->device_info.queue_index;

    for(unsigned int i = 0; i < num_command_pools; i++){
	if(vkCreateCommandPool(t3_graphics->device_info.logical_device, &command_pool_create_info, NULL, &t3_graphics->commands_info.command_pools[i]) != VK_SUCCESS)
	    return false;
    }

    VkCommandBufferAllocateInfo command_buffer_allocate_info = {0};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.pNext = NULL;
    command_buffer_allocate_info.commandPool = t3_graphics->commands_info.command_pools[0];
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    for(unsigned int i = 0; i < num_command_buffers; i++){
	if(vkAllocateCommandBuffers(t3_graphics->device_info.logical_device, &command_buffer_allocate_info, &t3_graphics->commands_info.command_buffers[i]) != VK_SUCCESS)
	    return false;
    }

    return true;
}

bool t3_get_swapchain_surface_capabilities(TripleT_Graphics *t3_graphics){
    VkSurfaceCapabilitiesKHR surface_capabililites = {0};
    if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(t3_graphics->device_info.physical_device, t3_graphics->surface, &surface_capabililites) != VK_SUCCESS)
        return false;
    
    t3_graphics->swapchain_info.min_image_count = surface_capabililites.minImageCount;
    t3_graphics->swapchain_info.max_image_count = surface_capabililites.maxImageCount;
    t3_graphics->swapchain_info.num_images_used = surface_capabililites.minImageCount + 1;
    t3_graphics->swapchain_info.min_image_extent = surface_capabililites.minImageExtent;
    t3_graphics->swapchain_info.max_image_extent = surface_capabililites.maxImageExtent;
    t3_graphics->swapchain_info.curr_image_extend = surface_capabililites.currentExtent;
    t3_graphics->swapchain_info.max_image_array_layers = surface_capabililites.maxImageArrayLayers;
    t3_graphics->swapchain_info.num_image_array_layers_used = 1;
    t3_graphics->swapchain_info.supported_transforms = surface_capabililites.supportedTransforms;
    t3_graphics->swapchain_info.chosen_transform = surface_capabililites.currentTransform;
    t3_graphics->swapchain_info.supported_alpha_flags = surface_capabililites.supportedCompositeAlpha;
    t3_graphics->swapchain_info.chosen_alpha_flag = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    t3_graphics->swapchain_info.supported_usage_flags = surface_capabililites.supportedUsageFlags;
    t3_graphics->swapchain_info.chosen_usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    return true;
}

bool t3_get_swapchain_device_formats(TripleT_Graphics *t3_graphics){
    unsigned int num_formats = 0;
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(t3_graphics->device_info.physical_device, t3_graphics->surface, &num_formats, NULL);
    if(num_formats == 0)
        return false;

    VkSurfaceFormatKHR *surface_formats = (VkSurfaceFormatKHR *) malloc(num_formats * sizeof(VkSurfaceFormatKHR));
    if(vkGetPhysicalDeviceSurfaceFormatsKHR(t3_graphics->device_info.physical_device, t3_graphics->surface, &num_formats, surface_formats) != VK_SUCCESS){
        free(surface_formats);
        return false;
    }

    bool found_format = false;
    static unsigned int num_possible_formats = 2;
    VkFormat possible_formats[num_possible_formats];
    possible_formats[0] = VK_FORMAT_B8G8R8A8_SRGB;
    possible_formats[1] = VK_FORMAT_B8G8R8A8_UNORM;

    /**
    * @brief: This Loop has 2 possible formats to choose: B8G8R8A8_SRGB or B8G8R8A8_UNORM. 
    *         If none of these formats are available, it will fallback 
    */
    for(unsigned int i = 0; i < num_possible_formats && found_format != true; i++){
        for(unsigned int j = 0; j < num_formats; j++){
            if(possible_formats[i] == surface_formats[j].format){
                t3_graphics->swapchain_info.format = surface_formats[j].format;
                t3_graphics->swapchain_info.color_space = surface_formats[j].colorSpace;
                found_format = true;
                break;
            }
        }
    }

    free(surface_formats);
    return found_format;
}

bool t3_get_swapchain_present_mode(TripleT_Graphics *t3_graphics){
    unsigned int num_present_modes = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(t3_graphics->device_info.physical_device, t3_graphics->surface, &num_present_modes, NULL);
    if(num_present_modes == 0)
        return false;

    VkPresentModeKHR *available_present_modes = (VkPresentModeKHR *) malloc(num_present_modes * sizeof(VkPresentModeKHR));
    if(vkGetPhysicalDeviceSurfacePresentModesKHR(t3_graphics->device_info.physical_device, t3_graphics->surface, &num_present_modes, available_present_modes) != VK_SUCCESS){
        free(available_present_modes);
        return false;
    }

    bool found_present_mode = false;
    static unsigned int num_possible_present_modes = 3;
    VkPresentModeKHR possible_present_modes[num_possible_present_modes];
    possible_present_modes[0] = VK_PRESENT_MODE_FIFO_KHR;
    possible_present_modes[1] = VK_PRESENT_MODE_MAILBOX_KHR;
    possible_present_modes[2] = VK_PRESENT_MODE_IMMEDIATE_KHR;

    /**
    * @brief: This Loop has 3 possible present modes to choose: FIFO, MAILBOX or IMMEDIATE. 
    *         If none of these present modes are available, it will fallback 
    */
    for(unsigned int i = 0; i < num_possible_present_modes && found_present_mode != true; i++){
        for(unsigned int j = 0; j < num_present_modes; j++){
            if(possible_present_modes[i] == available_present_modes[j]){
                t3_graphics->swapchain_info.present_mode = available_present_modes[j];
                found_present_mode = true;
                break;
            }
        }
    }

    free(available_present_modes);
    return found_present_mode;
}

static bool t3_init_swapchain(TripleT_Graphics *t3_graphics){
    if(!t3_get_swapchain_surface_capabilities(t3_graphics))
        return false;
    if(!t3_get_swapchain_device_formats(t3_graphics))
        return false;
    if(!t3_get_swapchain_present_mode(t3_graphics))
        return false;

    VkSwapchainCreateInfoKHR swapchain_create_info = {0};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = NULL;
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = t3_graphics->surface;
    swapchain_create_info.minImageCount = t3_graphics->swapchain_info.num_images_used;
    swapchain_create_info.imageFormat = t3_graphics->swapchain_info.format;
    swapchain_create_info.imageColorSpace = t3_graphics->swapchain_info.color_space;
    swapchain_create_info.imageExtent = t3_graphics->swapchain_info.curr_image_extend;
    swapchain_create_info.imageArrayLayers = t3_graphics->swapchain_info.num_image_array_layers_used;
    swapchain_create_info.imageUsage = t3_graphics->swapchain_info.chosen_usage_flags;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.pQueueFamilyIndices = NULL;
    swapchain_create_info.preTransform = t3_graphics->swapchain_info.chosen_transform;
    swapchain_create_info.compositeAlpha = t3_graphics->swapchain_info.chosen_alpha_flag;
    swapchain_create_info.presentMode = t3_graphics->swapchain_info.present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(t3_graphics->device_info.logical_device, &swapchain_create_info, NULL, &t3_graphics->swapchain_info.swapchain) != VK_SUCCESS)
        return false;

    return true;
}

bool t3_init_image_views(TripleT_Graphics *t3_graphics){
    vkGetSwapchainImagesKHR(t3_graphics->device_info.logical_device, t3_graphics->swapchain_info.swapchain, &t3_graphics->image_info.num_images, NULL);
    if(t3_graphics->image_info.num_images == 0)
	return false;

    t3_graphics->image_info.images = (VkImage *) malloc(t3_graphics->image_info.num_images * sizeof(VkImage));
    t3_graphics->image_info.image_view = (VkImageView *) malloc(t3_graphics->image_info.num_images * sizeof(VkImageView));
    
    if(vkGetSwapchainImagesKHR(t3_graphics->device_info.logical_device, t3_graphics->swapchain_info.swapchain, &t3_graphics->image_info.num_images, t3_graphics->image_info.images) != VK_SUCCESS)
	return false;

    for(unsigned int i = 0; i < t3_graphics->image_info.num_images; i++){
        VkImageViewCreateInfo image_view_create_info = {0};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.pNext = NULL;
        image_view_create_info.flags = 0;
        image_view_create_info.image = t3_graphics->image_info.images[i];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = t3_graphics->swapchain_info.format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY; 
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;

        if(vkCreateImageView(t3_graphics->device_info.logical_device, &image_view_create_info, NULL, &t3_graphics->image_info.image_view[i]) != VK_SUCCESS)
            return false;
    }
    t3_graphics->image_info.num_frames = 2;

    return true;
}

bool t3_create_sync_objects(TripleT_Graphics *t3_graphics){
    t3_graphics->sync_objects_info.fences = (VkFence *) malloc(t3_graphics->commands_info.num_command_buffers * sizeof(VkFence));
    t3_graphics->sync_objects_info.image_available_semaphores = (VkSemaphore *) malloc(t3_graphics->image_info.num_frames * sizeof(VkSemaphore));
    t3_graphics->sync_objects_info.render_finished_semaphores = (VkSemaphore *) malloc(t3_graphics->image_info.num_images * sizeof(VkSemaphore));

    VkSemaphoreCreateInfo semaphore_create_info = {0};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = NULL;
    semaphore_create_info.flags = 0;

    VkFenceCreateInfo fence_create_info = {0};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.pNext = NULL;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;    // MIGHT NEED TO CREATE IT IN THE SIGNALED STATE

    VkResult result;

    for(unsigned int i = 0; i < t3_graphics->commands_info.num_command_buffers; i++){
	result = vkCreateFence(t3_graphics->device_info.logical_device, &fence_create_info, NULL, &t3_graphics->sync_objects_info.fences[i]);
	if(result != VK_SUCCESS)
	    return false;
    }
    for(unsigned int i = 0; i < t3_graphics->image_info.num_frames; i++){
	result = vkCreateSemaphore(t3_graphics->device_info.logical_device, &semaphore_create_info, NULL, &t3_graphics->sync_objects_info.image_available_semaphores[i]);
	if(result != VK_SUCCESS)
	    return false;
    }
    for(unsigned int i = 0; i < t3_graphics->image_info.num_images; i++){
	result = vkCreateSemaphore(t3_graphics->device_info.logical_device, &semaphore_create_info, NULL, &t3_graphics->sync_objects_info.render_finished_semaphores[i]);
	if(result != VK_SUCCESS)
	    return false;
    }

    return true;
}

bool t3_create_shader(VkDevice logical_device, struct Shader_Data *shader_data, unsigned int *code, const unsigned int code_size, Shader_Type shader_type){
    VkShaderModuleCreateInfo shader_module_create_info = {
	.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	.pNext = NULL,
	.flags = 0,
	.pCode = code,
	.codeSize = code_size,
    };

    VkResult result = vkCreateShaderModule(logical_device, &shader_module_create_info, NULL, &shader_data->shader_module);
    if(result != VK_SUCCESS)
	return false;

   shader_data->shader_type = shader_type;
   shader_data->shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   shader_data->shader_stage_create_info.pNext = NULL;
   shader_data->shader_stage_create_info.flags = 0;
   switch(shader_type){
       case VERTEX_SHADER:
	    shader_data->shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;	
	    break;

       case FRAGMENT_SHADER:
	    shader_data->shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;	
	    break;
   }
   shader_data->shader_stage_create_info.module = shader_data->shader_module;
   shader_data->shader_stage_create_info.pName = "main";
   shader_data->shader_stage_create_info.pSpecializationInfo = NULL;

    return true;
}

VkPipelineVertexInputStateCreateInfo create_vertex_input_state_info(void){
    static VkVertexInputBindingDescription vertex_input_binding_desc = {
	.binding = 0,
	.stride = sizeof(TripleT_Vertex_3D),
	.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    static VkVertexInputAttributeDescription vertex_input_attr_desc[2] = {
	[0] = {
	    .location = 0,
	    .binding = 0,
	    .format = VK_FORMAT_R32G32B32_SFLOAT,
	    .offset = offsetof(TripleT_Vertex_3D, position),
	},
	[1] = {
	    .location = 1,
	    .binding = 0,
	    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
	    .offset = offsetof(TripleT_Vertex_3D, color),
	},
    };

    VkPipelineVertexInputStateCreateInfo vertex_state_create_info = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	.pNext = NULL,
	.flags = 0,
	.vertexBindingDescriptionCount = 1,
	.pVertexBindingDescriptions = &vertex_input_binding_desc,
	.vertexAttributeDescriptionCount = 2,
	.pVertexAttributeDescriptions = vertex_input_attr_desc,
    };

    return vertex_state_create_info;
}

VkPipelineInputAssemblyStateCreateInfo create_assembly_state_create_info(VkPrimitiveTopology topology){
    VkPipelineInputAssemblyStateCreateInfo assembly_state_create_info = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	.pNext = NULL,
	.flags = 0,
	.topology = topology,
	.primitiveRestartEnable = VK_FALSE,
    };

    return assembly_state_create_info;
}

VkPipelineViewportStateCreateInfo create_viewport_state_create_info(const VkExtent2D curr_image_extent){
    static VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float) curr_image_extent.width;
    viewport.height = (float) curr_image_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    static VkRect2D scissor = {0};
    scissor.extent = curr_image_extent;

    VkPipelineViewportStateCreateInfo viewport_state_create_info = {0};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.pNext = NULL;
    viewport_state_create_info.flags = 0;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;

    return viewport_state_create_info;
}

VkPipelineRasterizationStateCreateInfo create_rasterizer_state_info(bool enable_clamp, bool enable_rasterizer_discard, VkPolygonMode polygon_mode, VkCullModeFlags cull_flags, VkFrontFace face_orientation, float line_width){
    VkPipelineRasterizationStateCreateInfo rasterizer_state_info = {0};
    rasterizer_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_state_info.pNext = NULL;
    rasterizer_state_info.flags = 0;
    if(enable_clamp)
        rasterizer_state_info.depthClampEnable = VK_TRUE;
    else
        rasterizer_state_info.depthClampEnable = VK_FALSE;
    if(enable_rasterizer_discard)
        rasterizer_state_info.rasterizerDiscardEnable = VK_TRUE;
    else
        rasterizer_state_info.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_state_info.polygonMode = polygon_mode;
    rasterizer_state_info.cullMode = cull_flags;
    rasterizer_state_info.frontFace = face_orientation;
    rasterizer_state_info.depthBiasEnable = VK_FALSE;
    rasterizer_state_info.depthBiasConstantFactor = 0.0f;
    rasterizer_state_info.depthBiasClamp = 0.0f;
    rasterizer_state_info.depthBiasSlopeFactor = 0.0f;
    rasterizer_state_info.lineWidth = line_width;

    return rasterizer_state_info;
}

VkPipelineMultisampleStateCreateInfo create_multisample_state_create_info(uint32_t num_samples, bool enable_shading, float min_sample_shading_fraction){
    VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {0};
    multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.pNext = NULL;
    multisample_state_create_info.flags = 0;
    switch(num_samples){
        case 1:
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        break;

        case 2:
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
        break;

        case 4:
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
        break;

        case 8:
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;
        break;

        case 16:
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT;
        break;

        case 32:
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_32_BIT;
        break;

        case 64:
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_64_BIT;
        break;
    }

    if(enable_shading){
        multisample_state_create_info.sampleShadingEnable = VK_TRUE;
        multisample_state_create_info.minSampleShading = min_sample_shading_fraction;
    }
    else{
        multisample_state_create_info.sampleShadingEnable = VK_FALSE;
        multisample_state_create_info.minSampleShading = 1.0f;
    }
    multisample_state_create_info.pSampleMask = NULL;
    multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
    multisample_state_create_info.alphaToOneEnable = VK_FALSE;

    return multisample_state_create_info;
}

VkPipelineColorBlendStateCreateInfo create_color_blend_state_create_info(bool enable_color_blending){
    static VkPipelineColorBlendStateCreateInfo color_blend_state_info = {0};
    static VkPipelineColorBlendAttachmentState color_blend_attachment_state = {0};

    if(enable_color_blending){
        color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_info.pNext = NULL;
        color_blend_state_info.flags = 0;
        color_blend_state_info.logicOpEnable = VK_TRUE;
        color_blend_state_info.logicOp = VK_LOGIC_OP_COPY;
        color_blend_state_info.attachmentCount = 1;
        color_blend_attachment_state.blendEnable = VK_TRUE;
        color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_state_info.pAttachments = &color_blend_attachment_state;
        color_blend_state_info.blendConstants[0] = 0.0f;
        color_blend_state_info.blendConstants[1] = 0.0f;
        color_blend_state_info.blendConstants[2] = 0.0f;
        color_blend_state_info.blendConstants[3] = 0.0f;
    }else{
        color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_info.pNext = NULL;
        color_blend_state_info.flags = 0;
        color_blend_state_info.logicOpEnable = VK_FALSE;
        color_blend_state_info.logicOp = VK_LOGIC_OP_COPY;
        color_blend_state_info.attachmentCount = 1;
        color_blend_attachment_state.blendEnable = VK_FALSE;
        color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_state_info.pAttachments = &color_blend_attachment_state;
        color_blend_state_info.blendConstants[0] = 0.0f;
        color_blend_state_info.blendConstants[1] = 0.0f;
        color_blend_state_info.blendConstants[2] = 0.0f;
        color_blend_state_info.blendConstants[3] = 0.0f;
    }
    
    return color_blend_state_info;
}

VkPipelineDynamicStateCreateInfo create_dynamic_state_create_info(bool tesselation_enabled, bool depth_stencil_enabled){
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {0};

    if(!tesselation_enabled && ! depth_stencil_enabled){
        static VkDynamicState dynamic_states[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };
        
        dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.pNext = NULL;
        dynamic_state_create_info.flags = 0;
        dynamic_state_create_info.dynamicStateCount = 2;
        dynamic_state_create_info.pDynamicStates = dynamic_states;
    }else if(tesselation_enabled){
        // LOGIC TO IMPLEMENT LATER ON
    }else if(depth_stencil_enabled){
        // LOGIC TO IMPLEMENT LATER ON
    }else{
        // LOGIC TO IMPLEMENT LATER ON
    }

    return dynamic_state_create_info;
}

bool create_graphics_pipeline_layout(const VkDevice logical_device, VkPipelineLayout *pipeline_layout){
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {0};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pNext = NULL;
    pipeline_layout_create_info.flags = 0;
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pSetLayouts = NULL;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = NULL;

    if(vkCreatePipelineLayout(logical_device, &pipeline_layout_create_info, NULL, pipeline_layout) != VK_SUCCESS) 
        return false;

    return true;
}

bool t3_create_graphics_pipeline(TripleT_Graphics *t3_graphics){
    t3_graphics->graphics_pipeline_info.Shader_Info.num_shaders = 2;
    t3_graphics->graphics_pipeline_info.Shader_Info.shader_data = (struct Shader_Data *)malloc(t3_graphics->graphics_pipeline_info.Shader_Info.num_shaders * sizeof(struct Shader_Data));
    if(!t3_create_shader(t3_graphics->device_info.logical_device, &t3_graphics->graphics_pipeline_info.Shader_Info.shader_data[0], T3_Vertex_Shader_data, T3_Vertex_Shader_size, VERTEX_SHADER))
	return false;
    if(!t3_create_shader(t3_graphics->device_info.logical_device, &t3_graphics->graphics_pipeline_info.Shader_Info.shader_data[1], T3_Fragment_Shader_data, T3_Fragment_Shader_size, FRAGMENT_SHADER))
	return false;

    VkPipelineVertexInputStateCreateInfo vertex_state_create_info = create_vertex_input_state_info();
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = create_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    VkPipelineViewportStateCreateInfo viewport_state_create_info = create_viewport_state_create_info(t3_graphics->swapchain_info.curr_image_extend);
    VkPipelineRasterizationStateCreateInfo rasterizer_state_create_info = create_rasterizer_state_info(false, false, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 1.0f);
    VkPipelineMultisampleStateCreateInfo multisample_state_create_info = create_multisample_state_create_info(1, false, 0.0f);
    VkPipelineColorBlendStateCreateInfo color_blend_state_info = create_color_blend_state_create_info(false);
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = create_dynamic_state_create_info(false, false);
    if(!create_graphics_pipeline_layout(t3_graphics->device_info.logical_device, &t3_graphics->graphics_pipeline_info.pipeline_layout))
        return false;

    VkPipelineRenderingCreateInfo pipeline_rendering_create_info = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
	.pNext = NULL,
	.viewMask = 0,
	.colorAttachmentCount = 1,
	.pColorAttachmentFormats = &t3_graphics->swapchain_info.format,
    };

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
	.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	.pNext = &pipeline_rendering_create_info,
	.flags = 0,
	.stageCount = t3_graphics->graphics_pipeline_info.Shader_Info.num_shaders,
	.pStages = (VkPipelineShaderStageCreateInfo[]){
	    t3_graphics->graphics_pipeline_info.Shader_Info.shader_data[0].shader_stage_create_info,
	    t3_graphics->graphics_pipeline_info.Shader_Info.shader_data[1].shader_stage_create_info
	},
	.pVertexInputState = &vertex_state_create_info,
	.pInputAssemblyState = &input_assembly_state_create_info,
	.pTessellationState = NULL,
	.pViewportState = &viewport_state_create_info,
	.pRasterizationState = &rasterizer_state_create_info,
	.pMultisampleState = &multisample_state_create_info,
	.pDepthStencilState = NULL,
	.pColorBlendState = &color_blend_state_info,
	.pDynamicState = &dynamic_state_create_info,
	.layout = t3_graphics->graphics_pipeline_info.pipeline_layout,
	.renderPass = NULL,
	.basePipelineHandle = VK_NULL_HANDLE,
	.basePipelineIndex = -1,
    };

    if(vkCreateGraphicsPipelines(t3_graphics->device_info.logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, NULL, &t3_graphics->graphics_pipeline_info.graphics_pipeline) != VK_SUCCESS)
	return false;

    return true;
}

void t3_recreate_swapchain(TripleT_Graphics *t3_graphics){
    vkDeviceWaitIdle(t3_graphics->device_info.logical_device);
    t3_destroy_image_views(t3_graphics);  
    t3_destroy_swapchain(t3_graphics);
    t3_init_swapchain(t3_graphics);
    t3_init_image_views(t3_graphics);

    return;
}


void t3_destroy_graphics_pipeline(TripleT_Graphics *t3_graphics){
    for(unsigned int i = 0; i < t3_graphics->graphics_pipeline_info.Shader_Info.num_shaders; i++)
	vkDestroyShaderModule(t3_graphics->device_info.logical_device, t3_graphics->graphics_pipeline_info.Shader_Info.shader_data[i].shader_module, NULL);
    free(t3_graphics->graphics_pipeline_info.Shader_Info.shader_data);
    vkDestroyPipelineLayout(t3_graphics->device_info.logical_device, t3_graphics->graphics_pipeline_info.pipeline_layout, NULL);
    vkDestroyPipeline(t3_graphics->device_info.logical_device, t3_graphics->graphics_pipeline_info.graphics_pipeline, NULL);

    return;
}

void t3_destroy_sync_objects(TripleT_Graphics *t3_graphics){
    for(unsigned int i = 0; i < t3_graphics->commands_info.num_command_buffers; i++)
	vkDestroyFence(t3_graphics->device_info.logical_device, t3_graphics->sync_objects_info.fences[0], NULL);
    
    for(unsigned int i = 0; i < t3_graphics->image_info.num_frames; i++)
	vkDestroySemaphore(t3_graphics->device_info.logical_device, t3_graphics->sync_objects_info.image_available_semaphores[i], NULL);
    
    for(unsigned int i = 0; i < t3_graphics->image_info.num_images; i++)
	vkDestroySemaphore(t3_graphics->device_info.logical_device, t3_graphics->sync_objects_info.render_finished_semaphores[i], NULL);

    free(t3_graphics->sync_objects_info.image_available_semaphores);
    free(t3_graphics->sync_objects_info.render_finished_semaphores);
    free(t3_graphics->sync_objects_info.fences);

    return;
}

void t3_destroy_image_views(TripleT_Graphics *t3_graphics){
    for(unsigned int i = 0; i < t3_graphics->image_info.num_images; i++){
	vkDestroyImageView(t3_graphics->device_info.logical_device, t3_graphics->image_info.image_view[i], NULL);
    }
    free(t3_graphics->image_info.image_view);
    free(t3_graphics->image_info.images);

    return;
}

void t3_destroy_swapchain(TripleT_Graphics *t3_graphics){
    vkDestroySwapchainKHR(t3_graphics->device_info.logical_device, t3_graphics->swapchain_info.swapchain, NULL);

    return;
}

void t3_destroy_command_pool_buffer(TripleT_Graphics *t3_graphics){
    for(unsigned int i = 0; i < t3_graphics->commands_info.num_command_pools; i++){
	vkDestroyCommandPool(t3_graphics->device_info.logical_device, t3_graphics->commands_info.command_pools[i], NULL);
    }
    free(t3_graphics->commands_info.command_pools);
    free(t3_graphics->commands_info.command_buffers);

    return;
}

void t3_destroy_device(TripleT_Graphics *t3_graphics){
    vkDestroyDevice(t3_graphics->device_info.logical_device, NULL);
    for(unsigned int i = 0; i < t3_graphics->device_info.extensions.count; i++){
	free(t3_graphics->device_info.extensions.names[i]);
    }
    free(t3_graphics->device_info.extensions.names);

    return;
}

void t3_destroy_surface(TripleT_Graphics *t3_graphics){
    vkDestroySurfaceKHR(t3_graphics->instance, t3_graphics->surface, NULL);

    return;
}

void t3_destroy_instance(TripleT_Graphics *t3_graphics){
    if(t3_graphics->debug_enabled)
	t3_destroy_debug_messenger(t3_graphics->instance);
    vkDestroyInstance(t3_graphics->instance, NULL);

    return;
}
