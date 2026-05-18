/* License
 *
 * ${PROJ_DESCRIPTION}
 * Copyright (C) 2025 Yaokai Liu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 * Project Name: xGL
 * Module Name: xGLVulkan
 * Filename: XLGVkPhysicalDevice.c
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkPhysicalDevice.h"
#include "runtime-msg.h"
#if defined(XGL_WM_USING_GLFW)
  #include "GLFW/glfw3.h"
#else
#error "Please implement other window manager compatibility functions."
#endif
#include "XGLVkInstance.h"
#include <string.h>

void XGLVkPhysicalDevice_enumerateLayers(XGLVkPhyDevice *device);
void XGLVkPhysicalDevice_enumerateExtensions(XGLVkPhyDevice *device);
void XGLVkPhysicalDevice_enumerateQueueFamilies(XGLVkPhyDevice *device);

bool XGLVkPhysicalDevice_suitable(const XGLVkPhyDevice *device) {
  if (device->properties.apiVersion < XGL_VK_API_VERSION) {
    rt_message("Device '%s' not supports required Vulkan API", device->properties.deviceName);
    return false;
  }
  if (device->properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    rt_message("Device '%s' is not a discrete GPU", device->properties.deviceName);
    return false;
  }
  if (!device->features.geometryShader) {
    rt_message("Device '%s' has no geometry shader", device->properties.deviceName);
    return false;
  }
  return true;
}

void XGLVkPhysicalDevice_enumerate(XGLVkPhyDevice *device) {
  XGLVkPhysicalDevice_enumerateLayers(device);
  XGLVkPhysicalDevice_enumerateExtensions(device);
  XGLVkPhysicalDevice_enumerateQueueFamilies(device);
  vkGetPhysicalDeviceMemoryProperties(device->handle, &device->memoryProperties);
}

void XGLVkPhysicalDevice_enumerateQueueFamilies(XGLVkPhyDevice *device) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device->handle, &queueFamilyCount, nullptr);
  if (!device->queueFamilies) {
    device->queueFamilies = Array_new(sizeof(VkQueueFamilyProperties), -1, device->allocator);
  }
  Array_resize(device->queueFamilies, queueFamilyCount, nullptr);
  VkQueueFamilyProperties *queueFamilies = Array_first_real(device->queueFamilies);
  vkGetPhysicalDeviceQueueFamilyProperties(device->handle, &queueFamilyCount, queueFamilies);
}

void XGLVkPhysicalDevice_enumerateLayers(XGLVkPhyDevice *device) {
  uint32_t layerCount = 0;
  vkEnumerateDeviceLayerProperties(device->handle, &layerCount, nullptr);
  VkLayerProperties *layerProps = device->allocator->calloc(layerCount, sizeof(VkLayerProperties));
  vkEnumerateDeviceLayerProperties(device->handle, &layerCount, layerProps);
  if (!device->layers) {
    device->layers = Array_new(sizeof(XGLVkLayer), -1, device->allocator);
  }
  Array_resize(device->layers, layerCount, nullptr);
  XGLVkLayer *layers = Array_first_real(device->layers);
  for (uint32_t i = 0; i < layerCount; i++) {
    layers[i].extensionCount = 0;
    layers[i].properties = layerProps[i];
    vkEnumerateDeviceExtensionProperties(device->handle, layers[i].properties.layerName, &layers[i].extensionCount, nullptr);
    layers[i].extensions = device->allocator->calloc(layers[i].extensionCount, sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device->handle, layers[i].properties.layerName, &layers[i].extensionCount, layers[i].extensions);
  }
  device->allocator->free(layerProps);
}

void XGLVkPhysicalDevice_enumerateExtensions(XGLVkPhyDevice *device) {
  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(device->handle, nullptr, &extensionCount, nullptr);
  if (!device->extensions) {
    device->extensions = Array_new(sizeof(VkExtensionProperties), -1, device->allocator);
  }
  Array_resize(device->extensions, extensionCount, nullptr);
  VkExtensionProperties *extensionProps = Array_first_real(device->extensions);
  vkEnumerateDeviceExtensionProperties(device->handle, nullptr, &extensionCount, extensionProps);
}

VkResult XGLVkPhysicalDevice_verifyLayers(XGLVkPhyDevice *device, uint32_t layerCount, const char *layerNames[]) {
  VkResult result = VK_SUCCESS;
  uint32_t supportedCount = Array_length(device->layers);
  XGLVkLayer *layers = Array_first_real(device->layers);
  for (uint32_t i = 0; i < layerCount; i++) {
    for (uint32_t j = 0; j < supportedCount; j++) {
      if (strcmp(layers[j].properties.layerName, layerNames[i]) == 0) {
        rt_message("Requested handle layer '%s' found", layerNames[i]);
        goto __layer_verified;
      }
    }
    rt_warning("Failed to find requested handle layer '%s'", layerNames[i]);
    result = VK_ERROR_LAYER_NOT_PRESENT;
    __layer_verified:
  }
  return result;
}

VkResult XGLVkPhysicalDevice_verifyExtensions(XGLVkPhyDevice *device, uint32_t extensionCount, const char *extensionNames[]) {
  VkResult result = VK_SUCCESS;
  uint32_t layerCount = Array_length(device->layers);
  XGLVkLayer *layers = Array_first_real(device->layers);
  for (uint32_t i = 0; i < extensionCount; i ++) {
    for (uint32_t j = 0; j < layerCount; j++) {
      for (uint32_t k = 0; k < layers[j].extensionCount; k ++) {
        const char* extensionName = layers[j].extensions[k].extensionName;
        if (strcmp(extensionName, extensionNames[i]) == 0) {
          rt_message("Requested device extension '%s' found in layer '%s'", extensionNames[i], layers[j].properties.layerName);
          goto __extension_verified;
        }
      }
    }
    uint32_t extCount = Array_length(device->extensions);
    VkExtensionProperties *extProps = Array_first_real(device->extensions);
    for (uint32_t j = 0; j < extCount; j ++) {
      if (strcmp(extProps[j].extensionName, extensionNames[i]) == 0) {
        rt_message("Requested device extension '%s' found", extensionNames[i]);
        goto __extension_verified;
      }
    }
    rt_warning("Failed to find requested device extension '%s'", extensionNames[i]);
    result = VK_ERROR_EXTENSION_NOT_PRESENT;
    __extension_verified:
  }
  return result;
}

VkResult XGLVkPhysicalDevice_detectWindow(const XGLVkPhyDevice *device,
                                          XGLWMWindow *window,
                                          const XGLVkInstance *instance) {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
#if defined(XGL_WM_USING_GLFW)
  VkResult result = glfwCreateWindowSurface(instance->handle, window, nullptr, &surface);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create window handle: %u", result);
    return ~VK_SUCCESS;
  }
#else
  #error "Please implement other window manager compatibility functions."
#endif
  uint32_t queueFamilyCount = Array_length(device->queueFamilies);
  for (uint32_t j = 0; j < queueFamilyCount; j++) {
    VkBool32 supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device->handle, j, surface, &supported);
    if (supported) {
      vkDestroySurfaceKHR(instance->handle, surface, nullptr);
      return VK_SUCCESS;
    }
  }
  rt_message("Device '%s' is not linked to the current monitor, "
             "failed to detect the current window", device->properties.deviceName);
  vkDestroySurfaceKHR(instance->handle, surface, nullptr);
  return ~VK_SUCCESS;
}

void XGLVkPhysicalDevice_release(XGLVkPhyDevice *device, const Allocator *) {
  if (device->queueFamilies) {
    releasePrimeArray(device->queueFamilies);
  }
  if (device->layers) {
    Array_reset(device->layers, (destruct_t *) XGLVkLayer_release);
    Array_destroy(device->layers);
  }
}

uint32_t XGLVkPhysicalDevice_findMemType(const XGLVkPhyDevice *device,
                                         const VkMemoryPropertyFlags property,
                                         const uint32_t typeFilter) {
  auto props = device->memoryProperties;
  for (uint32_t i = 0; i < props.memoryTypeCount; i++) {
    if ((props.memoryTypes[i].propertyFlags & property) != property) { continue; }
    if (typeFilter & (1 << i)) { return i; }
  }
  return UINT32_MAX;
}