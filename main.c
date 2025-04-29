
#include <stdio.h>
#include <string.h>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "allocator.h"

#define XGL_VK_API_VERSION VK_API_VERSION_1_3
#define QUEUE_FAMILY_GRAPHICS_INDEX 0
#define QUEUE_FAMILY_PRESENT_INDEX  1
#define MAX_FRAME_ON_DRAW   2

#define lenof(_array) (sizeof(_array) / sizeof(typeof(_array[0])))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define minmax(a, b, c) min(max(a, min(b, c)), max(b, c))
#define maxmin(a, b, c) max(min(a, max(b, c)), min(b, c))

#define rt_error(fmt, ...)   fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)
#define rt_message(fmt, ...) fprintf(stdout, "[INFO] " fmt "\n", ##__VA_ARGS__)
#define rt_warning(fmt, ...) fprintf(stderr, "[WARNING] " fmt "\n", ##__VA_ARGS__)
#define rt_debug(fmt, ...)   fprintf(stdout, "[DEBUG] " fmt "\n", ##__VA_ARGS__)

uint8_t *loadBinFile(const char *filepath, uint32_t *real_size, const Allocator *allocator);

void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

const char* INSTANCE_REQUIRED_EXTENSION_NAMES[] = {
    /** @{ glfw required extensions */
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(WIN64)
    #define XGL_SURFACE_WIN32 1
    "VK_KHR_win32_surface",
#elif defined(linux)
#define XGL_SURFACE_WAYLAND 1
    "VK_KHR_wayland_surface",
#else
#error "Unknown os platform, unable to set window surface."
#endif
    /** @} */
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
//    VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME,
};

const char* INSTANCE_REQUIRED_LAYER_NAMES[] = {
    "VK_LAYER_KHRONOS_validation",
};

