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
 * Module Name: src
 * Filename: XGLVkSurface.c
 * Creator: Yaokai Liu
 * Create Date: 2025-04-30
 * Copyright (c) 2025 Yaokai Liu. All rights reserved.
 **/

#include "XGLVkSurface.h"
#include "runtime-msg.h"
#if defined(XGL_WM_USING_GLFW)
  #define GLFW_INCLUDE_VULKAN
  #include "GLFW/glfw3.h"
#else
#error "Please implement other window manager compatibility functions."
#endif
#include "XGLVkInstance.h"
#include "XGLVkPhysicalDevice.h"
#include "utils.h"

VkPresentModeKHR PRESENT_MODE = VK_PRESENT_MODE_FIFO_KHR;
#if defined(XGL_SURFACE_WIN32)
VkSurfaceFormatKHR SURFACE_FORMAT = { .format = VK_FORMAT_R8G8B8A8_SRGB, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR };
#elif defined(XGL_SURFACE_WAYLAND)
VkSurfaceFormatKHR SURFACE_FORMAT = { .format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR };
#endif

XGLVkSurface *XGLVkSurface_new(const XGLVkPhysicalDevice *device, XGLWMWindow *window, const XGLVkInstance *instance,
                               const Allocator *allocator) {
  XGLVkSurface *surface = allocator->calloc(1, sizeof(XGLVkSurface));
  surface->allocator = allocator;
  surface->format = SURFACE_FORMAT;
  surface->presentMode = PRESENT_MODE;
  surface->instance = instance;
  surface->window = window;
  surface->device = device;
  VkResult result = glfwCreateWindowSurface(instance->handle, window, nullptr, &surface->handle);
  if (result != VK_SUCCESS) {
    rt_error("Failed to create window handle: %u", result);
    return nullptr;
  }
  XGLVkSurface_update(surface);

  uint32_t modeCount = 0;
  uint32_t formatCount = 0;
  VkPresentModeKHR *modes = nullptr;
  VkSurfaceFormatKHR *formats = nullptr;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->handle, surface->handle, &formatCount, nullptr);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device->handle, surface->handle, &modeCount, nullptr);
  if (!modeCount || !formatCount) {
    XGLVkSurface_destroy(surface);
    return nullptr;
  }
  modes = surface->allocator->calloc(modeCount, sizeof(VkPresentModeKHR));
  formats = surface->allocator->calloc(formatCount, sizeof(VkSurfaceFormatKHR));
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->handle, surface->handle, &formatCount, formats);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device->handle, surface->handle, &modeCount, modes);
  for (uint32_t i = 0; i < modeCount; i ++) {
    rt_message("found present presentMode: %u", PRESENT_MODE);
    if (modes[i] == PRESENT_MODE) {
      goto __present_mode_verified;
    }
  }
  rt_error("Not supported present presentMode: %u", PRESENT_MODE);
  surface->allocator->free(formats);
  surface->allocator->free(modes);
  XGLVkSurface_destroy(surface);
  return nullptr;
  __present_mode_verified:
  rt_message("Chosen present presentMode: %u", PRESENT_MODE);
  for (uint32_t i = 0; i < formatCount; i ++) {
    rt_message("Found surface format: %u(ColorSpace: %u)",
               formats[i].format, formats[i].colorSpace);
    if (formats[i].format == SURFACE_FORMAT.format
        && formats[i].colorSpace == SURFACE_FORMAT.colorSpace) {
      goto __surface_format_verified;
    }
  }
  rt_error("Not supported surface format: %u(ColorSpace: %u)",
           SURFACE_FORMAT.format, SURFACE_FORMAT.colorSpace);
  surface->allocator->free(formats);
  surface->allocator->free(modes);
  XGLVkSurface_destroy(surface);
  return nullptr;
  __surface_format_verified:
  rt_message("Chosen surface format: %u(ColorSpace: %u)",
             SURFACE_FORMAT.format, SURFACE_FORMAT.colorSpace);
  surface->allocator->free(formats);
  surface->allocator->free(modes);

  return surface;
}

void XGLVkSurface_destroy(XGLVkSurface *surface) {
  if (surface->handle != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(surface->instance->handle, surface->handle, nullptr);
  }
  surface->allocator->free(surface);
}

void XGLVkSurface_update(XGLVkSurface *surface) {
  int width = 0, height = 0;
  VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
  glfwGetFramebufferSize(surface->window, &width, &height);

  const XGLVkPhysicalDevice *device = surface->device;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->handle, surface->handle, &surfaceCapabilities);
  surface->extent.width = maxmin((uint32_t) width, surfaceCapabilities.maxImageExtent.width, surfaceCapabilities.minImageExtent.width);
  surface->extent.height = maxmin((uint32_t) height, surfaceCapabilities.maxImageExtent.height, surfaceCapabilities.minImageExtent.height);
  surface->swapImageCount = maxmin(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
  surface->transform = surfaceCapabilities.currentTransform;
  surface->viewport.x = 0.0f;
  surface->viewport.y = 0.0f;
  surface->viewport.width = (float) surface->extent.width;
  surface->viewport.height = (float) surface->extent.height;
  surface->viewport.minDepth = -1.0f;
  surface->viewport.maxDepth =  1.0f;
  surface->scissor.offset.x = 0;
  surface->scissor.offset.y = 0;
  surface->scissor.extent = surface->extent;
}