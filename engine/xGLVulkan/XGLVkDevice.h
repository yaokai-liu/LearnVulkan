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
  VkDevice            handle;
  const Allocator *   allocator;
  const XGLVkSurface *surface;
  const XGLVkPhyDevice *
                      physical;
  XGLVkCommandPool   *cmdPool;
  XGLVkDescriptorPool*descPool;
  Array*              textures; // Array<VkImageView>
  Array*              samplers; // Array<VkSampler>
  VkBuffer            ubo;
  VkDeviceMemory      uboMem;
  VkCommandBuffer     transCmd;
  Array *             queues; // Array<XGLVkQueue>
} XGLVkDevice;

XGLVkDevice *XGLVkDevice_new(const XGLVkPhyDevice *physicalDevice, const XGLVkSurface *surface, const Allocator *allocator);
void XGLVkDevice_destroy(XGLVkDevice *device);

VkResult XGLVkDevice_render(XGLVkDevice *device, VkCommandBuffer command, VkSwapchainKHR *swapchains,
                            const XGLVkRecordInfo *renderInfo);
XGLVkQueue *XGLVkDevice_getQueue(const XGLVkDevice *device, uint32_t index);

#endif //XGL_VK_DEVICE_H