const char* REQUIRED_DEVICE_EXTENSION_NAMES[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const char* REQUIRED_DEVICE_LAYER_NAMES[] = {
};

static bool framebufferResized = false;

int main(int argc, char * argv[]) {
  VkResult result = {};

  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  const Allocator *allocator = &STDAllocator;

  GLFWwindow *window = glfwCreateWindow(1920 >> 1, (1080 >> 2) * 3, "Vulkan Window", nullptr, nullptr);
  if (window == NULL) {
      fprintf(stderr, "Failed to create GLFW window.\n");
     goto __failed_to_create_glfw_window;
  }

  glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);

  glfwMakeContextCurrent(window);

  /* show all extensions required */ {
    /* enumerate supported extensions */
    uint32_t supportedExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
    VkExtensionProperties *supportedExtensions = allocator->calloc(supportedExtensionCount, sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions);
    for (uint32_t i = 0; i < supportedExtensionCount; i++) {
      rt_debug("Support extension '%s' of version %u", supportedExtensions[i].extensionName, supportedExtensions->specVersion);
    }
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
      for (uint32_t j = 0; j < supportedExtensionCount; j++) {
        if (strcmp(glfwExtensions[i], supportedExtensions[j].extensionName) == 0) {
          rt_message("GLFW required extension '%s' is supported", glfwExtensions[i]);
          goto __glfw_required_extension_verified;
        }
      }
      rt_warning("GLFW required extension '%s' is not supported", glfwExtensions[i]);
      __glfw_required_extension_verified:
    }
    uint32_t extensionCount = lenof(INSTANCE_REQUIRED_EXTENSION_NAMES);
    const char * const*extensions = INSTANCE_REQUIRED_EXTENSION_NAMES;
    for (uint32_t i = 0; i < extensionCount; i++) {
      for (uint32_t j = 0; j < supportedExtensionCount; j++) {
        if (strcmp(extensions[i], supportedExtensions[j].extensionName) == 0) {
          rt_message("XGL required extension '%s' is supported", extensions[i]);
          goto __required_extension_verified;
        }
      }
      rt_warning("XGL required extension '%s' is not supported", extensions[i]);
__required_extension_verified:
    }
    allocator->free(supportedExtensions);
  }
  /* show all layers required */ {
    /* enumerate supported layers */
    uint32_t supportedLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr);
    VkLayerProperties *supportedLayers = allocator->calloc(supportedLayerCount, sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers);
    for (uint32_t i = 0; i < supportedLayerCount; i++) {
      rt_debug("Support layers '%s' of version %u(%u)", supportedLayers[i].layerName, supportedLayers->specVersion, supportedLayers->implementationVersion);
      rt_debug("%s", supportedLayers[i].description);
    }
    uint32_t requiredLayerCount = lenof(INSTANCE_REQUIRED_LAYER_NAMES);
    const char * const*requiredLayers = INSTANCE_REQUIRED_LAYER_NAMES;
    for (uint32_t i = 0; i < requiredLayerCount; i++) {
      for (uint32_t j = 0; j < supportedLayerCount; j++) {
        if (strcmp(requiredLayers[i], supportedLayers[j].layerName) == 0) {
          rt_message("XGL required layer '%s' is supported", requiredLayers[i]);
          goto __required_layer_verified;
        }
      }
      rt_warning("XGL required layer '%s' is not supported", requiredLayers[i]);
__required_layer_verified:
    }
    allocator->free(supportedLayers);
  }

  VkApplicationInfo applicationInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = "Vulkan Window",
      .applicationVersion = 0x00'00'01'00,
      .pEngineName = "X Engine",
      .engineVersion = 0x00'00'01'00,
      .apiVersion = XGL_VK_API_VERSION,
  };
  VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = 0,
      .messageSeverity = 0x0000'1111,
      .messageType = 0x0000'0007,
      .pfnUserCallback = debugCallback,
      .pUserData = nullptr,
  };
  VkInstanceCreateInfo instanceCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = &debugUtilsMessengerCreateInfo,
    .flags = 0,
    .pApplicationInfo = &applicationInfo,
    .enabledLayerCount = lenof(INSTANCE_REQUIRED_LAYER_NAMES),
    .ppEnabledLayerNames = INSTANCE_REQUIRED_LAYER_NAMES,
    .enabledExtensionCount = lenof(INSTANCE_REQUIRED_EXTENSION_NAMES),
    .ppEnabledExtensionNames = INSTANCE_REQUIRED_EXTENSION_NAMES,
  };
  VkInstance instance = {};
  result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
  if (result != VK_SUCCESS) {
    switch (result) {
      case VK_ERROR_EXTENSION_NOT_PRESENT: {
        rt_error("Not supported extension set on");
        break;
      }
      case VK_ERROR_LAYER_NOT_PRESENT: {
        rt_error("Not supported layer set on");
        break;
      }
      default: {
        rt_error("error occurred %u", result);
      }
    }
    goto __failed_to_create_vulkan_instance;
  }
  rt_message("All extensions and layers are satisfied");
  rt_message("Vulkan Instance created");

  VkSurfaceKHR surface = VK_NULL_HANDLE;
  result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create window surface: %u", result);
    goto __failed_to_create_surface;
  }

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties physicalDeviceProperties = {};
  VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
  uint32_t queueFamilyCount = 2;
  uint32_t queueFamilyIndices[2] = {};
  /* show all supported physical devices */ {
    uint32_t supportedPhysicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &supportedPhysicalDeviceCount, nullptr);
    if (supportedPhysicalDeviceCount == 0) {
      rt_error("No physical device");
      goto __failed_to_pick_physical_device;
    }
    VkPhysicalDevice *supportedPhysicalDevices = allocator->calloc(supportedPhysicalDeviceCount, sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &supportedPhysicalDeviceCount, supportedPhysicalDevices);
    for (uint32_t i = 0; i < supportedPhysicalDeviceCount; i ++) {
      vkGetPhysicalDeviceProperties(supportedPhysicalDevices[i], &physicalDeviceProperties);
      vkGetPhysicalDeviceFeatures(supportedPhysicalDevices[i], &physicalDeviceFeatures);
      rt_message("Detect Physical device '%s'", physicalDeviceProperties.deviceName);
      if (physicalDeviceProperties.apiVersion < XGL_VK_API_VERSION) {
        rt_message("Device '%s' not supports required Vulkan API, skip", physicalDeviceProperties.deviceName);
        continue;
      }
      if (physicalDeviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        rt_message("Device '%s' is not a discrete GPU, skip", physicalDeviceProperties.deviceName);
        continue;
      }
      if (!physicalDeviceFeatures.geometryShader) {
        rt_message("Device '%s' has no geometry shader, skip", physicalDeviceProperties.deviceName);
        continue;
      }
      physicalDevice = supportedPhysicalDevices[i];
      // select queue family
      uint32_t queueFamiliesCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);
      VkQueueFamilyProperties *queueFamilies = allocator->calloc(queueFamiliesCount, sizeof(VkQueueFamilyProperties));
      vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, queueFamilies);
      for (uint32_t j = 0; j < queueFamiliesCount; j++) {
        if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
          rt_message("Graphics queue family found");
          queueFamilyIndices[QUEUE_FAMILY_GRAPHICS_INDEX] = j;
          goto __graphics_queue_found;
        }
      }
      rt_message("No suitable queue family found for '%s'", physicalDeviceProperties.deviceName);
      allocator->free(queueFamilies);
      continue;
