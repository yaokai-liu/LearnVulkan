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
 * Filename: XGLVkCommandPool.c
 * Creator: Yaokai Liu
 * Create Date: 2025-05-01
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkCommandPool.h"
#include "XGLVkSwapchain.h"
#include "XGLVkPipeline.h"
#include "XGLVkSurface.h"
#include "XGLVkDevice.h"
#include "runtime-msg.h"

XGLVkCommandPool *XGLVkCommandPool_new(XGLVkDevice *device, const Allocator *allocator) {
  XGLVkCommandPool *commandPool = allocator->calloc(1, sizeof(XGLVkCommandPool));
  commandPool->allocator = allocator;
  commandPool->device = device;
  XGLVkQueue *graphicsQueue = Array_real_addr(device->queues, GRAPHICS_QUEUE_INDEX);
  VkCommandPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = graphicsQueue->index,
  };
  VkResult result = vkCreateCommandPool(device->handle, &poolInfo, nullptr, &commandPool->handle);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create command handle");
    XGLVkCommandPool_destroy(commandPool);
    return nullptr;
  }
  commandPool->bufferCount = MAX_FRAME_ON_DRAW;
  VkCommandBufferAllocateInfo bufferInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = commandPool->handle,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = commandPool->bufferCount,
  };
  commandPool->buffers = allocator->calloc(commandPool->bufferCount, sizeof(VkCommandBuffer));
  result = vkAllocateCommandBuffers(device->handle, &bufferInfo, commandPool->buffers);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create command buffer");
    XGLVkCommandPool_destroy(commandPool);
    return nullptr;
  }
  return commandPool;
}

void XGLVkCommandPool_destroy(XGLVkCommandPool *commandPool) {
  if (commandPool->buffers) {
    commandPool->allocator->free(commandPool->buffers);
  }
  if (commandPool->handle != VK_NULL_HANDLE) {
    vkDestroyCommandPool(commandPool->device->handle, commandPool->handle, nullptr);
  }
  commandPool->allocator->free(commandPool);
}

VkResult
XGLVkCommand_record(VkCommandBuffer command, const XGLVkSurface *surface, const XGLVkSwapchain *swapchain,
                    const XGLVkPipeline *pipeline, const XGLVkRenderInfo *recordInfo) {
  vkResetCommandBuffer(command, 0);
  VkCommandBufferBeginInfo commandBufferBeginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr, .flags = 0,
      .pInheritanceInfo = nullptr,
  };
  VkResult result = vkBeginCommandBuffer(command, &commandBufferBeginInfo);
  if (result != VK_SUCCESS) { return result; }
  VkRenderPassBeginInfo renderPassBeginInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext = nullptr,
      .renderPass = pipeline->renderPass,
      .framebuffer = swapchain->framebuffers[recordInfo->imageIndex],
      .renderArea = {.offset = {0, 0}, .extent = surface->extent},
      .clearValueCount = 1,
      .pClearValues = &recordInfo->clearValue,
  };
  vkCmdBeginRenderPass(command, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
  vkCmdSetViewport(command, 0, 1, &surface->viewport);
  vkCmdSetScissor(command, 0, 1, &surface->scissor);
  vkCmdDraw(command, 3, 1, 0, 0);
  vkCmdEndRenderPass(command);
  result = vkEndCommandBuffer(command);
  return result;
}