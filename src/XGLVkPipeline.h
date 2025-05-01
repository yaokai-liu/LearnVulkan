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
 * Filename: XGLVkPipeline.h
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#ifndef VULKAN_DEMO_XGL_VK_PIPELINE_H
#define VULKAN_DEMO_XGL_VK_PIPELINE_H

#include "XGLVulkan.h"

typedef struct XGLVkShader {
  uint32_t size;
  VkShaderStageFlagBits stage;
  const char *entryPoint;
  const uint32_t *code;
} XGLVkShader;

typedef struct XGLVkShaderCreatePack {
  uint32_t count;
  const Allocator *allocator;
  const XGLVkDevice *device;
  VkShaderModule *modules;
  VkPipelineShaderStageCreateInfo *infos;
} XGLVkShaderCreatePack;

typedef struct XGLVkPipeline {
  VkPipeline handle;
  const Allocator *allocator;
  const XGLVkDevice *device;
  VkPipelineLayout layout;
  VkRenderPass renderPass;
} XGLVkPipeline;

XGLVkShaderCreatePack *composeShaderModules(XGLVkDevice *device, XGLVkShader *shaders, uint32_t shaderCount, const Allocator *allocator);
void XGLVkShaderCreatePack_destroy(XGLVkShaderCreatePack *pack);

XGLVkPipeline *
XGLVkPipeline_new(XGLVkDevice *device, XGLVkShaderCreatePack *infoPack, const XGLVkSurface *surface, const Allocator *allocator);

void XGLVkPipeline_destroy(XGLVkPipeline *pipeline);
#endif //VULKAN_DEMO_XGL_VK_PIPELINE_H