__graphics_queue_found:
      for (uint32_t j = 0; j < queueFamiliesCount; j++) {
        VkBool32 supported = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(supportedPhysicalDevices[i], j, surface, &supported);
        if (supported) {
          rt_message("Present queue family found");
          queueFamilyIndices[QUEUE_FAMILY_PRESENT_INDEX] = j;
          allocator->free(queueFamilies);
          goto __physical_device_found;
        }
      }
    }
    rt_error("No suitable physical device found");
    allocator->free(supportedPhysicalDevices);
    goto __failed_to_pick_physical_device;
__physical_device_found:
    rt_message("Using Physical Device '%s' (driver version: %u)",
               physicalDeviceProperties.deviceName,
               physicalDeviceProperties.driverVersion);
    rt_message("Queue Family selected.");
    allocator->free(supportedPhysicalDevices);
  }

  /* show all extensions required */ {
    /* enumerate supported extensions */
    uint32_t supportedExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedExtensionCount, nullptr);
    VkExtensionProperties *supportedExtensions = allocator->calloc(supportedExtensionCount, sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedExtensionCount, supportedExtensions);
    for (uint32_t i = 0; i < supportedExtensionCount; i++) {
      rt_debug("Support device extension '%s' of version %u", supportedExtensions[i].extensionName, supportedExtensions->specVersion);
    }
    uint32_t extensionCount = lenof(REQUIRED_DEVICE_EXTENSION_NAMES);
    const char * const*extensions = REQUIRED_DEVICE_EXTENSION_NAMES;
    for (uint32_t i = 0; i < extensionCount; i++) {
      for (uint32_t j = 0; j < supportedExtensionCount; j++) {
        if (strcmp(extensions[i], supportedExtensions[j].extensionName) == 0) {
          rt_message("XGL required device extension '%s' is supported", extensions[i]);
          goto __required_device_extension_verified;
        }
      }
      rt_warning("XGL required device extension '%s' is not supported", extensions[i]);
__required_device_extension_verified:
    }
    allocator->free(supportedExtensions);
  }
  /* show all layers required */ {
    /* enumerate supported layers */
    uint32_t supportedLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr);
    VkLayerProperties *supportedLayers = allocator->calloc(supportedLayerCount, sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers);
    for (uint32_t i = 0; i < supportedLayerCount; i++) {
      rt_debug("Support device layers '%s' of version %u(%u)", supportedLayers[i].layerName, supportedLayers->specVersion, supportedLayers->implementationVersion);
      rt_debug("%s", supportedLayers[i].description);
    }
    uint32_t requiredLayerCount = lenof(INSTANCE_REQUIRED_LAYER_NAMES);
    const char * const*requiredLayers = INSTANCE_REQUIRED_LAYER_NAMES;
    for (uint32_t i = 0; i < requiredLayerCount; i++) {
      for (uint32_t j = 0; j < supportedLayerCount; j++) {
        if (strcmp(requiredLayers[i], supportedLayers[j].layerName) == 0) {
          rt_message("XGL required device layer '%s' is supported", requiredLayers[i]);
          goto __required_device_layer_verified;
        }
      }
      rt_warning("XGL required device layer '%s' is not supported", requiredLayers[i]);
__required_device_layer_verified:
    }
    allocator->free(supportedLayers);
  }

  uint32_t swapImageCount = {};
  VkExtent2D surfaceExtent = {};
  VkSurfaceTransformFlagBitsKHR surfaceTransform = {};
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
  #if defined(XGL_SURFACE_WIN32)
  VkSurfaceFormatKHR surfaceFormat = { .format = VK_FORMAT_R8G8B8A8_SRGB, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR };
  #elif defined(XGL_SURFACE_WAYLAND)
  VkSurfaceFormatKHR surfaceFormat = { .format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR };
  #endif
