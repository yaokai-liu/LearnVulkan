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
 * Filename: XGLVkDevice.c
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkDevice.h"
#include "XGLVkPhysicalDevice.h"
#include "XGLVkSurface.h"
#include "runtime-msg.h"
#include "util-macro.h"

const char* REQUIRED_DEVICE_LAYER_NAMES[] = {
};
const char* REQUIRED_DEVICE_EXTENSION_NAMES[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
const uint32_t REQUIRED_DEVICE_LAYER_NAME_COUNT = lenof(REQUIRED_DEVICE_LAYER_NAMES);
const uint32_t REQUIRED_DEVICE_EXTENSION_NAME_COUNT = lenof(REQUIRED_DEVICE_EXTENSION_NAMES);

XGLVkDevice *
XGLVkDevice_new(const XGLVkPhysicalDevice *physicalDevice, const XGLVkSurface *surface, const Allocator *allocator) {
  // select queue family
  uint32_t queueFamilyIndices[2] = {};
  uint32_t queueFamilyCount = Array_length(physicalDevice->queueFamilies);
  VkQueueFamilyProperties *queueFamilies = Array_first_real(physicalDevice->queueFamilies);
  for (uint32_t j = 0; j < queueFamilyCount; j++) {
    if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      rt_message("Graphics queue family found");
      queueFamilyIndices[GRAPHICS_QUEUE_INDEX] = j;
      goto __graphics_queue_found;
    }
  }
  rt_message("No suitable graphics queue family found for '%s'",
             physicalDevice->properties.deviceName);
  return nullptr;
  __graphics_queue_found:
  for (uint32_t j = 0; j < queueFamilyCount; j++) {
    VkBool32 supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice->handle, j,
                                         surface->handle, &supported);
    if (supported) {
      rt_message("Present queue family found");
      queueFamilyIndices[PRESENT_QUEUE_INDEX] = j;
      goto __physical_device_found;
    }
  }
  rt_error("No suitable present queue family found for '%s'",
           physicalDevice->properties.deviceName);
  return nullptr;
  __physical_device_found:
  rt_message("Queue Family selected.");

  float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo queueCreateInfos[] = {
      [GRAPHICS_QUEUE_INDEX] = {
          .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .queueFamilyIndex = queueFamilyIndices[GRAPHICS_QUEUE_INDEX],
          .queueCount = 1,
          .pQueuePriorities = &queuePriority,
      },
      [PRESENT_QUEUE_INDEX] = {
          .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .queueFamilyIndex = queueFamilyIndices[PRESENT_QUEUE_INDEX],
          .queueCount = 1,
          .pQueuePriorities = &queuePriority,
      }
  };
  VkDeviceCreateInfo logicDeviceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueCreateInfoCount = (queueFamilyIndices[1] == queueFamilyIndices[0]) ? 1 : 2,
      .pQueueCreateInfos = queueCreateInfos,
      .enabledLayerCount = lenof(REQUIRED_DEVICE_LAYER_NAMES),
      .ppEnabledLayerNames = REQUIRED_DEVICE_LAYER_NAMES,
      .enabledExtensionCount = lenof(REQUIRED_DEVICE_EXTENSION_NAMES),
      .ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSION_NAMES,
      .pEnabledFeatures = &physicalDevice->features,
  };
  VkDevice handle = VK_NULL_HANDLE;
  VkResult result = vkCreateDevice(physicalDevice->handle, &logicDeviceCreateInfo, nullptr, &handle);
  if (result != VK_SUCCESS) {
    switch (result) {
      case VK_ERROR_EXTENSION_NOT_PRESENT: {
        rt_error("Not supported extension set on");
        break;
      }
      case VK_ERROR_LAYER_NOT_PRESENT: {
        rt_error("Not supported layer set on");
        break;
      }
      default: {
        rt_error("error occurred %u", result);
      }
    }
    return nullptr;
  }
  XGLVkQueue queues[2] = { VK_NULL_HANDLE };
  rt_message("Logic handle created");
  vkGetDeviceQueue(handle, queueFamilyIndices[GRAPHICS_QUEUE_INDEX],
                   0, &queues[GRAPHICS_QUEUE_INDEX].queue);
  rt_message("Graphics queue selected");
  vkGetDeviceQueue(handle, queueFamilyIndices[PRESENT_QUEUE_INDEX],
                   0, &queues[PRESENT_QUEUE_INDEX].queue);
  queues[GRAPHICS_QUEUE_INDEX].index = queueFamilyIndices[GRAPHICS_QUEUE_INDEX];
  queues[PRESENT_QUEUE_INDEX].index = queueFamilyIndices[PRESENT_QUEUE_INDEX];
  rt_message("Present queue selected");
  XGLVkDevice *device = allocator->calloc(1, sizeof(XGLVkDevice));
  device->handle = handle;
  device->queues = Array_new(sizeof(XGLVkQueue), -1, allocator);
  Array_append(device->queues, queues, 2);
  rt_message("Logical device created");
  return device;
}

void XGLVkDevice_destroy(XGLVkDevice *device) {
  if (device->handle != VK_NULL_HANDLE) {
    vkDestroyDevice(device->handle, nullptr);
  }
  if (device->queues) {
    releasePrimeArray(device->queues);
  }
}

VkResult XGLVkDevice_render(XGLVkDevice *device, VkCommandBuffer command, VkSwapchainKHR *swapchains,
                            const XGLVkRenderInfo *renderInfo) {
  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .waitSemaphoreCount = renderInfo->waitSemCount,
      .pWaitSemaphores = renderInfo->presentSemaphores,
      .pWaitDstStageMask = renderInfo->waitStageFlags,
      .commandBufferCount = 1,
      .pCommandBuffers = &command,
      .signalSemaphoreCount = renderInfo->signalSemCount,
      .pSignalSemaphores = renderInfo->submitSemaphores,
  };
  XGLVkQueue *graphicsQueue = Array_real_addr(device->queues, GRAPHICS_QUEUE_INDEX);
  VkResult result = vkQueueSubmit(graphicsQueue->queue, 1, &submitInfo, renderInfo->waitFence);
  if (result != VK_SUCCESS) {
    rt_error("Failed to submit render command to queue");
    return result;
  }
  VkPresentInfoKHR presentInfo = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = renderInfo->submitSemaphores,
      .swapchainCount = renderInfo->swapchainCount,
      .pSwapchains = swapchains,
      .pImageIndices = &renderInfo->imageIndex,
      .pResults = nullptr,
  };
  XGLVkQueue *presentQueue = Array_real_addr(device->queues, GRAPHICS_QUEUE_INDEX);
  result = vkQueuePresentKHR(presentQueue->queue, &presentInfo);
  if (result != VK_SUCCESS) {
    rt_error("Failed to present to surface");
    return result;
  }
  return result;
}
