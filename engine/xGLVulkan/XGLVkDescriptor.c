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
 * Filename: XGLVkDescriptor.c
 * Creator: Yaokai Liu
 * Create Date: 2025-05-08
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkDescriptor.h"
#include "runtime-msg.h"
#include "XGLVkDevice.h"

XGLVkDescriptorPool *XGLVkDescriptorPool_new(const XGLVkDevice *device, const Allocator *allocator) {
  XGLVkDescriptorPool *pool = allocator->calloc(1, sizeof(XGLVkDescriptorPool));
  VkDescriptorPoolSize poolSize = {
    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = MAX_FRAME_ON_DRAW
  };
  VkDescriptorPoolCreateInfo poolCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .pNext = nullptr, .flags = 0,
    .maxSets = MAX_FRAME_ON_DRAW, .poolSizeCount = 1, .pPoolSizes = &poolSize
  };
  VkDescriptorPool handle = VK_NULL_HANDLE;
  VkResult result = vkCreateDescriptorPool(device->handle, &poolCreateInfo, nullptr, &handle);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create descriptor pool");
    return nullptr;
  }

  pool->handle = handle;
  pool->allocator = allocator;
  pool->device = device;
  pool->sets = Array_new(sizeof(VkDescriptorSet), -1, allocator);
  return pool;
}
void XGLVkDescriptorPool_destroy(XGLVkDescriptorPool *pool) {
  if (pool->sets) {
    releasePrimeArray(pool->sets);
  }
  if (pool->handle != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(pool->device->handle, pool->handle, nullptr);
  }
  pool->allocator->free(pool);
}