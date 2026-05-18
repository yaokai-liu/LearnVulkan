
#include "xgl.h"
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "allocator.h"
#include "runtime-msg.h"
#include "callback.h"
#include "utils.h"
#include "xGLVulkan/XGLVkCommand.h"
#include "xGLVulkan/XGLVkDescriptor.h"
#include "xGLVulkan/XGLVkDevice.h"
#include "xGLVulkan/XGLVkDeviceBuffer.h"
#include "xGLVulkan/XGLVkInstance.h"
#include "xGLVulkan/XGLVkPipeline.h"
#include "xGLVulkan/XGLVulkan.h"
#include "xGLVulkan/XGLVkRenderPass.h"
#include "xGLVulkan/XGLVkSurface.h"
#include "xGLVulkan/XGLVkSwapchain.h"
#include "xGLVulkan/XGLVkThreadSync.h"

uint8_t *loadBinFile(const char *filepath, uint32_t *real_size, const Allocator *allocator);

void processInput(GLFWwindow *window);

static const float DEFAULT_FRAME_WIDTH = 1000.0f;
static const float DEFAULT_FRAME_HEIGHT = 1000.0f;
static bool framebufferResized = false;

const XGLSoftwareInfo APPLICATION = {.name = "Vulkan Demo", .version = VK_MAKE_VERSION(0, 0, 1)};
const XGLSoftwareInfo ENGINE = {.name = "XGL Engine", .version = VK_MAKE_VERSION(0, 0, 1)};

