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
 * Module Name: src
 * Filename: XGLVkDevice.h
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#ifndef XGL_VK_DEVICE_H
#define XGL_VK_DEVICE_H

#include "XGLVulkan.h"

typedef struct XGLVkQueue {
  VkQueue  queue;
  uint32_t index;
} XGLVkQueue;

typedef struct XGLVkDevice {
  VkDevice handle;
  const Allocator *allocator;
  const XGLVkSurface *surface;
  const XGLVkPhysicalDevice *physical;
  Array *queues; // Array<XGLVkQueue>
  Array *descriptorPools; // Array<VkDescriptorPool>
} XGLVkDevice;

XGLVkDevice *XGLVkDevice_new(const XGLVkPhysicalDevice *physicalDevice, const XGLVkSurface *surface, const Allocator *allocator);
void XGLVkDevice_destroy(XGLVkDevice *device);

VkResult XGLVkDevice_render(XGLVkDevice *device, VkCommandBuffer command, VkSwapchainKHR *swapchains,
                            const XGLVkRecordInfo *renderInfo);
XGLVkQueue *XGLVkDevice_getQueue(XGLVkDevice *device, uint32_t index);
VkResult XGLVkDevice_cmdCopyBufferData(XGLVkDevice *device, XGLVkCommandPool *commandPool, const XGLVkBufferCopyInfo *bufferCopyInfo);

const VkDescriptorPool *XGLVkDevice_allocDescriptorPool(XGLVkDevice *device, uint32_t maxSetCount,
                                                        uint32_t poolSizeCount, VkDescriptorPoolSize *poolSizes);
#endif //XGL_VK_DEVICE_H
