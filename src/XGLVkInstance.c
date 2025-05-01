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
 * Project Name: VulkanDemo
 * Module Name: src
 * Filename: XGLVkInstance.c
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include <string.h>
#include "XGLVkInstance.h"
#include "XGLVkPhysicalDevice.h"
#include "util-macro.h"
#include "callback.h"
#include "runtime-msg.h"

const char* INSTANCE_REQUIRED_EXTENSION_NAMES[] = {
    /** @{ glfw required extensions */
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(XGL_SURFACE_WIN32)
    "VK_KHR_win32_surface",
#elif defined(XGL_SURFACE_WAYLAND)
    "VK_KHR_wayland_surface",
#else
#error "Unknown os platform, unable to set window handle."
#endif
    /** @} */
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
//    VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME,
};

const char* INSTANCE_REQUIRED_LAYER_NAMES[] = {
    "VK_LAYER_KHRONOS_validation",
};

void XGLVkInstance_enumerateLayers(XGLVkInstance *instance);
void XGLVkInstance_enumerateExtensions(XGLVkInstance *instance);
void XGLVkInstance_enumeratePhysicalDevices(XGLVkInstance *instance);

XGLVkInstance *XGLVkInstance_new(const XGLSoftware *app, const XGLSoftware *engine, const Allocator *allocator) {
  VkResult result;
  XGLVkInstance *instance = allocator->calloc(1, sizeof(XGLVkInstance));
  instance->enableValidationLayers = true;
  instance->apiVersion = XGL_VK_API_VERSION;
  instance->allocator = allocator;
  instance->engine = *engine;
  instance->app = *app;
  XGLVkInstance_enumerateLayers(instance);
  XGLVkInstance_enumerateExtensions(instance);
  result = XGLVkInstance_verifyLayers(instance,
                                      lenof(INSTANCE_REQUIRED_LAYER_NAMES),
                                      INSTANCE_REQUIRED_LAYER_NAMES);
  if (result != VK_SUCCESS) {
    XGLVkInstance_destroy(instance);
    return nullptr;
  }
  result = XGLVkInstance_verifyExtensions(instance,
                                          lenof(INSTANCE_REQUIRED_EXTENSION_NAMES),
                                          INSTANCE_REQUIRED_EXTENSION_NAMES);
  if (result != VK_SUCCESS) {
    XGLVkInstance_destroy(instance);
    return nullptr;
  }

  VkApplicationInfo applicationInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = app->name,
      .applicationVersion = app->version,
      .pEngineName = engine->name,
      .engineVersion = engine->version,
      .apiVersion = instance->apiVersion,
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
      .pNext = instance->enableValidationLayers ? &debugUtilsMessengerCreateInfo : nullptr,
      .flags = 0,
      .pApplicationInfo = &applicationInfo,
      .enabledLayerCount = lenof(INSTANCE_REQUIRED_LAYER_NAMES),
      .ppEnabledLayerNames = INSTANCE_REQUIRED_LAYER_NAMES,
      .enabledExtensionCount = lenof(INSTANCE_REQUIRED_EXTENSION_NAMES),
      .ppEnabledExtensionNames = INSTANCE_REQUIRED_EXTENSION_NAMES,
  };
  result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance->handle);
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
    return nullptr;
  }
  rt_message("All extensions and layers are satisfied");
  rt_message("Vulkan Instance created");

  XGLVkInstance_enumeratePhysicalDevices(instance);

  return instance;
}

VkResult XGLVkInstance_verifyLayers(XGLVkInstance *instance, uint32_t layerCount, const char *layerNames[]) {
  VkResult result = VK_SUCCESS;
  uint32_t supportedCount = Array_length(instance->layers);
  XGLVkLayer *layers = Array_first_real(instance->layers);
  for (uint32_t i = 0; i < layerCount; i++) {
    for (uint32_t j = 0; j < supportedCount; j++) {
      if (strcmp(layers[j].properties.layerName, layerNames[i]) == 0) {
        rt_message("Requested layer '%s' found", layerNames[i]);
        goto __layer_verified;
      }
    }
    rt_warning("Failed to find requested layer '%s'", layerNames[i]);
    result = VK_ERROR_LAYER_NOT_PRESENT;
    __layer_verified:
  }
  return result;
}

