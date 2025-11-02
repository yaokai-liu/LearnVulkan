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
 * Filename: XGLVkSync.h
 * Creator: Yaokai Liu
 * Create Date: 2025-05-01
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#ifndef XGL_VK_SYNC_H
#define XGL_VK_SYNC_H
#include "XGLVulkan.h"

typedef struct XGLVkSemaphoreGroup {
  const XGLVkDevice *device;
  const Allocator *allocator;
  VkSemaphore *semaphores;
  uint32_t count;
} XGLVkSemaphoreGroup;
typedef struct XGLVkFenceGroup {
  const XGLVkDevice *device;
  const Allocator *allocator;
  VkFence *fences;
  uint32_t count;
} XGLVkFenceGroup;

XGLVkSemaphoreGroup *XGLVkSemaphoreGroup_new(const XGLVkDevice *device, const VkSemaphoreCreateFlags *flags, uint32_t count, const Allocator *allocator);
void XGLVkSemaphoreGroup_destroy(XGLVkSemaphoreGroup *group);
XGLVkFenceGroup *XGLVkFenceGroup_new(const XGLVkDevice *device, const VkFenceCreateFlags *flags, uint32_t count, const Allocator *allocator);
void XGLVkFenceGroup_destroy(XGLVkFenceGroup *group);

#endif //XGL_VK_SYNC_H
