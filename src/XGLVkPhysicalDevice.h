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
 * Filename: XLGVkPhysicalDevice.h
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#ifndef VULKAN_DEMO_XGL_VK_PHYSICAL_DEVICE_H
#define VULKAN_DEMO_XGL_VK_PHYSICAL_DEVICE_H

#include "XGLVulkan.h"

typedef struct XGLVkPhysicalDevice {
  VkPhysicalDevice handle;
  const Allocator *allocator;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  Array *layers; // Array<XGLVkLayer>
  Array *extensions; // Array<VkExtensionProperties>
  Array *queueFamilies; // Array<VkQueueFamilyProperties>
} XGLVkPhysicalDevice;

void XGLVkPhysicalDevice_enumerateLayers(XGLVkPhysicalDevice *device);
void XGLVkPhysicalDevice_enumerateExtensions(XGLVkPhysicalDevice *device);
void XGLVkPhysicalDevice_enumerateQueueFamilies(XGLVkPhysicalDevice *device);

void XGLVkPhysicalDevice_release(XGLVkPhysicalDevice *device, const Allocator *allocator);

VkResult XGLVkPhysicalDevice_verifyLayers(XGLVkPhysicalDevice *device, uint32_t layerCount, const char *layerNames[]);
VkResult XGLVkPhysicalDevice_verifyExtensions(XGLVkPhysicalDevice *device, uint32_t extensionCount, const char *extensionNames[]);
VkResult XGLVkPhysicalDevice_detectWindow(const XGLVkPhysicalDevice *device, XGLWMWindow *window,
                                          const XGLVkInstance *instance);


#endif //VULKAN_DEMO_XGL_VK_PHYSICAL_DEVICE_H
