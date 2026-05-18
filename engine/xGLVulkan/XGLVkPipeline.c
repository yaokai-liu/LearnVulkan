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
 * Filename: XGLVkPipeline.c
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkPipeline.h"
#include "XGLVkSurface.h"
#include "XGLVkDevice.h"
#include "runtime-msg.h"
#include "utils.h"
#include "XGLVkRenderPass.h"

XGLVkPipeline * XGLVkPipeline_new(const XGLVkDevice *device, const XGLVkRenderPass *renderPass,
                                  const XGLVkPipelineInfo *info, const Allocator *allocator) {
  if (!info->shaderCount) { return nullptr; }
  VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .vertexBindingDescriptionCount = info->vertBindCount,
      .pVertexBindingDescriptions = info->vertBindings,
      .vertexAttributeDescriptionCount = info->vertAttrCount,
      .pVertexAttributeDescriptions = info->vertAttributes,
  };
  VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };
  VkDynamicState dynamicStates[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
  };
  uint32_t dynamicStateCount = lenof(dynamicStates);
  VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .dynamicStateCount = dynamicStateCount,
      .pDynamicStates = dynamicStates,
  };
  VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .viewportCount = 1,
      .pViewports = nullptr,
      .scissorCount = 1,
      .pScissors = nullptr,
  };
  VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_FRONT_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .depthBiasConstantFactor = 0,
      .depthBiasClamp = 0,
      .depthBiasSlopeFactor = 0,
      .lineWidth = 1.0f,
  };
  VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 1.0f,
      .pSampleMask = nullptr,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable = VK_FALSE,
  };
  VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo[[maybe_unused]] = {};
  VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {
      .blendEnable = VK_FALSE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };
  VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      #if defined(XGL_SURFACE_WIN32)
      .logicOpEnable = VK_TRUE,
      .logicOp = VK_LOGIC_OP_COPY,
      #elif defined(XGL_SURFACE_WAYLAND)
      .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_CLEAR,
      #endif
      .attachmentCount = 1,
      .pAttachments = &pipelineColorBlendAttachmentState,
      .blendConstants = {1.0f, 1.0f, 1.0f, 1.0f},
  };
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .setLayoutCount = info->descriptorSetLayouts ? Array_length(info->descriptorSetLayouts) : 0,
      .pSetLayouts = info->descriptorSetLayouts ? Array_first_real(info->descriptorSetLayouts) : nullptr,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr,
  };
  VkPipelineLayout layout = VK_NULL_HANDLE;
  VkResult result = vkCreatePipelineLayout(device->handle, &pipelineLayoutCreateInfo, nullptr, &layout);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create pipeline layout");
    return nullptr;
  }
  VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stageCount = info->shaderCount,
      .pStages = info->shaderStages,
      .pVertexInputState = &pipelineVertexInputStateCreateInfo,
      .pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo,
      .pTessellationState = nullptr,
      .pViewportState = &pipelineViewportStateCreateInfo,
      .pRasterizationState = &pipelineRasterizationStateCreateInfo,
      .pMultisampleState = &pipelineMultisampleStateCreateInfo,
      .pDepthStencilState = nullptr,
      .pColorBlendState = &pipelineColorBlendStateCreateInfo,
      .pDynamicState = &pipelineDynamicStateCreateInfo,
      .layout = layout,
      .renderPass = renderPass->handle,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
  };
  VkPipeline handle = VK_NULL_HANDLE;
  result = vkCreateGraphicsPipelines(device->handle, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &handle);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create graphics pipelines");
    return nullptr;
  }
  XGLVkPipeline *pipeline = allocator->calloc(1, sizeof(XGLVkPipeline));
  pipeline->allocator = allocator;
  pipeline->renderPass = renderPass;
  pipeline->device = device;
  pipeline->handle = handle;
  pipeline->layout = layout;
  return pipeline;
}

