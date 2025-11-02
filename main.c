
#include "xgl.h"
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "allocator.h"
#include "runtime-msg.h"
#include "callback.h"
#include "utils.h"

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
  const XGLVkPhysicalDevice *physicalDevice = XGLVkInstance_pickPhysicalDevice(window, instance);
  XGLVkSurface *surface = XGLVkSurface_new(physicalDevice, window, instance, allocator);
  XGLVkDevice *logicalDevice = XGLVkDevice_new(physicalDevice, surface, allocator);
  XGLVkCommandPool *commandPool = XGLVkCommandPool_new(logicalDevice, allocator);
  XGLVkCommandPool_newCommand(commandPool, MAX_FRAME_ON_DRAW);
  const Vertex cube_vertices[] = {
      {{-0.5f, -0.5f,  0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
      {{ 0.5f, -0.5f,  0.5f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
      {{ 0.5f,  0.5f,  0.5f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
      {{-0.5f,  0.5f,  0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
      {{-0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
      {{ 0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
      {{ 0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
      {{-0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
  };
  const uint32_t cube_vert_indices[] = {
      0, 1, 2, 0, 2, 3,
      4, 6, 5, 4, 7, 6,
      0, 4, 5, 0, 5, 1,
      3, 6, 7, 3, 2, 6,
      1, 5, 6, 1, 6, 2,
      4, 0, 3, 4, 3, 7
  };
  XGLVkBufferInfo bufferInfos[2] = {
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
  XGLVkDeviceBufferGroup *bufferGroup = XGLVkDeviceBufferGroup_new(
      logicalDevice, 2, bufferInfos,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocator);
  /** update buffer data */ {
    uint32_t sizes[] = { sizeof(cube_vertices), sizeof(cube_vert_indices) };
    uint32_t offsets[] = { 0, 0 };
    const void *datas[] = { cube_vertices, cube_vert_indices };
    XGLVkBufferCopyInfo copyInfo = {
        .count = 2, .datas = datas, .dstBuffers = bufferGroup->buffers,
        .sizes = sizes, .dstOffsets = offsets, .sameDst = false
    };
    XGLVkDevice_cmdCopyBufferData(logicalDevice, commandPool, &copyInfo);
  }
  XGLVkBufferInfo uboInfos[MAX_FRAME_ON_DRAW] = {};
  for (uint32_t i = 0; i < MAX_FRAME_ON_DRAW; i ++) {
    uboInfos[i].flags = 0;
    uboInfos[i].size = sizeof(MVP);
    uboInfos[i].usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    uboInfos[i].sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uboInfos[i].memoryOffset = i * sizeof(MVP);
    uboInfos[i].queueFamilyIndexCount = 0;
    uboInfos[i].pQueueFamilyIndices = nullptr;
  };
  XGLVkDeviceBufferGroup *uboGroup  = XGLVkDeviceBufferGroup_new(
      logicalDevice, MAX_FRAME_ON_DRAW, uboInfos,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, allocator
  );

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
  VkDescriptorSetLayoutBinding mvpLayoutBinding = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
  };
  result = composeSetLayouts(&pipeInfo, 1, &mvpLayoutBinding);
  if (result != VK_SUCCESS) { return -1; }
  VkDescriptorPoolSize poolSize = {
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = MAX_FRAME_ON_DRAW
  };
  VkDescriptorSetLayout setLayouts[MAX_FRAME_ON_DRAW] = {
      *(VkDescriptorSetLayout *) Array_first_real(pipeInfo.descriptorSetLayouts),
      *(VkDescriptorSetLayout *) Array_first_real(pipeInfo.descriptorSetLayouts),
  };
  const VkDescriptorPool *descriptorPool = XGLVkDevice_allocDescriptorPool(logicalDevice, MAX_FRAME_ON_DRAW, 1, &poolSize);
  VkDescriptorSetAllocateInfo setAllocateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .pNext = nullptr,
      .descriptorPool = *descriptorPool, .descriptorSetCount = MAX_FRAME_ON_DRAW,
      .pSetLayouts = setLayouts,
  };
  VkDescriptorSet descriptorSets[MAX_FRAME_ON_DRAW] = {};
  vkAllocateDescriptorSets(logicalDevice->handle, &setAllocateInfo, descriptorSets);
  for (uint32_t i = 0; i < MAX_FRAME_ON_DRAW; i ++) {
    VkDescriptorBufferInfo bufferInfo = { .buffer = uboGroup->buffers[i], .offset = 0, .range = sizeof(MVP) };
    VkWriteDescriptorSet writeInfo = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .pNext = nullptr,
        .dstSet = descriptorSets[i], .dstBinding = 0, .dstArrayElement = 0,
        .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr, .pBufferInfo = &bufferInfo, .pTexelBufferView = nullptr,
    };
    vkUpdateDescriptorSets(logicalDevice->handle, 1, &writeInfo, 0, nullptr);
  }
  VkVertexInputBindingDescription vertInputBindingDescriptions[] = {
      { .binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }
  };
  VkVertexInputAttributeDescription vertInputAttrDescriptions[2] = {
      { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex, coord) },
      { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex, color) }
  };
  result = composeVertexInputs(&pipeInfo, 1, vertInputBindingDescriptions, 2, vertInputAttrDescriptions);
  if (result != VK_SUCCESS) { return -1; }
  XGLVkPipeline * pipeline = XGLVkPipeline_new(logicalDevice, &pipeInfo, surface, allocator);
  XGLVkPipelineInfo_release(&pipeInfo);

  XGLVkSwapchain * swapchain = XGLVkSwapchain_new(logicalDevice, surface, pipeline, allocator);

  MVP mvp = {};
  floatMatDiag(mvp.model, (FVec4) {1.0f, 1.0f, 1.0f, 1.0f});
  // matAffineRotate(mvp.model, (FVec4){0.0f, 0.0f, 1.0f, 0.0f}, M_PI / 3);
  matFromLookAt(mvp.view, (FAffPoint4){2.0f, 2.0f, 2.0f, 1.0f},
                (FVec4){-2.0f, -2.0f, -2.0f, 0.0f},
                (FVec4){-0.0f, -0.0f, -1.0f, 0.0f});
  // matFromOrthoProjection(mvp.proj,
  //                       (FVec2) {-2.0f, 2.0f},
  //                       (FVec2) {-2.0f, 2.0f},
  //                       (FVec2)  {-2.0f, 2.0f});
  matFromPersProjection(mvp.proj,
                        (FVec2) {-2.0f, 2.0f},
                        (FVec2) {-2.0f, 2.0f},
                        (FVec2) {2.5f, 16.0f});

  MVP * const uboMapped = XGLVkDeviceBufferGroup_mapping(uboGroup, 0, MAX_FRAME_ON_DRAW * sizeof(MVP));
  clock_t start = clock();

  XGLVkSemaphoreGroup *imgAvailableSemGroup = XGLVkSemaphoreGroup_new(logicalDevice, nullptr, MAX_FRAME_ON_DRAW, allocator);
  XGLVkSemaphoreGroup *rendFinishedSemGroup = XGLVkSemaphoreGroup_new(logicalDevice, nullptr, MAX_FRAME_ON_DRAW, allocator);
  VkFenceCreateFlags fenceFlags[MAX_FRAME_ON_DRAW] = {[0] = VK_FENCE_CREATE_SIGNALED_BIT, [1] = VK_FENCE_CREATE_SIGNALED_BIT};
  XGLVkFenceGroup *presentFiniFenGroup = XGLVkFenceGroup_new(logicalDevice, fenceFlags, MAX_FRAME_ON_DRAW, allocator);
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
                              (FVec2) {2.5f, 16.0f});
      }
      VkSemaphore signaledSemaphores[] = { rendFinishedSemGroup->semaphores[swapchain->currentIndex]};
      VkSemaphore waiteSemaphores[] = { imgAvailableSemGroup->semaphores[swapchain->currentIndex]};
      VkPipelineStageFlags waiteStageFlags[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
      VkCommandBuffer command = XGLVkCommandPool_getCommand(commandPool, swapchain->currentIndex);
      XGLVkRecordInfo renderInfo = {
          .clearValue.color.float32 = {0.15f, 0.16f, 0.18f, 1.0f},
          .submitSemaphores = signaledSemaphores, .presentSemaphores = waiteSemaphores,
          .imageIndex = 0,.submitSemCount = 1, .presentSemCount = 1, .swapchainCount = 1,
          .waitFence = presentFiniFenGroup->fences[swapchain->currentIndex],
          .waitStageFlags = waiteStageFlags
      };
      result = vkAcquireNextImageKHR(logicalDevice->handle, swapchain->handle, UINT32_MAX,
                                     imgAvailableSemGroup->semaphores[swapchain->currentIndex], VK_NULL_HANDLE,
                                     &renderInfo.imageIndex);
      if (result != VK_SUCCESS) { break; }
      vkResetFences(logicalDevice->handle, 1, &presentFiniFenGroup->fences[swapchain->currentIndex]);

      result = XGLVkCommand_startRecord(command, surface, swapchain, pipeline, &renderInfo);
      if (result != VK_SUCCESS) { break; }
      /* record draw command */ {
          XGLVkCommand_adjustDevice(command, logicalDevice);
        float angle = (float) (((double) (clock() - start)) / CLOCKS_PER_SEC);
        matFromAffineRotate(mvp.model, (FVec4) {0.0f, 0.0f, 1.0f, 0.0f}, angle);
        uboMapped[swapchain->currentIndex] = mvp;
        VkBuffer vertexBuffer = bufferGroup->buffers[0];
        VkBuffer indexBuffer = bufferGroup->buffers[1];
        VkDeviceSize offset = { 0 };
        vkCmdBindVertexBuffers(command, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(command, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout,
                                0, 1, &descriptorSets[swapchain->currentIndex],
                                0, nullptr);
        vkCmdDrawIndexed(command, lenof(cube_vert_indices), 1, 0, 0, 0);
      }
      result = XGLVkCommand_endRecord(command);
      if (result != VK_SUCCESS) { break; }

      VkSwapchainKHR swapchains[] = {swapchain->handle};
      result = XGLVkDevice_render(logicalDevice, command, swapchains, &renderInfo);
      swapchain->currentIndex++;
    }
  }

  vkDeviceWaitIdle(logicalDevice->handle);

  XGLVkDeviceBufferGroup_destroy(uboGroup);
  XGLVkDeviceBufferGroup_destroy(bufferGroup);
  XGLVkSemaphoreGroup_destroy(rendFinishedSemGroup);
  XGLVkSemaphoreGroup_destroy(imgAvailableSemGroup);
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
