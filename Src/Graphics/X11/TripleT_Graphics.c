#include "TripleT_Graphics.h"
#include "../Internals/TripleT_Engine_Graphics_X11_Internals.h"
#include "../../UI/Internals/TripleT_Engine_X11_Internal.h"
#include "TripleT_Window.h"
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
static bool t3_create_render_pass(TripleT_Graphics *t3_graphics);
static bool t3_init_image_views(TripleT_Graphics *t3_graphics);
static bool t3_create_frame_buffers(TripleT_Graphics *t3_graphics);
static bool t3_create_sync_objects(TripleT_Graphics *t3_graphics);
// Graphics Destruction
static void t3_destroy_sync_objects(TripleT_Graphics *t3_graphics);
static void t3_destroy_frame_buffers(TripleT_Graphics *t3_graphics);
static void t3_destroy_image_views(TripleT_Graphics *t3_graphics);
static void t3_destroy_render_pass(TripleT_Graphics *t3_graphics);
static void t3_destroy_swapchain(TripleT_Graphics *t3_graphics);
static void t3_destroy_command_pool_buffer(TripleT_Graphics *t3_graphics);
static void t3_destroy_device(TripleT_Graphics *t3_graphics);
static void t3_destroy_surface(TripleT_Graphics *t3_graphics);
static void t3_destroy_instance(TripleT_Graphics *t3_graphics);

TripleT_Graphics *t3_init_graphics(const TripleT_Window *t3_window, TripleT_Graphics_Errors *t3_graphics_error){
    TripleT_Graphics *t3_graphics = (TripleT_Graphics *) malloc(sizeof(TripleT_Graphics));
    if(t3_graphics == NULL)
	return NULL;
    
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
    if(!t3_create_render_pass(t3_graphics)){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_RENDER_PASS;
	return NULL;
    }
    if(!t3_init_image_views(t3_graphics)){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_IMAGE_VIEW;
	return NULL;
    }
    if(!t3_create_frame_buffers(t3_graphics)){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_FRAME_BUFFER;
	return NULL;
    }
    if(!t3_create_sync_objects(t3_graphics)){
	if(t3_graphics_error != NULL)
	    *t3_graphics_error = TRIPLET_GRAPHICS_ERROR_SYNC_OBJECTS;
	return NULL;
    }

    return t3_graphics;
}

