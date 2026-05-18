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
 * Filename: XGLVkSurface.h
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#ifndef XGL_VK_SURFACE_H
#define XGL_VK_SURFACE_H

#include "XGLVulkan.h"

typedef struct XGLVkSurface {
  VkSurfaceKHR handle;
  VkViewport viewport;
  VkRect2D scissor;
  VkExtent2D extent;
  VkSurfaceFormatKHR format;
  XGLWMWindow *window;
  const Allocator *allocator;
  const XGLVkInstance *instance;
  const XGLVkPhyDevice *device;
  VkSurfaceTransformFlagsKHR transform;
  VkPresentModeKHR presentMode;
  uint32_t swapImageCount;
} XGLVkSurface;

XGLVkSurface *XGLVkSurface_new(const XGLVkPhyDevice *device, XGLWMWindow *window, const XGLVkInstance *instance,
                               const Allocator *allocator);
void XGLVkSurface_update(XGLVkSurface *surface);
void XGLVkSurface_destroy(XGLVkSurface *surface);

#endif //XGL_VK_SURFACE_H
