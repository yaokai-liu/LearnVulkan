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
 * Filename: XGLVkCommandPool.h
 * Creator: Yaokai Liu
 * Create Date: 2025-05-01
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#ifndef VULKAN_DEMO_XGL_VK_COMMAND_H
#define VULKAN_DEMO_XGL_VK_COMMAND_H

#include "XGLVulkan.h"

typedef struct XGLVkCommandPool {
  VkCommandPool handle;
  const XGLVkDevice *device;
  const Allocator *allocator;
  uint32_t bufferCount;
  VkCommandBuffer *buffers;
} XGLVkCommandPool;

XGLVkCommandPool *XGLVkCommandPool_new(XGLVkDevice *device, const Allocator *allocator);
void XGLVkCommandPool_destroy(XGLVkCommandPool *pool);
VkResult XGLVkCommand_record(VkCommandBuffer command, const XGLVkSurface *surface, const XGLVkSwapchain *swapchain,
                    const XGLVkPipeline *pipeline, const XGLVkRenderInfo *recordInfo);
#endif //VULKAN_DEMO_XGL_VK_COMMAND_H