void t3_temp_render_fn(TripleT_Graphics *t3_graphics){
    vkWaitForFences(t3_graphics->device_info.logical_device, 1, &t3_graphics->sync_objects_info.fences[0], VK_TRUE, UINT64_MAX);

    unsigned int image_index;

    vkAcquireNextImageKHR(t3_graphics->device_info.logical_device, t3_graphics->swapchain_info.swapchain, UINT64_MAX, t3_graphics->sync_objects_info.image_available_semaphores[0], VK_NULL_HANDLE, &image_index);

    vkResetFences(t3_graphics->device_info.logical_device, 1, &t3_graphics->sync_objects_info.fences[0]);

    vkResetCommandBuffer(t3_graphics->commands_info.command_buffers[0], 0);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(t3_graphics->commands_info.command_buffers[0], &begin_info);

    VkClearValue clear_color = {
	.color = {{0.1f, 0.2f, 0.3f, 1.0f}}
    };

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = t3_graphics->render_pass;
    render_pass_info.framebuffer = t3_graphics->image_info.frame_buffers[image_index];
    render_pass_info.renderArea.offset = (VkOffset2D){0, 0};
    render_pass_info.renderArea.extent = t3_graphics->swapchain_info.curr_image_extend;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(t3_graphics->commands_info.command_buffers[0], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdEndRenderPass(t3_graphics->commands_info.command_buffers[0]);

    vkEndCommandBuffer(t3_graphics->commands_info.command_buffers[0]);

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &t3_graphics->sync_objects_info.image_available_semaphores[0];
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &t3_graphics->commands_info.command_buffers[0];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &t3_graphics->sync_objects_info.render_finished_semaphores[0];

    vkQueueSubmit(t3_graphics->device_info.logical_device_queue, 1, &submit_info, t3_graphics->sync_objects_info.fences[0]);

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &t3_graphics->sync_objects_info.render_finished_semaphores[0];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &t3_graphics->swapchain_info.swapchain;
    present_info.pImageIndices = &image_index;

    vkQueuePresentKHR(t3_graphics->device_info.logical_device_queue, &present_info);

    return;
}

void t3_destroy_graphics(TripleT_Graphics *t3_graphics){
    t3_destroy_sync_objects(t3_graphics);
    t3_destroy_frame_buffers(t3_graphics);
    t3_destroy_image_views(t3_graphics);
    t3_destroy_render_pass(t3_graphics);
    t3_destroy_swapchain(t3_graphics);
    t3_destroy_command_pool_buffer(t3_graphics);
    t3_destroy_device(t3_graphics);
    t3_destroy_surface(t3_graphics);
    t3_destroy_instance(t3_graphics);
    free(t3_graphics); 

    return;
}

bool t3_init_instance(TripleT_Graphics *t3_graphics){
    VkApplicationInfo application_info = {0};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext = NULL;
    application_info.pApplicationName = "TripleT Graphics X11";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "No engine";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_0;

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

    VkDeviceCreateInfo device_create_info = {0};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = NULL;
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

bool t3_create_render_pass(TripleT_Graphics *t3_graphics){
    VkAttachmentDescription attachment_description = {0};
    attachment_description.flags = 0;
    attachment_description.format = t3_graphics->swapchain_info.format;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_reference = {0};
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_reference = {0};
    subpass_reference.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_reference.colorAttachmentCount = 1;
    subpass_reference.pColorAttachments = &color_attachment_reference;

    // This dependency struct was added when i was creating my draw function
    VkSubpassDependency subpass_dependency = {0};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_create_info = {0};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.pNext = NULL;
    render_pass_create_info.flags = 0;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &attachment_description;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_reference;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    if(vkCreateRenderPass(t3_graphics->device_info.logical_device, &render_pass_create_info, NULL, &t3_graphics->render_pass) != VK_SUCCESS) 
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
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;    // TEST IF THIS COMPONENT COULD BE THE WAY I CHANGE THE COLOR OF MY SCREEN
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

    return true;
}

bool t3_create_frame_buffers(TripleT_Graphics *t3_graphics){
    t3_graphics->image_info.frame_buffers = (VkFramebuffer *) malloc(t3_graphics->image_info.num_images * sizeof(VkFramebuffer));

    for(unsigned int i = 0; i < t3_graphics->image_info.num_images; i++){
	VkFramebufferCreateInfo frame_buffer_create_info = {0};
	frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frame_buffer_create_info.pNext = NULL;
	frame_buffer_create_info.flags = 0;
	frame_buffer_create_info.renderPass = t3_graphics->render_pass;
	frame_buffer_create_info.attachmentCount = 1;
	frame_buffer_create_info.pAttachments = &t3_graphics->image_info.image_view[i];
	frame_buffer_create_info.width = t3_graphics->swapchain_info.curr_image_extend.width;
	frame_buffer_create_info.height = t3_graphics->swapchain_info.curr_image_extend.height;
	frame_buffer_create_info.layers = t3_graphics->swapchain_info.num_image_array_layers_used;

	if(vkCreateFramebuffer(t3_graphics->device_info.logical_device, &frame_buffer_create_info, NULL, &t3_graphics->image_info.frame_buffers[i]) != VK_SUCCESS)
            return false;
    }

    return true;
}

bool t3_create_sync_objects(TripleT_Graphics *t3_graphics){
    t3_graphics->sync_objects_info.num_sync_objects = 1;
    t3_graphics->sync_objects_info.image_available_semaphores = (VkSemaphore *) malloc(t3_graphics->sync_objects_info.num_sync_objects * sizeof(VkSemaphore));
    t3_graphics->sync_objects_info.fences = (VkFence *) malloc(t3_graphics->sync_objects_info.num_sync_objects * sizeof(VkFence));
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
    for(unsigned int i = 0; i < t3_graphics->sync_objects_info.num_sync_objects; i++){
	result = vkCreateSemaphore(t3_graphics->device_info.logical_device, &semaphore_create_info, NULL, &t3_graphics->sync_objects_info.image_available_semaphores[i]);
	if(result != VK_SUCCESS)
	    return false;
	result = vkCreateFence(t3_graphics->device_info.logical_device, &fence_create_info, NULL, &t3_graphics->sync_objects_info.fences[i]);
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

void t3_destroy_sync_objects(TripleT_Graphics *t3_graphics){
    for(unsigned int i = 0; i < t3_graphics->sync_objects_info.num_sync_objects; i++){
	vkDestroySemaphore(t3_graphics->device_info.logical_device, t3_graphics->sync_objects_info.image_available_semaphores[i], NULL);
	vkDestroyFence(t3_graphics->device_info.logical_device, t3_graphics->sync_objects_info.fences[i], NULL);
    }
    for(unsigned int i = 0; i < t3_graphics->image_info.num_images; i++)
	vkDestroySemaphore(t3_graphics->device_info.logical_device, t3_graphics->sync_objects_info.render_finished_semaphores[i], NULL);

    free(t3_graphics->sync_objects_info.image_available_semaphores);
    free(t3_graphics->sync_objects_info.render_finished_semaphores);
    free(t3_graphics->sync_objects_info.fences);

    return;
}

void t3_destroy_frame_buffers(TripleT_Graphics *t3_graphics){
    for(unsigned int i = 0; i < t3_graphics->image_info.num_images; i++){
	vkDestroyFramebuffer(t3_graphics->device_info.logical_device, t3_graphics->image_info.frame_buffers[i], NULL);
    }
    free(t3_graphics->image_info.frame_buffers);

}

void t3_destroy_image_views(TripleT_Graphics *t3_graphics){
    for(unsigned int i = 0; i < t3_graphics->image_info.num_images; i++){
	vkDestroyImageView(t3_graphics->device_info.logical_device, t3_graphics->image_info.image_view[i], NULL);
    }
    free(t3_graphics->image_info.image_view);
    free(t3_graphics->image_info.images);

    return;
}

void t3_destroy_render_pass(TripleT_Graphics *t3_graphics){
    vkDestroyRenderPass(t3_graphics->device_info.logical_device, t3_graphics->render_pass, NULL);

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
    vkDestroyInstance(t3_graphics->instance, NULL);

    return;
}

