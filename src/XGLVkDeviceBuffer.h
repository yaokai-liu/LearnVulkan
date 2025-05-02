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
 * Filename: XGLVkDeviceBuffer.h
 * Creator: Yaokai Liu
 * Create Date: 2025-05-02
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#ifndef VULKAN_DEMO_XGL_VK_BUFFER_H
#define VULKAN_DEMO_XGL_VK_BUFFER_H

#include "XGLVulkan.h"

typedef struct XGLVkDeviceBufferGroup {
  VkDeviceMemory memory;
  VkMemoryPropertyFlags property;
  const Allocator *allocator;
  const XGLVkDevice *device;
  uint32_t  count;
  VkBuffer *buffers;
} XGLVkDeviceBufferGroup;

XGLVkDeviceBufferGroup *
XGLVkDeviceBufferGroup_new(const XGLVkDevice *device, uint32_t bufferCount, const XGLVkBufferInfo *bufferInfos,
                           VkMemoryPropertyFlags memoryProperty, const Allocator *allocator);
void XGLVkDeviceBufferGroup_destroy(XGLVkDeviceBufferGroup *group);

void XGLVkDeviceMemory_copyData(XGLVkDeviceBufferGroup *group, const uint32_t offset, const uint32_t size, const void *data);

#endif //VULKAN_DEMO_XGL_VK_BUFFER_H
