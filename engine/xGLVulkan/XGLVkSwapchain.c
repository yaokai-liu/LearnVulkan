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
 * Filename: XGLVkSwapchain.c
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkSwapchain.h"
#include "XGLVkPipeline.h"
#include "XGLVkSurface.h"
#include "XGLVkDevice.h"
#include "runtime-msg.h"
#include "XGLVkRenderPass.h"

XGLVkSwapchain *
XGLVkSwapchain_new(const XGLVkDevice *device, const XGLVkRenderPass *renderPass, const XGLVkPipeline *pipeline, const Allocator *allocator) {
  const XGLVkSurface *surface = renderPass->surface;
  const uint32_t queueCount = Array_length(device->queues);
  const XGLVkQueue *queues = Array_first_real(device->queues);
  uint32_t *queueIndices = allocator->calloc(queueCount, sizeof(uint32_t));
  for (uint32_t i = 0; i < queueCount; i++) {
    queueIndices[i] = queues[i].index;
  }
  XGLVkSwapchain *swapchain = allocator->calloc(1, sizeof(XGLVkSwapchain));
  swapchain->allocator = allocator;
  swapchain->pipeline = pipeline;
  swapchain->surface = surface;
  swapchain->device = device;
  VkSwapchainCreateInfoKHR swapchainInfo = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .surface = surface->handle,
      .minImageCount = surface->swapImageCount,
      .imageFormat = surface->format.format,
      .imageColorSpace = surface->format.colorSpace,
      .imageExtent = surface->extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = (queues[0].index == queues[1].index) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
      .queueFamilyIndexCount = (queues[0].index == queues[1].index) ? 0 : queueCount,
      .pQueueFamilyIndices = (queues[0].index == queues[1].index) ? nullptr : queueIndices,
      .preTransform = surface->transform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = surface->presentMode,
      .clipped = VK_TRUE,
      .oldSwapchain = nullptr,
  };
  VkResult result = vkCreateSwapchainKHR(device->handle, &swapchainInfo, nullptr, &swapchain->handle);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create swapchain: %u", result);
    XGLVkSwapchain_destroy(swapchain);
    return nullptr;
  }
  vkGetSwapchainImagesKHR(device->handle, swapchain->handle, &swapchain->swapImageCount, nullptr);
  VkImage *swapImages = allocator->calloc(swapchain->swapImageCount, sizeof(VkImage));
  vkGetSwapchainImagesKHR(device->handle, swapchain->handle, &swapchain->swapImageCount, swapImages);
  swapchain->swapImageViews = allocator->calloc(swapchain->swapImageCount, sizeof(VkImageView));
  VkImageViewCreateInfo imgViewInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .image = nullptr,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = surface->format.format,
      .components = {
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY,
      },
      .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
      },
  };
  for (uint32_t i = 0; i < swapchain->swapImageCount; i++) {
    imgViewInfo.image = swapImages[i];
    result = vkCreateImageView(device->handle, &imgViewInfo,
                               nullptr, &swapchain->swapImageViews[i]);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create image view: %u", result);
      XGLVkSwapchain_destroy(swapchain);
      allocator->free(swapImages);
      return nullptr;
    }
  }
  VkFramebufferCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .renderPass = renderPass->handle,
      .attachmentCount = 1,
      .pAttachments = nullptr,
      .width = surface->extent.width,
      .height = surface->extent.height,
      .layers = 1,
  };
  swapchain->framebuffers = allocator->calloc(swapchain->swapImageCount, sizeof(VkFramebuffer));
  for (uint32_t i = 0; i < swapchain->swapImageCount; i++) {
    info.pAttachments = &swapchain->swapImageViews[i];
    result = vkCreateFramebuffer(device->handle, &info, nullptr, &swapchain->framebuffers[i]);
    if (result != VK_SUCCESS) {
      rt_error("Failed to create framebuffers");
      XGLVkSwapchain_destroy(swapchain);
      allocator->free(swapImages);
      return nullptr;
    }
  }
  allocator->free(swapImages);
  return swapchain;
}

void XGLVkSwapchain_destroy(XGLVkSwapchain *swapchain) {
  if (swapchain->swapImageViews) {
    for (uint32_t j = 0; j < swapchain->swapImageCount; j ++) {
      if (swapchain->swapImageViews[j] != VK_NULL_HANDLE) {
        vkDestroyImageView(swapchain->device->handle, swapchain->swapImageViews[j], nullptr);
      }
    }
    swapchain->allocator->free(swapchain->swapImageViews);
  }
  if (swapchain->framebuffers) {
    for (uint32_t j = 0; j < swapchain->swapImageCount; j ++) {
      if (swapchain->framebuffers[j] != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(swapchain->device->handle, swapchain->framebuffers[j], nullptr);
      }
    }
    swapchain->allocator->free(swapchain->framebuffers);
  }
  if (swapchain->handle != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(swapchain->device->handle, swapchain->handle, nullptr);
  }
  swapchain->allocator->free(swapchain);
}
