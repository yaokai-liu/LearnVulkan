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
 * Filename: callback.c
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include <vulkan/vulkan.h>
#include "callback.h"
#include "runtime-msg.h"

void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
  bool *framebufferResized = glfwGetWindowUserPointer(window);
  *framebufferResized = true;
  while (width == 0 || height == 0) {
    rt_message("minimize window");
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
}

const char *MESSAGE_TYPES[] = {
    "General", "Validation", "Performance", "DeviceAddressBinding"
};

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData[[maybe_unused]]) {
  if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT) {
    messageType = 3;
  } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
    messageType = 2;
  } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
    messageType = 1;
  } else {
//    messageType = 0;
    return VK_TRUE;
  }
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    rt_debug("(%s) %s", MESSAGE_TYPES[messageType], pCallbackData->pMessage);
  }
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    rt_message("(%s) %s", MESSAGE_TYPES[messageType], pCallbackData->pMessage);
  }
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    rt_warning("(%s) %s", MESSAGE_TYPES[messageType], pCallbackData->pMessage);
  }
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    rt_error("(%s) %s", MESSAGE_TYPES[messageType], pCallbackData->pMessage);
  }
  return VK_FALSE;
}
