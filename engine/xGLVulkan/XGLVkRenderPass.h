/* License
 *
 * xGL - A Graphic Library implemented in C with Vulkan
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
 * Filename: XGLVkRenderPass.h
 * Creator: Yaokai Liu
 * Create Date: 2025-11-04
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#ifndef XGL_VK_RENDER_PASS_H
#define XGL_VK_RENDER_PASS_H
#include "XGLVulkan.h"

typedef struct XGLVkRenderPass {
  VkRenderPass handle;
  const Allocator *allocator;
  const XGLVkDevice *device;
  const XGLVkSurface *surface;
} XGLVkRenderPass;
XGLVkRenderPass *XGLVkRenderPass_new(const XGLVkDevice *device, const XGLVkSurface *surface, const Allocator *allocator);
void XGLVkRenderPass_destroy(XGLVkRenderPass *pass);
#endif //XGL_VK_RENDER_PASS_H