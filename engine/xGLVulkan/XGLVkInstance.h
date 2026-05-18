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
 * Filename: XGLVkInstance.h
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#ifndef XGL_VK_INSTANCE_H
#define XGL_VK_INSTANCE_H

#include "XGLVulkan.h"

typedef struct XGLVkInstance {
  XGLSoftwareInfo app, engine;
  uint32_t  apiVersion;
  VkInstance handle;
  bool       enableValidationLayers;
  Array *layers; // Array<XGLVkLayer>
  Array *devices; // Array<XGLVkPhyDevice>
  Array *extensions; // Array<VkExtensionProperties>
  const Allocator *allocator;
} XGLVkInstance;

XGLVkInstance *XGLVkInstance_new(const XGLSoftwareInfo *app, const XGLSoftwareInfo *engine, const Allocator *allocator);
void XGLVkInstance_destroy(XGLVkInstance *instance);
const XGLVkPhyDevice *XGLVkInstance_pickPhysicalDevice(XGLWMWindow *window, const XGLVkInstance *instance);

VkResult XGLVkInstance_verifyLayers(XGLVkInstance *instance, uint32_t layerCount, const char *layerNames[]);
VkResult XGLVkInstance_verifyExtensions(XGLVkInstance *instance, uint32_t extensionCount, const char *extensionNames[]);

#endif //XGL_VK_INSTANCE_H
