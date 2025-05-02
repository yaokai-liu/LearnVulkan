
#include "xgl.h"
#include <stdio.h>
#include <string.h>
#include "allocator.h"
#include "runtime-msg.h"
#include "callback.h"
#include "util-macro.h"

uint8_t *loadBinFile(const char *filepath, uint32_t *real_size, const Allocator *allocator);

void processInput(GLFWwindow *window);

static bool framebufferResized = false;

const XGLSoftware APPLICATION = {.name = "Vulkan Demo", .version = VK_MAKE_VERSION(0, 0, 1)};
const XGLSoftware ENGINE = {.name = "XGL Engine", .version = VK_MAKE_VERSION(0, 0, 1)};

int main(int argc, char * argv[]) {
  VkResult result = {};

  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  const Allocator *allocator = &STDAllocator;

  GLFWwindow *window = glfwCreateWindow(1920 >> 1, (1080 >> 2) * 3, "Vulkan Window", nullptr, nullptr);
  if (window == NULL) {
    fprintf(stderr, "Failed to create GLFW window.\n");
    glfwTerminate();
    return -1;
  }

  glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
  glfwSetWindowUserPointer(window, &framebufferResized);
  glfwMakeContextCurrent(window);

  XGLVkInstance *instance = XGLVkInstance_new(&APPLICATION, &ENGINE, allocator);
  const XGLVkPhysicalDevice *physicalDevice = XGLVkInstance_pickPhysicalDevice(window, instance);
  XGLVkSurface *surface = XGLVkSurface_new(physicalDevice, window, instance, allocator);
  XGLVkDevice *logicalDevice = XGLVkDevice_new(physicalDevice, surface, allocator);

  XGLVkPipelineInfo pipeInfo = { .device = logicalDevice, .allocator = allocator };
  uint32_t vertShaderCodeSize = 0;
  uint32_t fragShaderCodeSize = 0;
  uint8_t *vertShaderCode = loadBinFile("shaders/vert.spv", &vertShaderCodeSize, allocator);
  uint8_t *fragShaderCode = loadBinFile("shaders/frag.spv", &fragShaderCodeSize, allocator);
  XGLVkShader shaders[2] = {
      {.stage = VK_SHADER_STAGE_VERTEX_BIT, .entryPoint = "main", .code = (uint32_t *) vertShaderCode, .size = vertShaderCodeSize},
      {.stage = VK_SHADER_STAGE_FRAGMENT_BIT, .entryPoint = "main", .code = (uint32_t *) fragShaderCode, .size =fragShaderCodeSize},
  };
  result = composeShaderModules(&pipeInfo, shaders, 2);
  if (result != VK_SUCCESS) { return -1; }
  VkVertexInputBindingDescription vertInputBindingDescriptions[] = {
      { .binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }
  };
  VkVertexInputAttributeDescription vertInputAttrDescriptions[2] = {
      { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, coord) },
      { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex, color) }
  };
  result = composeVertexInputs(&pipeInfo,
                               1, vertInputBindingDescriptions,
                               2, vertInputAttrDescriptions);
  XGLVkPipeline * pipeline = XGLVkPipeline_new(logicalDevice, &pipeInfo, surface, allocator);
  XGLVkShaderCreatePack_destroy(&pipeInfo);

  XGLVkSwapchain * swapchain = XGLVkSwapchain_new(logicalDevice, surface, pipeline, allocator);
  XGLVkCommandPool *commandPool = XGLVkCommandPool_new(logicalDevice, allocator);
  XGLVkCommandPool_newCommand(commandPool, MAX_FRAME_ON_DRAW);
  XGLVkSemaphoreGroup *imgAvaSemGroup = XGLVkSemaphoreGroup_new(logicalDevice, nullptr, MAX_FRAME_ON_DRAW, allocator);
  XGLVkSemaphoreGroup *rendFiniSemGroup = XGLVkSemaphoreGroup_new(logicalDevice, nullptr, MAX_FRAME_ON_DRAW, allocator);
  VkFenceCreateFlags fenceFlags[MAX_FRAME_ON_DRAW] = {[0] = VK_FENCE_CREATE_SIGNALED_BIT, [1] = VK_FENCE_CREATE_SIGNALED_BIT};
  XGLVkFenceGroup *presentFiniFenGroup = XGLVkFenceGroup_new(logicalDevice, fenceFlags, MAX_FRAME_ON_DRAW, allocator);

  const Vertex vertices[] = {
      {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}},
      {{ 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}},
      {{ 0.5f,  0.5f}, {0.0f, 1.0f, 1.0f, 1.0f}},
      {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}},
  };
  const uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
  XGLVkBufferInfo bufferInfos[2] = {
    {
      .flags = 0, .size = sizeof(vertices), .pQueueFamilyIndices = nullptr,
      .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE, .memoryOffset = 0, .queueFamilyIndexCount = 0,
    },
    {
      .flags = 0, .size = sizeof(indices), .pQueueFamilyIndices = nullptr,
      .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE, .memoryOffset = sizeof(vertices), .queueFamilyIndexCount = 0,
    }
  };
  XGLVkDeviceBufferGroup *bufferGroup = XGLVkDeviceBufferGroup_new(
      logicalDevice, 2, bufferInfos,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocator);
  uint32_t sizes[] = {sizeof(vertices), sizeof(indices)};
  uint32_t offsets[] = {0, 0};
  const void *datas[] = {vertices, indices};
  XGLVkBufferCopyInfo copyInfo = {
      .count = 2, .datas = datas, .dstBuffers = bufferGroup->buffers,
      .sizes = sizes, .dstOffsets = offsets, .sameDst = false
  };
  XGLVkDevice_cmdCopyBufferData(logicalDevice, commandPool, &copyInfo);

  while(!glfwWindowShouldClose(window)) {
    processInput(window);
    glfwPollEvents();
    /* draw */ {
      swapchain->currentIndex = swapchain->currentIndex % MAX_FRAME_ON_DRAW;
      vkWaitForFences(logicalDevice->handle, 1, &presentFiniFenGroup->fences[swapchain->currentIndex],
                      VK_TRUE, UINT32_MAX);
      if (framebufferResized) {
        framebufferResized = false;
        vkDeviceWaitIdle(logicalDevice->handle);
        XGLVkSurface_update(surface);
        XGLVkSwapchain_destroy(swapchain);
        swapchain = XGLVkSwapchain_new(logicalDevice, surface, pipeline, allocator);
      }

      VkSemaphore signaledSemaphores[] = {rendFiniSemGroup->semaphores[swapchain->currentIndex]};
      VkSemaphore waiteSemaphores[] = {imgAvaSemGroup->semaphores[swapchain->currentIndex]};
      VkPipelineStageFlags waiteStageFlags[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
      VkCommandBuffer command = XGLVkCommandPool_getCommand(commandPool, swapchain->currentIndex);
      XGLVkRecordInfo renderInfo = {
          .clearValue.color = {0.15f, 0.16f, 0.18f, 1.0f},
          .submitSemaphores = signaledSemaphores, .presentSemaphores = waiteSemaphores,
          .imageIndex = 0,.signalSemCount = 1, .waitSemCount = 1, .swapchainCount = 1,
          .waitFence = presentFiniFenGroup->fences[swapchain->currentIndex],
          .waitStageFlags = waiteStageFlags
      };
      result = vkAcquireNextImageKHR(logicalDevice->handle, swapchain->handle, UINT32_MAX,
                                     imgAvaSemGroup->semaphores[swapchain->currentIndex], VK_NULL_HANDLE,
                                     &renderInfo.imageIndex);
      if (result != VK_SUCCESS) { break; }
      vkResetFences(logicalDevice->handle, 1, &presentFiniFenGroup->fences[swapchain->currentIndex]);

      result = XGLVkCommand_startRecord(command, surface, swapchain, pipeline, &renderInfo);
      if (result != VK_SUCCESS) { break; }
      VkBuffer vertexBuffer = bufferGroup->buffers[0];
      VkBuffer indexBuffer = bufferGroup->buffers[1];
      VkDeviceSize offset = {0};
      vkCmdBindVertexBuffers(command, 0, 1, &vertexBuffer, &offset);
      vkCmdBindIndexBuffer(command, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
      vkCmdDrawIndexed(command, lenof(indices), 1, 0, 0, 0);
      result = XGLVkCommand_endRecord(command);
      if (result != VK_SUCCESS) { break; }

      VkSwapchainKHR swapchains[] = {swapchain->handle};
      result = XGLVkDevice_render(logicalDevice, command, swapchains, &renderInfo);
      swapchain->currentIndex++;
    }
  }

  vkDeviceWaitIdle(logicalDevice->handle);

  XGLVkDeviceBufferGroup_destroy(bufferGroup);
  XGLVkSemaphoreGroup_destroy(rendFiniSemGroup);
  XGLVkSemaphoreGroup_destroy(imgAvaSemGroup);
  XGLVkFenceGroup_destroy(presentFiniFenGroup);
  XGLVkCommandPool_destroy(commandPool);
  XGLVkSwapchain_destroy(swapchain);
  XGLVkPipeline_destroy(pipeline);
  XGLVkDevice_destroy(logicalDevice);
  XGLVkSurface_destroy(surface);
  XGLVkInstance_destroy(instance);

  glfwDestroyWindow(window);
  glfwTerminate();

  return result;
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

uint8_t *loadBinFile(const char *filepath, uint32_t *real_size, const Allocator *allocator) {
  uint32_t fileSize = 0;
  FILE *file = nullptr;
  file = fopen(filepath, "rb");
  if (!file) {
    rt_error("Failed to open binary file '%s'", filepath);
    return nullptr;
  }
  fseek(file, 0, SEEK_END);
  fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  uint8_t *content = allocator->calloc(((fileSize - 1) / sizeof(uint32_t) + 1), sizeof(uint32_t));
  *real_size = fread(content, sizeof(uint8_t), fileSize, file);
  return content;
}
