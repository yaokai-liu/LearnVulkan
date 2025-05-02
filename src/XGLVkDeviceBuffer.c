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
 * Filename: XGLVkDeviceBuffer.c
 * Creator: Yaokai Liu
 * Create Date: 2025-05-02
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkDeviceBuffer.h"
#include "XGLVkPhysicalDevice.h"
#include "XGLVkDevice.h"
#include "runtime-msg.h"

XGLVkDeviceBufferGroup *
XGLVkDeviceBufferGroup_new(const XGLVkDevice *device, const uint32_t bufferCount, const XGLVkBufferInfo *bufferInfos,
                           const VkMemoryPropertyFlags memoryProperty, const Allocator *allocator) {

  XGLVkDeviceBufferGroup *group = allocator->calloc(1, sizeof(XGLVkDeviceBufferGroup));
  group->allocator = allocator;
  group->device = device;
  group->buffers = allocator->calloc(bufferCount, sizeof(VkBuffer));
  for (group->count = 0; group->count < bufferCount; group->count ++) {
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = bufferInfos[group->count].flags,
        .size = bufferInfos[group->count].size,
        .usage = bufferInfos[group->count].usage,
        .sharingMode = bufferInfos[group->count].sharingMode,
        .queueFamilyIndexCount = bufferInfos[group->count].queueFamilyIndexCount,
        .pQueueFamilyIndices = bufferInfos[group->count].pQueueFamilyIndices,
    };
    VkResult result = vkCreateBuffer(device->handle, &bufferCreateInfo,
                            nullptr, &group->buffers[group->count]);
    if (result !=VK_SUCCESS) {
      rt_message("Failed to create buffer");
      XGLVkDeviceBufferGroup_destroy(group);
      return nullptr;
    }
  }
  uint32_t memorySize = 0, memoryTypeBits = 0;
  for (uint32_t i = 0; i < group->count; i ++){
    VkMemoryRequirements memoryRequirements = { };
    vkGetBufferMemoryRequirements(device->handle, group->buffers[i], &memoryRequirements);
    group->property = memoryProperty;
    memoryTypeBits |= memoryRequirements.memoryTypeBits;
    memorySize += memoryRequirements.size;
  }
  uint32_t memoryTypeIndex = XGLVkPhysicalDevice_findMemType(device->physical, memoryProperty, memoryTypeBits);
  VkMemoryAllocateInfo memoryAllocateInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = memorySize,
      .memoryTypeIndex = memoryTypeIndex,
  };
  VkResult result = vkAllocateMemory(device->handle, &memoryAllocateInfo, nullptr, &group->memory);
  if (result != VK_SUCCESS) {
    rt_message("Failed to allocate memory for vertex buffer on device '%s'", device->physical->properties.deviceName);
    XGLVkDeviceBufferGroup_destroy(group);
    return nullptr;
  }
  for (uint32_t i = 0; i < group->count; i ++) {
    vkBindBufferMemory(device->handle, group->buffers[i], group->memory, bufferInfos[i].memoryOffset);
  }
  return group;
}

void XGLVkDeviceBufferGroup_destroy(XGLVkDeviceBufferGroup *group) {
  if (group->buffers) {
    for (uint32_t i = 0; i < group->count; i++) {
      vkDestroyBuffer(group->device->handle, group->buffers[i], nullptr);
    }
    group->allocator->free(group->buffers);
  }
  if (group->memory) { vkFreeMemory(group->device->handle, group->memory, nullptr); }
  group->allocator->free(group);
}

void XGLVkDeviceMemory_copyData(XGLVkDeviceBufferGroup *group, const uint32_t offset, const uint32_t size, const void *data) {
  void *mappingAddr = nullptr;
  vkMapMemory(group->device->handle, group->memory, offset, size, 0, &mappingAddr);
  group->allocator->memcpy(mappingAddr, data, size);
  if ((group->property & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
    VkMappedMemoryRange memoryRange = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = nullptr, .memory = group->memory,
        .offset = offset, .size = size,
    };
    vkFlushMappedMemoryRanges( group->device->handle, 1, &memoryRange);
  }
  vkUnmapMemory(group->device->handle, group->memory);
}
