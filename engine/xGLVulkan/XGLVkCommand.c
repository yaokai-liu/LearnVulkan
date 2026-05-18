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
 * Filename: XGLVkCommand.c
 * Creator: Yaokai Liu
 * Create Date: 2025-05-01
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkCommand.h"
#include "XGLVkSwapchain.h"
#include "XGLVkPipeline.h"
#include "XGLVkSurface.h"
#include "XGLVkDevice.h"
#include "runtime-msg.h"
#include "XGLVkRenderPass.h"

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
  commandPool->buffers = Array_new(sizeof(VkCommandBuffer), -1, allocator);
  return commandPool;
}

VkCommandBuffer *XGLVkCommandPool_newCommand(const XGLVkCommandPool *pool, const uint32_t count) {
  VkCommandBufferAllocateInfo bufferInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = pool->handle,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = count,
  };
  uint32_t bufferCount = Array_length(pool->buffers);
  Array_resize(pool->buffers, bufferCount + count, nullptr);
  VkCommandBuffer *addendBuffers = Array_real_addr(pool->buffers, bufferCount);
  VkResult result = vkAllocateCommandBuffers(pool->device->handle, &bufferInfo, addendBuffers);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create command buffers");
    return nullptr;
  }
  return addendBuffers;
}

VkCommandBuffer XGLVkCommandPool_getCommand(XGLVkCommandPool *pool, uint32_t index) {
  return *(VkCommandBuffer *) Array_real_addr(pool->buffers, index);
}

void XGLVkCommandPool_destroy(XGLVkCommandPool *pool) {
  if (pool->buffers) {
    releasePrimeArray(pool->buffers);
  }
  if (pool->handle != VK_NULL_HANDLE) {
    vkDestroyCommandPool(pool->device->handle, pool->handle, nullptr);
  }
  pool->allocator->free(pool);
}

VkResult
XGLVkCommand_startRecord(VkCommandBuffer command, const XGLVkRenderPass *renderPass, const XGLVkSwapchain *swapchain,
                         const XGLVkPipeline *pipeline, const XGLVkRecordInfo *recordInfo) {
  const XGLVkSurface *surface = renderPass->surface;
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
      .renderPass = renderPass->handle,
      .framebuffer = swapchain->framebuffers[recordInfo->imageIndex],
      .renderArea = {.offset = {0, 0}, .extent = surface->extent},
      .clearValueCount = 1,
      .pClearValues = &recordInfo->clearValue,
  };
  vkCmdBeginRenderPass(command, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
  vkCmdSetViewport(command, 0, 1, &surface->viewport);
  vkCmdSetScissor(command, 0, 1, &surface->scissor);
  return result;
}

VkResult XGLVkCommand_endRecord(VkCommandBuffer command) {
  vkCmdEndRenderPass(command);
  VkResult result = vkEndCommandBuffer(command);
  return result;
}

void XGLVkCommand_adjustDevice(VkCommandBuffer command, const XGLVkDevice *device) {
  vkCmdSetViewport(command, 0, 1, &device->surface->viewport);
}