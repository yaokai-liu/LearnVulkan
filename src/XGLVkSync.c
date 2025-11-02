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
 * Filename: XGLVkSync.c
 * Creator: Yaokai Liu
 * Create Date: 2025-05-01
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkSync.h"
#include "XGLVkDevice.h"
#include "runtime-msg.h"


XGLVkSemaphoreGroup *XGLVkSemaphoreGroup_new(const XGLVkDevice *device, const VkSemaphoreCreateFlags *flags,
                                             uint32_t count, const Allocator *allocator) {
  VkSemaphoreCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr, .flags = 0,
  };
  XGLVkSemaphoreGroup *group = allocator->calloc(1, sizeof(XGLVkSemaphoreGroup));
  group->allocator = allocator;
  group->device = device;
  group->count = count;

  group->semaphores = allocator->calloc(count, sizeof(XGLVkSemaphoreGroup));
  for (uint32_t i = 0; i < count; i ++) {
    if (flags) { info.flags = flags[i]; }
    VkResult result = vkCreateSemaphore(device->handle, &info, nullptr, &group->semaphores[i]);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create semaphore group");
      XGLVkSemaphoreGroup_destroy(group);
      return nullptr;
    }
  }
  return group;
}

void XGLVkSemaphoreGroup_destroy(XGLVkSemaphoreGroup *group) {
  if (group->semaphores) {
    for (uint32_t i = 0; i < group->count; i++) {
      if (group->semaphores[i] != VK_NULL_HANDLE) {
        vkDestroySemaphore(group->device->handle, group->semaphores[i], nullptr);
      }
    }
    group->allocator->free(group->semaphores);
  }
  group->allocator->free(group);
}

XGLVkFenceGroup *XGLVkFenceGroup_new(const XGLVkDevice *device, const VkFenceCreateFlags *flags,
                                     uint32_t count, const Allocator *allocator) {
  VkFenceCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr, .flags = 0,
  };
  XGLVkFenceGroup *group = allocator->calloc(1, sizeof(XGLVkFenceGroup));
  group->allocator = allocator;
  group->device = device;
  group->count = count;

  group->fences = allocator->calloc(count, sizeof(XGLVkFenceGroup));
  for (uint32_t i = 0; i < count; i ++) {
    if (flags) { info.flags = flags[i]; }
    VkResult result = vkCreateFence(device->handle, &info, nullptr, &group->fences[i]);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create fence group");
      XGLVkFenceGroup_destroy(group);
      return nullptr;
    }
  }
  return group;
}

void XGLVkFenceGroup_destroy(XGLVkFenceGroup *group) {
  if (group->fences) {
    for (uint32_t i = 0; i < group->count; i++) {
      if (group->fences[i] != VK_NULL_HANDLE) {
        vkDestroyFence(group->device->handle, group->fences[i], nullptr);
      }
    }
    group->allocator->free(group->fences);
  }
  group->allocator->free(group);
}