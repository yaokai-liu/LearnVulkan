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
 * Filename: XGLVkPipeline.c
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkPipeline.h"
#include "XGLVkSurface.h"
#include "XGLVkDevice.h"
#include "runtime-msg.h"
#include "util-macro.h"

XGLVkPipeline *
XGLVkPipeline_new(XGLVkDevice *device, XGLVkShaderCreatePack *infoPack, const XGLVkSurface *surface, const Allocator *allocator) {
  if (!infoPack->count) { return nullptr; }

  XGLVkPipeline *pipeline = allocator->calloc(1, sizeof(XGLVkPipeline));
  pipeline->allocator = allocator;
  pipeline->device = device;

  VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .vertexBindingDescriptionCount = 0,
      .pVertexBindingDescriptions = nullptr,
      .vertexAttributeDescriptionCount = 0,
      .pVertexAttributeDescriptions = nullptr,
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
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
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
  VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
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
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr,
  };
  VkResult result = vkCreatePipelineLayout(device->handle, &pipelineLayoutCreateInfo, nullptr, &pipeline->layout);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create pipeline layout");
    XGLVkPipeline_destroy(pipeline);
    return nullptr;
  }
  VkAttachmentDescription colorAttachmentDescription = {
      .flags = 0,
      .format = surface->format.format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };
  VkAttachmentReference colorAttachmentReference = {
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };

  VkSubpassDescription subpassDescription = {
      .flags = 0,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .pInputAttachments = nullptr,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentReference,
      .pResolveAttachments = nullptr,
      .pDepthStencilAttachment = nullptr,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = nullptr,
  };
  VkSubpassDependency subpassDependency = {
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = 0,
  };
  VkRenderPassCreateInfo renderPassCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .attachmentCount = 1,
      .pAttachments = &colorAttachmentDescription,
      .subpassCount = 1,
      .pSubpasses = &subpassDescription,
      .dependencyCount = 1,
      .pDependencies = &subpassDependency,
  };
  result = vkCreateRenderPass(device->handle, &renderPassCreateInfo, nullptr, &pipeline->renderPass);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create render pass");
    XGLVkPipeline_destroy(pipeline);
    return nullptr;
  }
  VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stageCount = infoPack->count,
      .pStages = infoPack->infos,
      .pVertexInputState = &pipelineVertexInputStateCreateInfo,
      .pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo,
      .pTessellationState = nullptr,
      .pViewportState = &pipelineViewportStateCreateInfo,
      .pRasterizationState = &pipelineRasterizationStateCreateInfo,
      .pMultisampleState = &pipelineMultisampleStateCreateInfo,
      .pDepthStencilState = nullptr,
      .pColorBlendState = &pipelineColorBlendStateCreateInfo,
      .pDynamicState = &pipelineDynamicStateCreateInfo,
      .layout = pipeline->layout,
      .renderPass = pipeline->renderPass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
  };
  result = vkCreateGraphicsPipelines(device->handle, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline->handle);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create graphics handle");
    XGLVkPipeline_destroy(pipeline);
  }
  return pipeline;
}

XGLVkShaderCreatePack *composeShaderModules(XGLVkDevice *device, XGLVkShader *shaders, uint32_t shaderCount, const Allocator *allocator) {
  XGLVkShaderCreatePack *infoPack = allocator->calloc(1, sizeof(XGLVkShaderCreatePack));
  infoPack->allocator = allocator;
  infoPack->device = device;
  infoPack->modules = allocator->calloc(shaderCount, sizeof(VkShaderModule));
  infoPack->infos = allocator->calloc(shaderCount, sizeof(VkPipelineShaderStageCreateInfo));
  VkShaderModuleCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .codeSize = 0,
      .pCode = nullptr,
  };
  for (uint32_t i = 0; i < shaderCount; i++) {
    info.codeSize = shaders[i].size;
    info.pCode = shaders[i].code;
    VkResult result = vkCreateShaderModule(device->handle, &info, nullptr, &infoPack->modules[i]);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create vertex shader module");
      infoPack->count = i;
      return infoPack;
    }
    infoPack->infos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    infoPack->infos[i].pNext = nullptr;
    infoPack->infos[i].flags = 0;
    infoPack->infos[i].stage = shaders[i].stage;
    infoPack->infos[i].module = infoPack->modules[i];
    infoPack->infos[i].pName = shaders[i].entryPoint;
    infoPack->infos[i].pSpecializationInfo = nullptr;
  }
  infoPack->count = shaderCount;
  return infoPack;
}

void XGLVkShaderCreatePack_destroy(XGLVkShaderCreatePack *pack) {
  if (pack->modules) {
    for (uint32_t i = 0; i < pack->count; i ++) {
      if (pack->modules[i] != VK_NULL_HANDLE) {
        vkDestroyShaderModule(pack->device->handle, pack->modules[i], nullptr);
      }
    }
    pack->allocator->free(pack->modules);
  }
  if (pack->infos) {
    pack->allocator->free(pack->infos);
  }
  pack->allocator->free(pack);
}

void XGLVkPipeline_destroy(XGLVkPipeline *pipeline) {
  if (pipeline->handle != VK_NULL_HANDLE) {
    vkDestroyPipeline(pipeline->device->handle, pipeline->handle, nullptr);
  }
  if (pipeline->renderPass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(pipeline->device->handle, pipeline->renderPass, nullptr);
  }
  if (pipeline->layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(pipeline->device->handle, pipeline->layout, nullptr);
  }
}