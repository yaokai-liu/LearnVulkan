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
 * Filename: XGLVkPipeline.h
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#ifndef XGL_VK_PIPELINE_H
#define XGL_VK_PIPELINE_H

#include "XGLVulkan.h"

typedef struct XGLVkShader {
  uint32_t size;
  VkShaderStageFlagBits stage;
  const char *entryPoint;
  const uint32_t *code;
} XGLVkShaderInfo;

typedef struct XGLVkPipelineInfo {
  uint32_t shaderCount;
  uint32_t vertAttrCount;
  uint32_t vertBindCount;
  const Allocator *allocator;
  const XGLVkDevice *device;
  VkShaderModule *shaderModules;
  VkPipelineShaderStageCreateInfo *shaderStages;
  VkVertexInputAttributeDescription *vertAttributes;
  VkVertexInputBindingDescription *vertBindings;
  Array *descriptorSetLayouts; // Array<VkDescriptorSetLayout>
} XGLVkPipelineInfo;

typedef struct XGLVkPipeline {
  VkPipeline handle;
  const Allocator *allocator;
  const XGLVkDevice *device;
  const XGLVkRenderPass *renderPass;
  VkPipelineLayout layout;
} XGLVkPipeline;

VkResult composeShaderModules(XGLVkPipelineInfo *info,
                              const XGLVkShaderInfo *shaders, uint32_t shaderCount);
VkResult composeVertexInputs(XGLVkPipelineInfo *info,
                             uint32_t bindingCount, const VkVertexInputBindingDescription *bindings,
                             uint32_t attributeCount, const VkVertexInputAttributeDescription *attributes);
VkResult composeSetLayouts(XGLVkPipelineInfo *info, uint32_t bindingCount, VkDescriptorSetLayoutBinding *bindings);
void XGLVkPipelineInfo_release(XGLVkPipelineInfo *info);

XGLVkPipeline *
XGLVkPipeline_new(const XGLVkDevice *device, const XGLVkRenderPass *renderPass, const XGLVkPipelineInfo *info, const Allocator *allocator);

void XGLVkPipeline_destroy(XGLVkPipeline *pipeline);
#endif //XGL_VK_PIPELINE_H