VkResult XGLVkInstance_verifyExtensions(XGLVkInstance *instance, uint32_t extensionCount, const char *extensionNames[]) {
  VkResult result = VK_SUCCESS;
  uint32_t layerCount = Array_length(instance->layers);
  XGLVkLayer *layers = Array_first_real(instance->layers);
  for (uint32_t i = 0; i < extensionCount; i ++) {
    for (uint32_t j = 0; j < layerCount; j++) {
      for (uint32_t k = 0; k < layers[j].extensionCount; k ++) {
        const char* extensionName = layers[j].extensions[k].extensionName;
        if (strcmp(extensionName, extensionNames[i]) == 0) {
          rt_message("Requested instance extension '%s' found in layer '%s'", extensionNames[i], layers[j].properties.layerName);
          goto __extension_verified;
        }
      }
    }
    uint32_t extCount = Array_length(instance->extensions);
    VkExtensionProperties *extProps = Array_first_real(instance->extensions);
    for (uint32_t j = 0; j < extCount; j ++) {
      if (strcmp(extProps[j].extensionName, extensionNames[i]) == 0) {
        rt_message("Requested instance extension '%s' found", extensionNames[i]);
        goto __extension_verified;
      }
    }
    rt_warning("Failed to find requested instance extension '%s'", extensionNames[i]);
    result = VK_ERROR_EXTENSION_NOT_PRESENT;
    __extension_verified:
  }
  return result;
}


void XGLVkInstance_enumerateLayers(XGLVkInstance *instance) {
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  VkLayerProperties*layerProps = instance->allocator->calloc(layerCount, sizeof(VkLayerProperties));
  vkEnumerateInstanceLayerProperties(&layerCount, layerProps);
  if (!instance->layers) {
    instance->layers = Array_new(sizeof(XGLVkLayer), -1, instance->allocator);
  }
  Array_resize(instance->layers, layerCount, nullptr);
  XGLVkLayer *layers = Array_first_real(instance->layers);
  for (uint32_t i = 0; i < layerCount; i++) {
    layers[i].extensionCount = 0;
    layers[i].properties = layerProps[i];
    vkEnumerateInstanceExtensionProperties(layerProps[i].layerName, &layers[i].extensionCount, nullptr);
    layers[i].extensions = instance->allocator->calloc(layers[i].extensionCount, sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(layerProps[i].layerName, &layers[i].extensionCount, layers[i].extensions);
  }
  instance->allocator->free(layerProps);
}

void XGLVkInstance_enumerateExtensions(XGLVkInstance *instance) {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  if (!instance->extensions) {
    instance->extensions = Array_new(sizeof(VkExtensionProperties), -1, instance->allocator);
  }
  Array_resize(instance->extensions, extensionCount, nullptr);
  VkExtensionProperties *extensionProps = Array_first_real(instance->extensions);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProps);
}

void XGLVkInstance_enumeratePhysicalDevices(XGLVkInstance *instance) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance->handle, &deviceCount, nullptr);
  if (deviceCount == 0) {
    rt_error("No physical handle found");
  }
  VkPhysicalDevice *devices = instance->allocator->calloc(deviceCount, sizeof(VkPhysicalDevice));
  vkEnumeratePhysicalDevices(instance->handle, &deviceCount, devices);

  if (!instance->devices) {
    instance->devices = Array_new(sizeof(XGLVkPhysicalDevice), -1, instance->allocator);
  }
  Array_resize(instance->devices, deviceCount, nullptr);
  XGLVkPhysicalDevice *xglDevices = Array_first_real(instance->devices);
  for (uint32_t i = 0; i < deviceCount; i ++) {
    xglDevices[i].handle = devices[i];
    xglDevices[i].allocator = instance->allocator;
    vkGetPhysicalDeviceProperties(devices[i], &xglDevices[i].properties);
    vkGetPhysicalDeviceFeatures(devices[i], &xglDevices[i].features);
    rt_message("Physical Device '%s'(driver version: %u) found",
               xglDevices[i].properties.deviceName,
               xglDevices[i].properties.driverVersion);
  }
  instance->allocator->free(devices);
}

void XGLVkInstance_destroy(XGLVkInstance *instance) {
  if (instance->enableValidationLayers) {
//    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  }
  if (instance->layers) {
    Array_reset(instance->layers, (destruct_t *) XGLVkLayer_release);
    Array_destroy(instance->layers);
  }
  if (instance->devices) {
    Array_reset(instance->devices, (destruct_t *) XGLVkPhysicalDevice_release);
    Array_destroy(instance->devices);
  }
  if (instance->handle != VK_NULL_HANDLE) {
    vkDestroyInstance(instance->handle, nullptr);
  }
}