int main(int , char * []) {
  VkResult result = {};

  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  const Allocator *allocator = &STDAllocator;

  GLFWwindow *window = glfwCreateWindow(1000, 1000, "Vulkan Window", nullptr, nullptr);
  if (window == NULL) {
    fprintf(stderr, "Failed to create GLFW window.\n");
    glfwTerminate();
    return -1;
  }

  glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
  glfwSetWindowUserPointer(window, &framebufferResized);
  glfwMakeContextCurrent(window);

  XGLVkInstance *instance = XGLVkInstance_new(&APPLICATION, &ENGINE, allocator);
  const XGLVkPhyDevice *physicalDevice = XGLVkInstance_pickPhysicalDevice(window, instance);
  XGLVkSurface *surface = XGLVkSurface_new(physicalDevice, window, instance, allocator);
  XGLVkDevice *logicalDevice = XGLVkDevice_new(physicalDevice, surface, allocator);

  XGLVkCommandPool *commandPool = XGLVkCommandPool_new(logicalDevice, allocator);
  XGLVkCommandPool_newCommand(commandPool, MAX_FRAME_ON_DRAW);
  const XGLVertex cube_vertices[] = {
      {{-0.5f, -0.5f,  0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f},{}},
      {{ 0.5f, -0.5f,  0.5f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f},{}},
      {{ 0.5f,  0.5f,  0.5f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f},{}},
      {{-0.5f,  0.5f,  0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f},{}},
      {{-0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f},{}},
      {{ 0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f},{}},
      {{ 0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f},{}},
      {{-0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f},{}},
  };
  const uint32_t cube_vert_indices[] = {
      0, 1, 2, 0, 2, 3,
      4, 6, 5, 4, 7, 6,
      0, 4, 5, 0, 5, 1,
      3, 6, 7, 3, 2, 6,
      1, 5, 6, 1, 6, 2,
      4, 0, 3, 4, 3, 7
  };
  XGLVkBufferInfo deviceBufferInfos[2] = {
      {
          .flags = 0, .size = sizeof(cube_vertices), .memoryOffset = 0,
          .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
          .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
          .pQueueFamilyIndices = nullptr, .queueFamilyIndexCount = 0,
      },
      {
          .flags = 0, .size = sizeof(cube_vert_indices), .memoryOffset = sizeof(cube_vertices),
          .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
          .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
          .pQueueFamilyIndices = nullptr, .queueFamilyIndexCount = 0,
      }
  };
  XGLVkDeviceBufferGroup *deviceBufferGroup = XGLVkDeviceBufferGroup_new(logicalDevice, allocator);
  XGLVkDeviceBufferGroup_allocBuffers(deviceBufferGroup, deviceBufferInfos, 2, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  /** trans buffer data to device */ {
    XGLVkBufferTransInfo transInfos[] = {
      { .dstBufferIndex = 0, .dstOffset = 0, .size = sizeof(cube_vertices), .data = cube_vertices },
      { .dstBufferIndex = 1, .dstOffset = 0, .size = sizeof(cube_vert_indices), .data = cube_vert_indices }
    };
    XGLVkDeviceBuffer_cmdTransBufferData(deviceBufferGroup, transInfos, 2);
  }

  XGLVkBufferInfo uniformBufferInfos[MAX_FRAME_ON_DRAW] = {};
  for (uint32_t i = 0; i < MAX_FRAME_ON_DRAW; i ++) {
      uniformBufferInfos[i].flags = 0;
      uniformBufferInfos[i].size = sizeof(XGLModelViewProjection);
      uniformBufferInfos[i].usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      uniformBufferInfos[i].sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      uniformBufferInfos[i].memoryOffset = i * sizeof(XGLModelViewProjection);
      uniformBufferInfos[i].queueFamilyIndexCount = 0;
      uniformBufferInfos[i].pQueueFamilyIndices = nullptr;
  };
  XGLVkDeviceBufferGroup *uniformGroup = XGLVkDeviceBufferGroup_new(logicalDevice, allocator);
  XGLVkDeviceBufferGroup_allocBuffers(uniformGroup, uniformBufferInfos, MAX_FRAME_ON_DRAW,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  XGLVkRenderPass *renderPass = XGLVkRenderPass_new(logicalDevice, surface, allocator);
  XGLVkPipelineInfo pipeInfo = { .device = logicalDevice, .allocator = allocator };
  uint32_t vertShaderCodeSize = 0;
  uint32_t fragShaderCodeSize = 0;
  uint8_t *vertShaderCode = loadBinFile("shaders/vert.spv", &vertShaderCodeSize, allocator);
  uint8_t *fragShaderCode = loadBinFile("shaders/frag.spv", &fragShaderCodeSize, allocator);
  XGLVkShaderInfo shaderInfos[2] = {
      {.stage = VK_SHADER_STAGE_VERTEX_BIT, .entryPoint = "main", .code = (uint32_t *) vertShaderCode, .size = vertShaderCodeSize},
      {.stage = VK_SHADER_STAGE_FRAGMENT_BIT, .entryPoint = "main", .code = (uint32_t *) fragShaderCode, .size =fragShaderCodeSize},
  };
  result = composeShaderModules(&pipeInfo, shaderInfos, 2);
  if (result != VK_SUCCESS) { return -1; }
  VkDescriptorSetLayoutBinding mvpLayoutBinding = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
  };
  result = composeSetLayouts(&pipeInfo, 1, &mvpLayoutBinding);
  if (result != VK_SUCCESS) { return -1; }
  VkDescriptorSetLayout setLayouts[MAX_FRAME_ON_DRAW] = {};
  for (uint32_t i = 0; i < MAX_FRAME_ON_DRAW; i ++) {
      setLayouts[i] = *(VkDescriptorSetLayout *) Array_first_real(pipeInfo.descriptorSetLayouts);
  }
  VkDescriptorSetAllocateInfo setAllocateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .pNext = nullptr,
      .descriptorPool = logicalDevice->descPool->handle, .descriptorSetCount = MAX_FRAME_ON_DRAW,
      .pSetLayouts = setLayouts,
  };
  VkDescriptorSet descriptorSets[MAX_FRAME_ON_DRAW] = {};
  vkAllocateDescriptorSets(logicalDevice->handle, &setAllocateInfo, descriptorSets);
  for (uint32_t i = 0; i < MAX_FRAME_ON_DRAW; i ++) {
    VkDescriptorBufferInfo bufferInfo = {
        .buffer = *(VkBuffer*)Array_real_addr(uniformGroup->buffers, i),
        .offset = 0, .range = sizeof(XGLModelViewProjection)
    };
    VkWriteDescriptorSet writeInfo = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .pNext = nullptr,
        .dstSet = descriptorSets[i], .dstBinding = 0, .dstArrayElement = 0,
        .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr, .pBufferInfo = &bufferInfo, .pTexelBufferView = nullptr,
    };
    vkUpdateDescriptorSets(logicalDevice->handle, 1, &writeInfo, 0, nullptr);
  }
  VkVertexInputBindingDescription vertInputBindingDescriptions[] = {
      { .binding = 0, .stride = sizeof(XGLVertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }
  };
  VkVertexInputAttributeDescription vertInputAttrDescriptions[2] = {
      { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(XGLVertex, coord) },
      { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(XGLVertex, color) }
  };
  result = composeVertexInputs(&pipeInfo, 1, vertInputBindingDescriptions, 2, vertInputAttrDescriptions);
  if (result != VK_SUCCESS) { return -1; }
  XGLVkPipeline * pipeline = XGLVkPipeline_new(logicalDevice, renderPass, &pipeInfo, allocator);
  XGLVkPipelineInfo_release(&pipeInfo);

  XGLVkSwapchain *swapchain = XGLVkSwapchain_new(logicalDevice, renderPass, pipeline, allocator);

  XGLModelViewProjection mvp = {};
  floatMatDiag(mvp.model, (FVec4) {1.0f, 1.0f, 1.0f, 1.0f});
  matAffineRotate(mvp.model, (FVec4){0.0f, 0.0f, 1.0f, 0.0f}, M_PI / 3);
  matFromLookAt(mvp.view, (FAffPoint4){2.0f, 2.0f, 2.0f, 1.0f},
                (FVec4){-2.0f, -2.0f, -2.0f, 0.0f},
                (FVec4){0.0f, 1.0f, 0.0f, 0.0f});
  // matFromOrthoProjection(mvp.proj,
  //                       (FVec2) {-2.0f, 2.0f},
  //                       (FVec2) {-2.0f, 2.0f},
  //                       (FVec2)  {-2.0f, 2.0f});
  matFromPersProjection(mvp.proj,
                        (FVec2) {-2.0f, 2.0f},
                        (FVec2) {-2.0f, 2.0f},
                        (FVec2) {2.0f, 6.0f});

  XGLModelViewProjection * const uboMapped = XGLVkDeviceBufferGroup_mapping(
    uniformGroup, 0, MAX_FRAME_ON_DRAW * sizeof(XGLModelViewProjection)
  );
  clock_t start = clock();

  XGLVkSemaphoreGroup *imgAvailableSemGroup = XGLVkSemaphoreGroup_new(logicalDevice, nullptr, MAX_FRAME_ON_DRAW, allocator);
  XGLVkSemaphoreGroup *rendFinishedSemGroup = XGLVkSemaphoreGroup_new(logicalDevice, nullptr, MAX_FRAME_ON_DRAW, allocator);
  VkFenceCreateFlags fenceFlags[MAX_FRAME_ON_DRAW] = {[0] = VK_FENCE_CREATE_SIGNALED_BIT, [1] = VK_FENCE_CREATE_SIGNALED_BIT};
  XGLVkFenceGroup *presentFiniFenGroup = XGLVkFenceGroup_new(logicalDevice, fenceFlags, MAX_FRAME_ON_DRAW, allocator);
  while(!glfwWindowShouldClose(window)) {
    processInput(window);
    glfwPollEvents();
    /* draw */ {
      vkWaitForFences(logicalDevice->handle, 1, &presentFiniFenGroup->fences[swapchain->swapImageIndex],
                      VK_TRUE, UINT32_MAX);
      if (framebufferResized) {
        uint32_t imageIndex = swapchain->swapImageIndex;
        framebufferResized = false;
        vkDeviceWaitIdle(logicalDevice->handle);
        XGLVkSurface_update(surface);
        XGLVkSwapchain_destroy(swapchain);
        swapchain = XGLVkSwapchain_new(logicalDevice, renderPass, pipeline, allocator);
        swapchain->swapImageIndex = imageIndex;
        const float scales[2] = {
            ((float) surface->extent.width) / DEFAULT_FRAME_WIDTH,
            ((float) surface->extent.height) / DEFAULT_FRAME_HEIGHT
        };
        // matFromOrthoProjection(mvp.proj,
        //                       (FVec2) {-2.0f * scales[0], 2.0f * scales[0]},
        //                       (FVec2) {-2.0f * scales[1], 2.0f * scales[1]},
        //                       (FVec2)  {-2.0f, 2.0f});
        matFromPersProjection(mvp.proj,
                              (FVec2) {-2.0f * scales[0], 2.0f * scales[0]},
                              (FVec2) {-2.0f * scales[1], 2.0f * scales[1]},
                              (FVec2) {2.0f, 6.0f});
      }
      VkSemaphore waiteSemaphores[] = { imgAvailableSemGroup->semaphores[swapchain->swapImageIndex] };
      result = vkAcquireNextImageKHR(logicalDevice->handle, swapchain->handle, UINT32_MAX,
                                     imgAvailableSemGroup->semaphores[swapchain->swapImageIndex], VK_NULL_HANDLE,
                                     &swapchain->swapImageIndex);
      if (result != VK_SUCCESS) { break; }
      VkSemaphore signaledSemaphores[] = { rendFinishedSemGroup->semaphores[swapchain->swapImageIndex] };
      VkPipelineStageFlags waiteStageFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
      VkCommandBuffer command = XGLVkCommandPool_getCommand(commandPool, swapchain->swapImageIndex);
      XGLVkRecordInfo renderInfo = {
          .clearValue.color.float32 = {0.16f, 0.16f, 0.16f, 1.0f},
          .submitSemaphores = signaledSemaphores, .presentSemaphores = waiteSemaphores,
          .imageIndex = swapchain->swapImageIndex, .submitSemCount = 1, .presentSemCount = 1, .swapchainCount = 1,
          .waitFence = presentFiniFenGroup->fences[swapchain->swapImageIndex],
          .waitStageFlags = waiteStageFlags
      };
      vkResetFences(logicalDevice->handle, 1, &presentFiniFenGroup->fences[swapchain->swapImageIndex]);


      float angle = (float) (((double) (clock() - start)) / CLOCKS_PER_SEC);
      matFromAffineRotate(mvp.model, (FVec4) {0.0f, -1.0f, 0.0f, 0.0f}, angle);
      // matAffineShift(mvp.model, (FVec4) {0.0f, 0.0f, 0.4f * sinf(2 * angle), 0.0f});
      uboMapped[swapchain->swapImageIndex] = mvp;

      result = XGLVkCommand_startRecord(command, renderPass, swapchain, pipeline, &renderInfo);
      if (result != VK_SUCCESS) { break; }
      /* record draw command */ {
        XGLVkCommand_adjustDevice(command, logicalDevice);
        VkBuffer *vertexBuffers = Array_real_addr(deviceBufferGroup->buffers, 0);
        VkBuffer *indexBuffers = Array_real_addr(deviceBufferGroup->buffers, 1);
        VkDeviceSize offset = { 0 };
        vkCmdBindVertexBuffers(command, 0, 1, vertexBuffers, &offset);
        vkCmdBindIndexBuffer(command, *indexBuffers, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout,
                                0, 1, &descriptorSets[swapchain->swapImageIndex],
                                0, nullptr);
        vkCmdDrawIndexed(command, lenof(cube_vert_indices), 1, 0, 0, 0);
      }
      result = XGLVkCommand_endRecord(command);
      if (result != VK_SUCCESS) { break; }

      VkSwapchainKHR swapchains[] = {swapchain->handle};
      result = XGLVkDevice_render(logicalDevice, command, swapchains, &renderInfo);
    }
  }

  vkDeviceWaitIdle(logicalDevice->handle);

  XGLVkDeviceBufferGroup_destroy(uniformGroup);
  XGLVkDeviceBufferGroup_destroy(deviceBufferGroup);
  XGLVkSemaphoreGroup_destroy(rendFinishedSemGroup);
  XGLVkSemaphoreGroup_destroy(imgAvailableSemGroup);
  XGLVkFenceGroup_destroy(presentFiniFenGroup);
  XGLVkCommandPool_destroy(commandPool);
  XGLVkSwapchain_destroy(swapchain);
  XGLVkPipeline_destroy(pipeline);
  XGLVkRenderPass_destroy(renderPass);
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