__get_surface_capabilities:
  /* get capabilities of surface */{
    uint32_t presentModeCount = 0;
    uint32_t surfaceFormatCount = 0;
    VkPresentModeKHR *presentModes = nullptr;
    VkSurfaceFormatKHR *surfaceFormats = nullptr;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
    surfaceFormats = allocator->calloc(surfaceFormatCount, sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    presentModes = allocator->calloc(presentModeCount, sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);
    if (!presentModeCount || !surfaceFormatCount) {
      allocator->free(presentModes);
      allocator->free(surfaceFormats);
      goto __failed_to_get_capabilities_of_surface;
    }
    for (uint32_t i = 0; i < presentModeCount; i ++) {
      if (presentModes[i] == presentMode) { goto __present_mode_verified; }
    }
    rt_error("Not supported present mode: %s", "VK_PRESENT_MODE_FIFO_KHR");
    allocator->free(presentModes);
    allocator->free(surfaceFormats);
    goto __failed_to_get_capabilities_of_surface;
    __present_mode_verified:
    rt_message("Chosen present mode: %s", "VK_PRESENT_MODE_FIFO_KHR");
    for (uint32_t i = 0; i < surfaceFormatCount; i ++) {
      if (surfaceFormats[i].format != surfaceFormat.format) { continue; }
      if (surfaceFormats[i].colorSpace != surfaceFormat.colorSpace) { continue; }
      goto __surface_format_verified;
    }
    rt_error("Not supported surface format: %s", "R8G8B8A8_SRGB(ColorSpace: SRGB_NONLINEAR)");
    allocator->free(presentModes);
    allocator->free(surfaceFormats);
    goto __failed_to_get_capabilities_of_surface;
__surface_format_verified:
    rt_message("Chosen surface format: %s", "R8G8B8A8_SRGB(ColorSpace: SRGB_NONLINEAR)");
    int width = 0, height = 0;
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    glfwGetFramebufferSize(window, &width, &height);
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
    surfaceExtent.width = maxmin((uint32_t) width, surfaceCapabilities.maxImageExtent.width, surfaceCapabilities.minImageExtent.width);
    surfaceExtent.height = maxmin((uint32_t) height, surfaceCapabilities.maxImageExtent.height, surfaceCapabilities.minImageExtent.height);
    swapImageCount = maxmin(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
    surfaceTransform = surfaceCapabilities.currentTransform;
    allocator->free(presentModes);
    allocator->free(surfaceFormats);
    if (framebufferResized) {
      framebufferResized = false;
      goto __recreate_swapchain;
    }
  }

  VkDevice logicDevice = VK_NULL_HANDLE;
  VkQueue graphicsQueue = VK_NULL_HANDLE;
  VkQueue presentQueue = VK_NULL_HANDLE;
  /* create logic device and queues */ {
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[] = {
        [QUEUE_FAMILY_GRAPHICS_INDEX] = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queueFamilyIndices[QUEUE_FAMILY_GRAPHICS_INDEX],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        },
        [QUEUE_FAMILY_PRESENT_INDEX] = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queueFamilyIndices[QUEUE_FAMILY_PRESENT_INDEX],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        }
    };
    VkDeviceCreateInfo logicDeviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = (queueFamilyIndices[1] == queueFamilyIndices[0]) ? 1 : 2,
        .pQueueCreateInfos = queueCreateInfos,
        .enabledLayerCount = lenof(REQUIRED_DEVICE_LAYER_NAMES),
        .ppEnabledLayerNames = REQUIRED_DEVICE_LAYER_NAMES,
        .enabledExtensionCount = lenof(REQUIRED_DEVICE_EXTENSION_NAMES),
        .ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSION_NAMES,
        .pEnabledFeatures = &physicalDeviceFeatures,
    };
    result = vkCreateDevice(physicalDevice, &logicDeviceCreateInfo, nullptr, &logicDevice);
    if (result != VK_SUCCESS) {
      switch (result) {
        case VK_ERROR_EXTENSION_NOT_PRESENT: {
          rt_error("Not supported extension set on");
          break;
        }
        case VK_ERROR_LAYER_NOT_PRESENT: {
          rt_error("Not supported layer set on");
          break;
        }
        default: {
          rt_error("error occurred %u", result);
        }
      }
      goto __failed_to_create_logic_device;
    }
    rt_message("Logic device created");
    vkGetDeviceQueue(logicDevice, queueFamilyIndices[QUEUE_FAMILY_GRAPHICS_INDEX], 0, &graphicsQueue);
    rt_message("Graphics queue selected");
    vkGetDeviceQueue(logicDevice, queueFamilyIndices[QUEUE_FAMILY_PRESENT_INDEX], 0, &presentQueue);
    rt_message("Present queue selected");
  }

  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = (float) surfaceExtent.width,
      .height = (float) surfaceExtent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  VkRect2D scissor = {
      .offset = {.x = 0, .y = 0}, .extent = surfaceExtent
  };
  VkShaderModule vertShaderModule =  VK_NULL_HANDLE;
  VkShaderModule fragShaderModule =  VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkRenderPass pipelineRenderPass = VK_NULL_HANDLE;
  VkPipeline pipeline = VK_NULL_HANDLE;
  /* create graphics pipeline */ {
    uint32_t vertShaderCodeSize = 0;
    uint32_t fragShaderCodeSize = 0;
    uint8_t *vertShaderCode = loadBinFile("shaders/vert.spv", &vertShaderCodeSize, allocator);
    uint8_t *fragShaderCode = loadBinFile("shaders/frag.spv", &fragShaderCodeSize, allocator);
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = 0,
        .pCode = nullptr,
    };
    shaderModuleCreateInfo.codeSize = vertShaderCodeSize;
    shaderModuleCreateInfo.pCode = (uint32_t *) vertShaderCode;
    result = vkCreateShaderModule(logicDevice, &shaderModuleCreateInfo, nullptr, &vertShaderModule);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create vertex shader module");
      goto __failed_to_create_vertex_shader_module;
    }
    shaderModuleCreateInfo.codeSize = fragShaderCodeSize;
    shaderModuleCreateInfo.pCode = (uint32_t *) fragShaderCode;
    result = vkCreateShaderModule(logicDevice, &shaderModuleCreateInfo, nullptr, &fragShaderModule);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create fragment shader module");
      goto __failed_to_create_fragment_shader_module;
    }
    VkPipelineShaderStageCreateInfo pipelineStageCreateInfos[] = {
        {
               .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
               .pNext = nullptr,
               .flags = 0,
               .stage = VK_SHADER_STAGE_VERTEX_BIT,
               .module = vertShaderModule,
               .pName = "main",
               .pSpecializationInfo = nullptr,
           },
        {
               .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
               .pNext = nullptr,
               .flags = 0,
               .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
               .module = fragShaderModule,
               .pName = "main",
               .pSpecializationInfo = nullptr,
           },
    };
    uint32_t pipelineStageCreateInfoCount = lenof(pipelineStageCreateInfos);
    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };
    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    uint32_t dynamicStateCount = lenof(dynamicStates);
    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = dynamicStateCount,
        .pDynamicStates = dynamicStates,
    };
    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };
    VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0,
        .depthBiasClamp = 0,
        .depthBiasSlopeFactor = 0,
        .lineWidth = 1.0f,
    };
    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };
    VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
    VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        #if defined(XGL_SURFACE_WIN32)
        .logicOpEnable = VK_TRUE,
        .logicOp = VK_LOGIC_OP_COPY,
        #elif defined(XGL_SURFACE_WAYLAND)
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_CLEAR,
        #endif
        .attachmentCount = 1,
        .pAttachments = &pipelineColorBlendAttachmentState,
        .blendConstants = {1.0f, 1.0f, 1.0f, 1.0f},
    };
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };
    result = vkCreatePipelineLayout(logicDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create pipeline layout");
      goto __failed_to_create_pipeline_layout;
    }
    VkAttachmentDescription colorAttachmentDescription = {
        .flags = 0,
        .format = surfaceFormat.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentReference colorAttachmentReference = {
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkSubpassDescription subpassDescription = {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReference,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    };
    VkSubpassDependency subpassDependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0,
    };
    VkRenderPassCreateInfo renderPassCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &colorAttachmentDescription,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency,
    };
    result = vkCreateRenderPass(logicDevice, &renderPassCreateInfo, nullptr, &pipelineRenderPass);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create render pass");
      goto __failed_to_create_render_pass;
    }
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = pipelineStageCreateInfoCount,
        .pStages = pipelineStageCreateInfos,
        .pVertexInputState = &pipelineVertexInputStateCreateInfo,
        .pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &pipelineViewportStateCreateInfo,
        .pRasterizationState = &pipelineRasterizationStateCreateInfo,
        .pMultisampleState = &pipelineMultisampleStateCreateInfo,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &pipelineColorBlendStateCreateInfo,
        .pDynamicState = &pipelineDynamicStateCreateInfo,
        .layout = pipelineLayout,
        .renderPass = pipelineRenderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };
    result = vkCreateGraphicsPipelines(logicDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create graphics pipeline");
      goto __failed_to_create_graphics_pipeline;
    }
  }

  VkCommandPool commandPool = VK_NULL_HANDLE;
  VkCommandBuffer commandBuffer[MAX_FRAME_ON_DRAW] = { VK_NULL_HANDLE };
  /* create command pool and command buffer */ {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndices[QUEUE_FAMILY_GRAPHICS_INDEX],
    };
    result = vkCreateCommandPool(logicDevice, &commandPoolCreateInfo, nullptr, &commandPool);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create command pool");
      goto __failed_to_create_command_pool;
    }
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAME_ON_DRAW,
    };
    result = vkAllocateCommandBuffers(logicDevice, &commandBufferAllocateInfo, commandBuffer);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create command buffer");
      goto __failed_to_create_command_buffer;
    }
  }

  VkSemaphore imageAvailableSemaphore[MAX_FRAME_ON_DRAW] = { VK_NULL_HANDLE };
  VkSemaphore renderCompletedSemaphore[MAX_FRAME_ON_DRAW] = { VK_NULL_HANDLE };
  VkFence     presentCompletedFence[MAX_FRAME_ON_DRAW] = { VK_NULL_HANDLE };
  /* create sync objects */ {
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr, .flags = 0,
    };
    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr, .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    for (uint32_t i = 0; i < MAX_FRAME_ON_DRAW; i ++) {
      result = vkCreateSemaphore(logicDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore[i]);
      if (result != VK_SUCCESS) {
        rt_error("Failed to create image semaphore");
        goto __failed_to_create_sync_objects;
      }
      result = vkCreateSemaphore(logicDevice, &semaphoreCreateInfo, nullptr, &renderCompletedSemaphore[i]);
      if (result != VK_SUCCESS) {
        rt_error("Failed to create render semaphore");
        goto __failed_to_create_sync_objects;
      }
      result = vkCreateFence(logicDevice, &fenceCreateInfo, nullptr, &presentCompletedFence[i]);
      if (result != VK_SUCCESS) {
        rt_error("Failed to create draw fence");
        goto __failed_to_create_sync_objects;
      }
    }
  }

  uint32_t currentFrameIndex = 0;
  VkImageView *swapImageViews = nullptr;
  VkFramebuffer *framebuffers = nullptr;
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;
__create_swapchain:
  /* create swapchain, images, image views, and framebuffers */ {
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface,
        .minImageCount = swapImageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = surfaceExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = (queueFamilyIndices[0] == queueFamilyIndices[1]) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = (queueFamilyIndices[0] == queueFamilyIndices[1]) ? 0 : queueFamilyCount,
        .pQueueFamilyIndices = (queueFamilyIndices[0] == queueFamilyIndices[1]) ? nullptr : queueFamilyIndices,
        .preTransform = surfaceTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapchain,
    };
    result = vkCreateSwapchainKHR(logicDevice, &swapchainCreateInfo, nullptr, &swapchain);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create swapchain: %u", result);
      goto __failed_to_create_swapchain;
    }
    vkGetSwapchainImagesKHR(logicDevice, swapchain, &swapImageCount, nullptr);
    VkImage *swapImages = allocator->calloc(swapImageCount, sizeof(VkImage));
    vkGetSwapchainImagesKHR(logicDevice, swapchain, &swapImageCount, swapImages);
    swapImageViews = allocator->calloc(swapImageCount, sizeof(VkImageView));
    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = nullptr,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = surfaceFormat.format,
        .components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    for (uint32_t i = 0; i < swapImageCount; i++) {
      imageViewCreateInfo.image = swapImages[i];
      result = vkCreateImageView(logicDevice, &imageViewCreateInfo, nullptr, &swapImageViews[i]);
      if (result != VK_SUCCESS) {
        rt_error("Failed to create image view: %u", result);
        for (uint32_t j = 0; j < i; j++) {
          vkDestroyImageView(logicDevice, swapImageViews[j], nullptr);
        }
        allocator->free(swapImages);
        allocator->free(swapImageViews);
        goto __failed_to_create_image_views;
      }
    }
    framebuffers = allocator->calloc(swapImageCount, sizeof(VkFramebuffer));
    for (uint32_t i = 0; i < swapImageCount; i++) {
      VkFramebufferCreateInfo framebufferCreateInfo = {
          .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .renderPass = pipelineRenderPass,
          .attachmentCount = 1,
          .pAttachments = &swapImageViews[i],
          .width = surfaceExtent.width,
          .height = surfaceExtent.height,
          .layers = 1,
      };
      result = vkCreateFramebuffer(logicDevice, &framebufferCreateInfo, nullptr, &framebuffers[i]);
      if (result != VK_SUCCESS) {
        rt_error("Failed to create framebuffers");
        for (uint32_t j = 0; j < i; j++) {
          vkDestroyFramebuffer(logicDevice, framebuffers[j], nullptr);
        }
        allocator->free(framebuffers);
        goto __failed_to_create_framebuffers;
      }
    }
    allocator->free(swapImages);
  }

  while(!glfwWindowShouldClose(window)) {
    processInput(window);
    glfwPollEvents();
    /* draw */ {
      vkWaitForFences(logicDevice, 1, &presentCompletedFence[currentFrameIndex], VK_TRUE, UINT32_MAX);

      if (framebufferResized) {
        goto __get_surface_capabilities;
      }
      uint32_t imageIndex = 0;
      result = vkAcquireNextImageKHR(logicDevice, swapchain, UINT32_MAX, imageAvailableSemaphore[currentFrameIndex], VK_NULL_HANDLE, &imageIndex);
      if (result != VK_SUCCESS) {
        switch (result) {
          case VK_ERROR_OUT_OF_DATE_KHR: {
            goto __recreate_swapchain;
          }
          default:{ glfwSetWindowShouldClose(window, GLFW_TRUE); }
        }
      }

      vkResetFences(logicDevice, 1, &presentCompletedFence[currentFrameIndex]);

      vkResetCommandBuffer(commandBuffer[currentFrameIndex], 0);
      VkCommandBufferBeginInfo commandBufferBeginInfo = {
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
          .pNext = nullptr, .flags = 0,
          .pInheritanceInfo = nullptr,
      };
      result = vkBeginCommandBuffer(commandBuffer[currentFrameIndex], &commandBufferBeginInfo);
      if (result != VK_SUCCESS) {
        rt_error("Failed to begin command buffer");
        goto __failed_to_render;
      }
      /* command buffer record */ {
        VkClearValue clearValue = {.color = {0.02f, 0.02f, 0.02f, 1.0f}};
        VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = pipelineRenderPass,
            .framebuffer = framebuffers[imageIndex],
            .renderArea = {.offset = {0, 0}, .extent = surfaceExtent},
            .clearValueCount = 1,
            .pClearValues = &clearValue,
        };
        vkCmdBeginRenderPass(commandBuffer[currentFrameIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer[currentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdSetViewport(commandBuffer[currentFrameIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer[currentFrameIndex], 0, 1, &scissor);
        vkCmdDraw(commandBuffer[currentFrameIndex], 3, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffer[currentFrameIndex]);
        result = vkEndCommandBuffer(commandBuffer[currentFrameIndex]);
        if (result != VK_SUCCESS) {
          rt_message("Failed to record command buffer");
          goto __failed_to_render;
        }
      }
      VkSemaphore signaledSemaphores[] = {renderCompletedSemaphore[currentFrameIndex]};
      VkSemaphore waiteSemaphores[] = {imageAvailableSemaphore[currentFrameIndex]};
      VkPipelineStageFlags waiteStageFlags[] = {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT};
      VkSubmitInfo submitInfo = {
          .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
          .pNext = nullptr,
          .waitSemaphoreCount = lenof(waiteSemaphores),
          .pWaitSemaphores = waiteSemaphores,
          .pWaitDstStageMask = waiteStageFlags,
          .commandBufferCount = 1,
          .pCommandBuffers = &commandBuffer[currentFrameIndex],
          .signalSemaphoreCount = lenof(signaledSemaphores),
          .pSignalSemaphores = signaledSemaphores,
      };
      result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, presentCompletedFence[currentFrameIndex]);
      if (result != VK_SUCCESS) {
        rt_error("Failed to submit render command to queue");
        goto __failed_to_render;
      }
      VkPresentInfoKHR presentInfo = {
          .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
          .pNext = nullptr,
          .waitSemaphoreCount = 1,
          .pWaitSemaphores = signaledSemaphores,
          .swapchainCount = 1,
          .pSwapchains = &swapchain,
          .pImageIndices = &imageIndex,
          .pResults = nullptr,
      };
      result = vkQueuePresentKHR(presentQueue, &presentInfo);
      if (result != VK_SUCCESS) {
        switch (result) {
          case VK_SUBOPTIMAL_KHR:
          case VK_ERROR_OUT_OF_DATE_KHR: {
            goto __recreate_swapchain;
          }
          default:{ glfwSetWindowShouldClose(window, GLFW_TRUE); }
        }
      }

      currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAME_ON_DRAW;
    }
  }

  vkDeviceWaitIdle(logicDevice);
__failed_to_render:
  for (uint32_t i = 0; i < MAX_FRAME_ON_DRAW; i++) {
    vkDestroySemaphore(logicDevice, imageAvailableSemaphore[i], nullptr);
    vkDestroySemaphore(logicDevice, renderCompletedSemaphore[i], nullptr);
    vkDestroyFence(logicDevice, presentCompletedFence[i], nullptr);
  }
__failed_to_create_sync_objects:
__failed_to_create_command_buffer:
  vkDestroyCommandPool(logicDevice, commandPool, nullptr);
__failed_to_create_command_pool:
  for (uint32_t j = 0; j < swapImageCount; j ++) {
    vkDestroyFramebuffer(logicDevice, framebuffers[j], nullptr);
  }
  allocator->free(framebuffers);
__failed_to_create_framebuffers:
  vkDestroyPipeline(logicDevice, pipeline, nullptr);
__failed_to_create_graphics_pipeline:
  vkDestroyRenderPass(logicDevice, pipelineRenderPass, nullptr);
__failed_to_create_render_pass:
  vkDestroyPipelineLayout(logicDevice, pipelineLayout, nullptr);
__failed_to_create_pipeline_layout:
  vkDestroyShaderModule(logicDevice, fragShaderModule, nullptr);
__failed_to_create_fragment_shader_module:
  vkDestroyShaderModule(logicDevice, vertShaderModule, nullptr);
__failed_to_create_vertex_shader_module:
  for (uint32_t j = 0; j < swapImageCount; j ++) {
    vkDestroyImageView(logicDevice, swapImageViews[j], nullptr);
  }
  allocator->free(swapImageViews);
__failed_to_create_image_views:
  vkDestroySwapchainKHR(logicDevice, swapchain, nullptr);
__failed_to_create_swapchain:
  vkDestroyDevice(logicDevice, nullptr);
__failed_to_create_logic_device:
__failed_to_get_capabilities_of_surface:
__failed_to_pick_physical_device:
  vkDestroySurfaceKHR(instance, surface, nullptr);
__failed_to_create_surface:
  vkDestroyInstance(instance, nullptr);
__failed_to_create_vulkan_instance:
  glfwDestroyWindow(window);
__failed_to_create_glfw_window:
  glfwTerminate();

  return result;

__recreate_swapchain:
  vkDeviceWaitIdle(logicDevice);
  for (uint32_t j = 0; j < swapImageCount; j ++) {
    vkDestroyFramebuffer(logicDevice, framebuffers[j], nullptr);
  }
  for (uint32_t j = 0; j < swapImageCount; j ++) {
    vkDestroyImageView(logicDevice, swapImageViews[j], nullptr);
  }
  vkDestroySwapchainKHR(logicDevice, swapchain, nullptr);
  goto __create_swapchain;
}

void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {

  framebufferResized = true;
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

uint8_t *loadBinFile(const char *filepath, uint32_t *real_size, const Allocator *allocator) {
  uint32_t fileSize = 0;
  FILE *file = nullptr;
  file = fopen(filepath, "rb");
  if (!file) {
    rt_error("Failed to open binary file '%s'", filepath);
    return nullptr;
  }
  fseek(file, 0, SEEK_END);
  fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  uint8_t *content = allocator->calloc(((fileSize - 1) / sizeof(uint32_t) + 1), sizeof(uint32_t));
  *real_size = fread(content, sizeof(uint8_t), fileSize, file);
  return content;
}


const char *MESSAGE_TYPES[] = {
    "General", "Validation", "Performance", "DeviceAddressBinding"

};

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
  if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT) {
    messageType = 3;
  } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
    messageType = 2;
  } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
    messageType = 1;
  } else {
//    messageType = 0;
    return VK_FALSE;
  }
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    rt_debug("(%s) %s", MESSAGE_TYPES[messageType], pCallbackData->pMessage);
  }
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    rt_message("(%s) %s", MESSAGE_TYPES[messageType], pCallbackData->pMessage);
  }
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    rt_warning("(%s) %s", MESSAGE_TYPES[messageType], pCallbackData->pMessage);
  }
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    rt_error("(%s) %s", MESSAGE_TYPES[messageType], pCallbackData->pMessage);
  }
  return VK_FALSE;
}
