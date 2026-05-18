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
 * Filename: XGLVkDeviceBuffer.c
 * Creator: Yaokai Liu
 * Create Date: 2025-05-02
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkDeviceBuffer.h"
#include "XGLVkPhysicalDevice.h"
#include "XGLVkDevice.h"
#include "runtime-msg.h"
#include "minmax.h"

typedef struct XGLVkDeviceMemoryMapping {
  uint32_t offset, size;
  void *   address;
} XGLVkDeviceMemoryMapping;


XGLVkDeviceBufferGroup *
XGLVkDeviceBufferGroup_new(const XGLVkDevice *device, const Allocator *allocator) {
  XGLVkDeviceBufferGroup *group = allocator->calloc(1, sizeof(XGLVkDeviceBufferGroup));
  group->allocator = allocator;
  group->mapping = nullptr;
  group->device = device;
  group->buffers = Array_new(sizeof(VkBuffer), -1, allocator);
  return group;
}

VkResult
XGLVkDeviceBufferGroup_allocBuffers(XGLVkDeviceBufferGroup *group, const XGLVkBufferInfo *bufferInfos,
                                    const uint32_t bufferInfoCount, const VkMemoryPropertyFlags memoryProperty) {
  const uint32_t count = Array_length(group->buffers);
  Array_resize(group->buffers, count + bufferInfoCount, nullptr);
  VkBuffer *buffers = Array_first_real(group->buffers);
  for (uint32_t i = 0; i < bufferInfoCount; i ++) {
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = bufferInfos[i].flags,
        .size = bufferInfos[i].size,
        .usage = bufferInfos[i].usage,
        .sharingMode = bufferInfos[i].sharingMode,
        .queueFamilyIndexCount = bufferInfos[i].queueFamilyIndexCount,
        .pQueueFamilyIndices = bufferInfos[i].pQueueFamilyIndices,
    };
    VkResult result = vkCreateBuffer(group->device->handle, &bufferCreateInfo, nullptr, &buffers[count + i]);
    if (result !=VK_SUCCESS) {
      rt_message("Failed to create buffer");
      XGLVkDeviceBufferGroup_destroy(group);
      return result;
    }
  }
  uint32_t memorySize = 0, memoryTypeBits = 0;
  for (uint32_t i = 0; i < bufferInfoCount; i ++){
    VkMemoryRequirements memoryRequirements = { };
    vkGetBufferMemoryRequirements(group->device->handle, buffers[count + i], &memoryRequirements);
    group->property = memoryProperty;
    memoryTypeBits |= memoryRequirements.memoryTypeBits;
    memorySize += memoryRequirements.size;
  }
  const uint32_t memoryTypeIndex = XGLVkPhysicalDevice_findMemType(group->device->physical, memoryProperty, memoryTypeBits);
  const VkMemoryAllocateInfo memoryAllocateInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = memorySize,
      .memoryTypeIndex = memoryTypeIndex,
  };
  VkResult result = vkAllocateMemory(group->device->handle, &memoryAllocateInfo, nullptr, &group->memory);
  if (result != VK_SUCCESS) {
    rt_message("Failed to allocate memory for vertex buffer on device '%s'", group->device->physical->properties.deviceName);
    XGLVkDeviceBufferGroup_destroy(group);
    return result;
  }
  for (uint32_t i = 0; i < bufferInfoCount; i ++) {
    vkBindBufferMemory(group->device->handle, buffers[i], group->memory, bufferInfos[i].memoryOffset);
  }
  return VK_SUCCESS;
}

void XGLVkDeviceBufferGroup_destroy(XGLVkDeviceBufferGroup *group) {
  if (group->buffers) {
    const uint32_t count = Array_length(group->buffers);
    VkBuffer *buffers = Array_first_real(group->buffers);
    for (uint32_t i = 0; i < count; i++) {
      vkDestroyBuffer(group->device->handle, buffers[i], nullptr);
    }
    group->allocator->free(group->buffers);
  }
  if (group->mapping) {
    vkUnmapMemory(group->device->handle, group->memory);
    group->allocator->free(group->mapping);
  }
  if (group->memory) { vkFreeMemory(group->device->handle, group->memory, nullptr); }
  group->allocator->free(group);
}