VkResult
composeShaderModules(XGLVkPipelineInfo *info, const XGLVkShaderInfo *shaders, uint32_t shaderCount) {
  if (info->device == VK_NULL_HANDLE || !info->allocator) { return ~VK_SUCCESS; }
  info->shaderModules = info->allocator->calloc(shaderCount, sizeof(VkShaderModule));
  info->shaderStages = info->allocator->calloc(shaderCount, sizeof(VkPipelineShaderStageCreateInfo));
  VkShaderModuleCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .codeSize = 0,
      .pCode = nullptr,
  };
  VkResult result = 0;
  for (uint32_t i = 0; i < shaderCount; i++) {
    createInfo.codeSize = shaders[i].size;
    createInfo.pCode = shaders[i].code;
    result = vkCreateShaderModule(info->device->handle, &createInfo,
                                  nullptr, &info->shaderModules[i]);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create shader module");
      info->shaderCount = i;
      return result;
    }
    info->shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info->shaderStages[i].pNext = nullptr;
    info->shaderStages[i].flags = 0;
    info->shaderStages[i].stage = shaders[i].stage;
    info->shaderStages[i].module = info->shaderModules[i];
    info->shaderStages[i].pName = shaders[i].entryPoint;
    info->shaderStages[i].pSpecializationInfo = nullptr;
  }
  info->shaderCount = shaderCount;
  return result;
}

VkResult composeVertexInputs(XGLVkPipelineInfo *info,
         uint32_t bindingCount, const VkVertexInputBindingDescription *bindings,
         uint32_t attributeCount, const VkVertexInputAttributeDescription *attributes) {
  info->vertBindCount = bindingCount;
  info->vertAttrCount = attributeCount;
  info->vertBindings = info->allocator->calloc(bindingCount, sizeof(VkVertexInputBindingDescription));
  info->vertAttributes = info->allocator->calloc(attributeCount, sizeof(VkVertexInputAttributeDescription));
  info->allocator->memcpy(info->vertBindings, bindings, bindingCount * sizeof(VkVertexInputBindingDescription));
  info->allocator->memcpy(info->vertAttributes, attributes, attributeCount * sizeof(VkVertexInputAttributeDescription));
  return VK_SUCCESS;
}
VkResult composeSetLayouts(XGLVkPipelineInfo *info, uint32_t bindingCount, VkDescriptorSetLayoutBinding *bindings) {
  VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .pNext = nullptr,
      .flags = 0, .bindingCount = bindingCount, .pBindings = bindings,
  };
  VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
  VkResult result = vkCreateDescriptorSetLayout(info->device->handle, &setLayoutCreateInfo, nullptr, &setLayout);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create set layout");
    return result;
  }
  if (!info->descriptorSetLayouts) { info->descriptorSetLayouts = Array_new(sizeof(VkDescriptorSetLayout), -1, info->allocator); }
  Array_append(info->descriptorSetLayouts, &setLayout, 1);
  return result;
}

void XGLVkPipelineInfo_release(XGLVkPipelineInfo *info) {
  if (info->shaderModules) {
    for (uint32_t i = 0; i < info->shaderCount; i ++) {
      if (info->shaderModules[i] != VK_NULL_HANDLE) {
        vkDestroyShaderModule(info->device->handle, info->shaderModules[i], nullptr);
      }
    }
    info->allocator->free(info->shaderModules);
    info->shaderModules = nullptr;
  }
  info->shaderCount = 0;
  if (info->vertBindings) {
    info->allocator->free(info->vertBindings);
    info->vertBindings = nullptr;
  }
  info->vertBindCount = 0;
  if (info->vertAttributes) {
    info->allocator->free(info->vertAttributes);
    info->vertAttributes= nullptr;
  }
  info->vertAttrCount = 0;
  if (info->shaderStages) {
    info->allocator->free(info->shaderStages);
    info->shaderStages = nullptr;
  }
  if (info->descriptorSetLayouts) {
    uint32_t count = Array_length(info->descriptorSetLayouts);
    VkDescriptorSetLayout *layouts = Array_first_real(info->descriptorSetLayouts);
    for (uint32_t i = 0; i < count; i ++) {
      if (layouts[i] != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(info->device->handle, layouts[i], nullptr);
      }
    }
    releasePrimeArray(info->descriptorSetLayouts);
    info->descriptorSetLayouts = nullptr;
  }
}

void XGLVkPipeline_destroy(XGLVkPipeline *pipeline) {
  if (pipeline->handle != VK_NULL_HANDLE) {
    vkDestroyPipeline(pipeline->device->handle, pipeline->handle, nullptr);
  }
  if (pipeline->layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(pipeline->device->handle, pipeline->layout, nullptr);
  }
  pipeline->allocator->free(pipeline);
}
