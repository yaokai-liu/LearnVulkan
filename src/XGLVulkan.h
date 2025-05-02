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
 * Filename: XGLVulkan.h
 * Creator: Yaokai Liu
 * Create Date: 2025-04-29
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#ifndef VULKAN_DEMO_VULKAN_INSTANCE_H
#define VULKAN_DEMO_VULKAN_INSTANCE_H

#include <vulkan/vulkan.h>
#include "allocator.h"
#include "array.h"
#if defined(XGL_SURFACE_WIN32)
#define XGL_VK_API_VERSION VK_API_VERSION_1_3
#elif defined(XGL_SURFACE_WAYLAND)
#define XGL_VK_API_VERSION VK_API_VERSION_1_2
#else
#error "Unknown os platform, unable to set window handle."
#endif

#define GRAPHICS_QUEUE_INDEX 0
#define PRESENT_QUEUE_INDEX  1
#define TRANSFER_QUEUE_INDEX 2
#define TOTAL_QUEUE_TYPE_COUNT 3

#define MAX_FRAME_ON_DRAW   2

#if defined(XGL_WM_USING_GLFW)
  typedef struct GLFWwindow XGLWMWindow;
#else
  #error "Please implement other window manager compatibility functions."
#endif
typedef struct XGLVkInstance XGLVkInstance;
typedef struct XGLVkPhysicalDevice XGLVkPhysicalDevice;
typedef struct XGLVkSurface XGLVkSurface;
typedef struct XGLVkDevice XGLVkDevice;
typedef struct XGLVkQueue XGLVkQueue;
typedef struct XGLVkCommandPool XGLVkCommandPool;
typedef struct XGLVkPipeline XGLVkPipeline;
typedef struct XGLVkSwapchain XGLVkSwapchain;
typedef struct XGLVkSemaphoreGroup XGLVkSemaphoreGroup;
typedef struct XGLVkFenceGroup XGLVkFenceGroup;
typedef struct XGLVkDeviceBufferGroup XGLVkDeviceBufferGroup;

typedef struct XGLVkLayer {
  VkLayerProperties properties;
  VkExtensionProperties *extensions;
  uint32_t     extensionCount;
} XGLVkLayer;

typedef struct XGLSoftware {
  const char *name;
  uint32_t version;
} XGLSoftware;

typedef struct XGLVkBufferInfo {
  VkBufferCreateFlags    flags;
  VkDeviceSize           size;
  VkBufferUsageFlags     usage;
  VkSharingMode          sharingMode;
  uint32_t               memoryOffset;
  uint32_t               queueFamilyIndexCount;
  const uint32_t*        pQueueFamilyIndices;
} XGLVkBufferInfo;

typedef struct XGLVkRecordInfo {
  VkClearValue clearValue;
  uint32_t     imageIndex;
  uint32_t     waitSemCount;
  uint32_t     signalSemCount;
  uint32_t     swapchainCount;
  VkFence      waitFence;
  VkSemaphore *presentSemaphores;
  VkSemaphore *submitSemaphores;
  VkPipelineStageFlags *waitStageFlags;
} XGLVkRecordInfo;

typedef struct XGLVkBufferCopyInfo {
  uint32_t  count;
  bool      sameDst;
  VkBuffer *dstBuffers;
  const void **datas;
  const uint32_t *sizes;
  uint32_t *dstOffsets;
} XGLVkBufferCopyInfo;

void XGLVkLayer_release(XGLVkLayer *layer, const Allocator *allocator);

extern const uint32_t REQUIRED_DEVICE_EXTENSION_NAME_COUNT;
extern const uint32_t REQUIRED_DEVICE_LAYER_NAME_COUNT;
extern const char* REQUIRED_DEVICE_EXTENSION_NAMES[];
extern const char* REQUIRED_DEVICE_LAYER_NAMES[];


#endif //VULKAN_DEMO_VULKAN_INSTANCE_H