void *XGLVkDeviceBufferGroup_mapping(XGLVkDeviceBufferGroup *group, uint32_t offset, uint32_t size) {
  if (group->mapping) { vkUnmapMemory(group->device->handle, group->memory); }
  else { group->mapping = group->allocator->calloc(1, sizeof(XGLVkDeviceMemoryMapping)); }
  group->mapping->address = nullptr;
  group->mapping->offset = offset;
  group->mapping->size = size;
  vkMapMemory(group->device->handle, group->memory, offset, size, 0, &group->mapping->address);
  return group->mapping->address;
}

void XGLVkDeviceMemory_copyData(XGLVkDeviceBufferGroup *group, const uint32_t offset, const uint32_t size, const void *data) {
  void *mappingAddr = XGLVkDeviceBufferGroup_mapping(group, offset, size);
  group->allocator->memcpy(mappingAddr, data, size);
  if ((group->property & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
    const VkMappedMemoryRange memoryRange = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = nullptr, .memory = group->memory,
        .offset = offset, .size = size,
    };
    vkFlushMappedMemoryRanges( group->device->handle, 1, &memoryRange);
  }
}

VkResult XGLVkDeviceBuffer_cmdTransBufferData(const XGLVkDeviceBufferGroup *dstGroup,
                                              const XGLVkBufferTransInfo *transInfos, const uint32_t transInfoCount) {
  uint32_t max_size = 0;
  for (uint32_t i = 0; i < transInfoCount; i ++) { max_size = max(max_size, transInfos[i].size); }
  const XGLVkBufferInfo stagingBufferInfo = {
    .flags = 0, .size = max_size,
    .pQueueFamilyIndices = nullptr, .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE, .memoryOffset = 0, .queueFamilyIndexCount = 0,
  };
  XGLVkDeviceBufferGroup *stagingBufferGroup = XGLVkDeviceBufferGroup_new(dstGroup->device, dstGroup->allocator);
  XGLVkDeviceBufferGroup_allocBuffers(stagingBufferGroup, &stagingBufferInfo, 1,
                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

  VkCommandBuffer transferCommand = stagingBufferGroup->device->transCmd;
  const VkCommandBufferBeginInfo commandBufferBeginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr, .pInheritanceInfo = nullptr, .flags = 0,
  };
  VkResult result = VK_SUCCESS;
  const VkBuffer *srcBuffer = Array_real_addr(stagingBufferGroup->buffers, 0);
  for (uint32_t i = 0; i < transInfoCount; i++) {
    XGLVkDeviceMemory_copyData(stagingBufferGroup, 0, transInfos[i].size, transInfos[i].data);
    result = vkBeginCommandBuffer(transferCommand, &commandBufferBeginInfo);
    if (result != VK_SUCCESS) {
      rt_error("Failed to begin transfer command");
      return result;
    }
    VkBufferCopy copy = {.srcOffset = 0, .size = transInfos[i].size, .dstOffset = transInfos[i].dstOffset };
    const VkBuffer *dstBuffer = Array_real_addr(dstGroup->buffers, transInfos[i].dstBufferIndex);
    vkCmdCopyBuffer(transferCommand, *srcBuffer, *dstBuffer, 1, &copy);
    result = vkEndCommandBuffer(transferCommand);
    if (result != VK_SUCCESS) {
      rt_error("Failed to end transfer command");
      return result;
    }
    VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .commandBufferCount = 1,
      .pCommandBuffers = &transferCommand,
    };
    const XGLVkQueue *queue = XGLVkDevice_getQueue(stagingBufferGroup->device, TRANSFER_QUEUE_INDEX);
    result = vkQueueSubmit(queue->queue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
      rt_error("Failed to submit command to transfer queue");
      return result;
    }
    result = vkDeviceWaitIdle(stagingBufferGroup->device->handle);
    if (result != VK_SUCCESS) {
      rt_error("Device not idled");
      return result;
    }
  }
  XGLVkDeviceBufferGroup_destroy(stagingBufferGroup);

  return result;
}
