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
 * Filename: XGLVkRenderPass.c
 * Creator: Yaokai Liu
 * Create Date: 2025-11-04
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkRenderPass.h"
#include "XGLVkDevice.h"
#include "XGLVkSurface.h"
#include "runtime-msg.h"

XGLVkRenderPass *XGLVkRenderPass_new(const XGLVkDevice *device, const XGLVkSurface *surface, const Allocator *allocator) {

  VkAttachmentReference colorAttachmentReference = {
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };
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
  VkRenderPass handle = VK_NULL_HANDLE;
  VkResult result = vkCreateRenderPass(device->handle, &renderPassCreateInfo, nullptr, &handle);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create render pass");
    return nullptr;
  }
  XGLVkRenderPass *pass = allocator->calloc(1, sizeof(XGLVkRenderPass));
  pass->allocator = allocator;
  pass->surface = surface;
  pass->handle = handle;
  pass->device = device;
  return pass;
}

void XGLVkRenderPass_destroy(XGLVkRenderPass *pass) {
  vkDestroyRenderPass(pass->device->handle, pass->handle, nullptr);
  pass->allocator->free(pass);
}
