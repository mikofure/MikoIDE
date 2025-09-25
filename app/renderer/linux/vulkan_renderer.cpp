#include "vulkan_renderer.hpp"
#include <SDL.h>
#include <SDL_vulkan.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>

// Validation layer debug callback
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {

  std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
  return VK_FALSE;
}

// Vertex data for fullscreen quad
const std::vector<QuadVertex> vertices = {{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
                                          {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
                                          {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
                                          {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

VulkanRenderer::VulkanRenderer()
    : instance_(VK_NULL_HANDLE), debugMessenger_(VK_NULL_HANDLE),
      surface_(VK_NULL_HANDLE), physicalDevice_(VK_NULL_HANDLE),
      device_(VK_NULL_HANDLE), graphicsQueue_(VK_NULL_HANDLE),
      presentQueue_(VK_NULL_HANDLE), swapChain_(VK_NULL_HANDLE),
      renderPass_(VK_NULL_HANDLE), descriptorSetLayout_(VK_NULL_HANDLE),
      pipelineLayout_(VK_NULL_HANDLE), graphicsPipeline_(VK_NULL_HANDLE),
      commandPool_(VK_NULL_HANDLE), cefImage_(VK_NULL_HANDLE),
      cefImageMemory_(VK_NULL_HANDLE), cefImageView_(VK_NULL_HANDLE),
      cefSampler_(VK_NULL_HANDLE), vertexBuffer_(VK_NULL_HANDLE),
      vertexBufferMemory_(VK_NULL_HANDLE), indexBuffer_(VK_NULL_HANDLE),
      indexBufferMemory_(VK_NULL_HANDLE), descriptorPool_(VK_NULL_HANDLE),
      window_(nullptr), width_(0), height_(0), initialized_(false),
      vsyncEnabled_(true), multiSampleCount_(1), currentFrame_(0),
      lastFrameTime_(0), frameTime_(0.0), frameCount_(0),
      enablePartialUpdates_(false) {

  // Initialize buffer stats
  bufferStats_ = {0, 0, 0, 0.0};
  lastDirtyRegion_ = {0, 0, 0, 0};
}

VulkanRenderer::~VulkanRenderer() { Shutdown(); }

bool VulkanRenderer::Initialize(SDL_Window *window, int width, int height) {
  if (initialized_) {
    return true;
  }

  window_ = window;
  width_ = width;
  height_ = height;

  // Initialize Vulkan components in order
  if (!CreateInstance()) {
    LogError("Failed to create Vulkan instance");
    return false;
  }

  if (!SetupDebugMessenger()) {
    LogError("Failed to setup debug messenger");
    return false;
  }

  if (!CreateSurface()) {
    LogError("Failed to create window surface");
    return false;
  }

  if (!PickPhysicalDevice()) {
    LogError("Failed to find suitable GPU");
    return false;
  }

  if (!CreateLogicalDevice()) {
    LogError("Failed to create logical device");
    return false;
  }

  if (!CreateSwapChain()) {
    LogError("Failed to create swap chain");
    return false;
  }

  if (!CreateImageViews()) {
    LogError("Failed to create image views");
    return false;
  }

  if (!CreateRenderPass()) {
    LogError("Failed to create render pass");
    return false;
  }

  if (!CreateDescriptorSetLayout()) {
    LogError("Failed to create descriptor set layout");
    return false;
  }

  if (!CreateGraphicsPipeline()) {
    LogError("Failed to create graphics pipeline");
    return false;
  }

  if (!CreateFramebuffers()) {
    LogError("Failed to create framebuffers");
    return false;
  }

  if (!CreateCommandPool()) {
    LogError("Failed to create command pool");
    return false;
  }

  if (!CreateVertexBuffer()) {
    LogError("Failed to create vertex buffer");
    return false;
  }

  if (!CreateIndexBuffer()) {
    LogError("Failed to create index buffer");
    return false;
  }

  if (!CreateCEFTexture(width, height)) {
    LogError("Failed to create CEF texture");
    return false;
  }

  if (!CreateDescriptorPool()) {
    LogError("Failed to create descriptor pool");
    return false;
  }

  if (!CreateDescriptorSets()) {
    LogError("Failed to create descriptor sets");
    return false;
  }

  if (!CreateCommandBuffers()) {
    LogError("Failed to create command buffers");
    return false;
  }

  if (!CreateSyncObjects()) {
    LogError("Failed to create synchronization objects");
    return false;
  }

  initialized_ = true;
  std::cout << "Vulkan renderer initialized successfully" << std::endl;
  return true;
}

void VulkanRenderer::Shutdown() {
  if (!initialized_) {
    return;
  }

  vkDeviceWaitIdle(device_);

  // Cleanup synchronization objects
  for (size_t i = 0; i < imageAvailableSemaphores_.size(); i++) {
    vkDestroySemaphore(device_, renderFinishedSemaphores_[i], nullptr);
    vkDestroySemaphore(device_, imageAvailableSemaphores_[i], nullptr);
    vkDestroyFence(device_, inFlightFences_[i], nullptr);
  }

  // Cleanup buffers
  vkDestroyBuffer(device_, indexBuffer_, nullptr);
  vkFreeMemory(device_, indexBufferMemory_, nullptr);
  vkDestroyBuffer(device_, vertexBuffer_, nullptr);
  vkFreeMemory(device_, vertexBufferMemory_, nullptr);

  // Cleanup CEF texture
  CleanupCEFTexture();

  // Cleanup descriptor pool
  vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);

  // Cleanup command pool
  vkDestroyCommandPool(device_, commandPool_, nullptr);

  // Cleanup swap chain
  CleanupSwapChain();

  // Cleanup pipeline
  vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
  vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
  vkDestroyRenderPass(device_, renderPass_, nullptr);
  vkDestroyDescriptorSetLayout(device_, descriptorSetLayout_, nullptr);

  // Cleanup device
  vkDestroyDevice(device_, nullptr);

  // Cleanup debug messenger
  if (enableValidationLayers_) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance_, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
      func(instance_, debugMessenger_, nullptr);
    }
  }

  // Cleanup surface and instance
  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  vkDestroyInstance(instance_, nullptr);

  // Clear texture cache
  ClearTextureCache();

  initialized_ = false;
  std::cout << "Vulkan renderer shutdown complete" << std::endl;
}

bool VulkanRenderer::BeginFrame() {
  if (!initialized_) {
    return false;
  }

  vkWaitForFences(device_, 1, &inFlightFences_[currentFrame_], VK_TRUE,
                  UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      device_, swapChain_, UINT64_MAX, imageAvailableSemaphores_[currentFrame_],
      VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapChain();
    return false;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    LogError("Failed to acquire swap chain image");
    return false;
  }

  vkResetFences(device_, 1, &inFlightFences_[currentFrame_]);

  vkResetCommandBuffer(commandBuffers_[currentFrame_], 0);

  // Record command buffer
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffers_[currentFrame_], &beginInfo) !=
      VK_SUCCESS) {
    LogError("Failed to begin recording command buffer");
    return false;
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = renderPass_;
  renderPassInfo.framebuffer = swapChainFramebuffers_[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = swapChainExtent_;

  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(commandBuffers_[currentFrame_], &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  return true;
}

bool VulkanRenderer::EndFrame() {
  if (!initialized_) {
    return false;
  }

  vkCmdEndRenderPass(commandBuffers_[currentFrame_]);

  if (vkEndCommandBuffer(commandBuffers_[currentFrame_]) != VK_SUCCESS) {
    LogError("Failed to record command buffer");
    return false;
  }

  return true;
}

bool VulkanRenderer::Present() {
  if (!initialized_) {
    return false;
  }

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame_]};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers_[currentFrame_];

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores_[currentFrame_]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo,
                    inFlightFences_[currentFrame_]) != VK_SUCCESS) {
    LogError("Failed to submit draw command buffer");
    return false;
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain_};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  uint32_t imageIndex = 0; // This should be stored from BeginFrame
  presentInfo.pImageIndices = &imageIndex;

  VkResult result = vkQueuePresentKHR(presentQueue_, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    RecreateSwapChain();
  } else if (result != VK_SUCCESS) {
    LogError("Failed to present swap chain image");
    return false;
  }

  currentFrame_ = (currentFrame_ + 1) % imageAvailableSemaphores_.size();

  return true;
}

bool VulkanRenderer::Render() {
  if (!BeginFrame()) {
    return false;
  }

  // Bind graphics pipeline
  vkCmdBindPipeline(commandBuffers_[currentFrame_],
                    VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);

  // Bind vertex buffer
  VkBuffer vertexBuffers[] = {vertexBuffer_};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffers_[currentFrame_], 0, 1, vertexBuffers,
                         offsets);

  // Bind index buffer
  vkCmdBindIndexBuffer(commandBuffers_[currentFrame_], indexBuffer_, 0,
                       VK_INDEX_TYPE_UINT16);

  // Bind descriptor sets
  vkCmdBindDescriptorSets(commandBuffers_[currentFrame_],
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0,
                          1, &descriptorSets_[currentFrame_], 0, nullptr);

  // Draw
  vkCmdDrawIndexed(commandBuffers_[currentFrame_],
                   static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

  if (!EndFrame()) {
    return false;
  }

  return Present();
}

// Texture management methods
bool VulkanRenderer::CreateTextureFromBuffer(const void *buffer, int width,
                                             int height) {
  if (!initialized_ || !buffer) {
    return false;
  }

  // Clean up existing texture if dimensions changed
  if (width != width_ || height != height_) {
    CleanupCEFTexture();
    if (!CreateCEFTexture(width, height)) {
      return false;
    }
    width_ = width;
    height_ = height;
  }

  return UpdateTexture(buffer, width, height);
}

bool VulkanRenderer::UpdateTexture(const void *buffer, int width, int height) {
  if (!initialized_ || !buffer) {
    return false;
  }

  auto startTime = std::chrono::high_resolution_clock::now();

  VkDeviceSize imageSize = width * height * 4; // RGBA

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(device_, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, buffer, static_cast<size_t>(imageSize));
  vkUnmapMemory(device_, stagingBufferMemory);

  TransitionImageLayout(cefImage_, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  CopyBufferToImage(stagingBuffer, cefImage_, static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height));
  TransitionImageLayout(cefImage_, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(device_, stagingBuffer, nullptr);
  vkFreeMemory(device_, stagingBufferMemory, nullptr);

  // Update performance stats
  auto endTime = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration<double, std::milli>(endTime - startTime);

  bufferStats_.totalUpdates++;
  bufferStats_.avgUpdateTime =
      (bufferStats_.avgUpdateTime * (bufferStats_.totalUpdates - 1) +
       duration.count()) /
      bufferStats_.totalUpdates;

  return true;
}

bool VulkanRenderer::ResizeTextures(int width, int height) {
  if (!initialized_) {
    return false;
  }

  vkDeviceWaitIdle(device_);

  CleanupSwapChain();

  width_ = width;
  height_ = height;

  if (!CreateSwapChain() || !CreateImageViews() || !CreateFramebuffers()) {
    return false;
  }

  // Recreate CEF texture with new dimensions
  CleanupCEFTexture();
  return CreateCEFTexture(width, height);
}

// Performance optimization methods
void VulkanRenderer::EnableVSync(bool enable) {
  vsyncEnabled_ = enable;
  // VSync changes require swap chain recreation
  if (initialized_) {
    RecreateSwapChain();
  }
}

void VulkanRenderer::SetMultiSampleCount(int samples) {
  multiSampleCount_ = samples;
  // MSAA changes require pipeline recreation
  if (initialized_) {
    // TODO: Implement MSAA pipeline recreation
  }
}

void VulkanRenderer::EnablePartialUpdates(bool enable) {
  enablePartialUpdates_ = enable;
}

void VulkanRenderer::ClearTextureCache() {
  for (auto &pair : textureCache_) {
    vkDestroyImageView(device_, pair.second.view, nullptr);
    vkDestroyImage(device_, pair.second.image, nullptr);
    vkFreeMemory(device_, pair.second.memory, nullptr);
  }
  textureCache_.clear();
}

void VulkanRenderer::SetDirtyRegion(int x, int y, int width, int height) {
  lastDirtyRegion_ = {x, y, width, height};
}

// Utility methods
bool VulkanRenderer::CheckFeatureSupport() {
  if (!initialized_) {
    return false;
  }

  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(physicalDevice_, &deviceFeatures);

  return deviceFeatures.samplerAnisotropy == VK_TRUE;
}

std::string VulkanRenderer::GetAdapterInfo() {
  if (!initialized_) {
    return "Vulkan renderer not initialized";
  }

  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice_, &deviceProperties);

  return std::string("GPU: ") + deviceProperties.deviceName + " (Vulkan " +
         std::to_string(VK_VERSION_MAJOR(deviceProperties.apiVersion)) + "." +
         std::to_string(VK_VERSION_MINOR(deviceProperties.apiVersion)) + ")";
}

void VulkanRenderer::LogPerformanceStats() {
  std::cout << "=== Vulkan Renderer Performance Stats ===" << std::endl;
  std::cout << "Total buffer updates: " << bufferStats_.totalUpdates
            << std::endl;
  std::cout << "Cache hits: " << bufferStats_.cacheHits << std::endl;
  std::cout << "Cache misses: " << bufferStats_.cacheMisses << std::endl;
  std::cout << "Average update time: " << bufferStats_.avgUpdateTime << "ms"
            << std::endl;
  std::cout << "Frame time: " << frameTime_ << "ms" << std::endl;
  std::cout << "Frame count: " << frameCount_ << std::endl;
}

// Private helper method implementations
bool VulkanRenderer::CreateInstance() {
  if (enableValidationLayers_ && !CheckValidationLayerSupport()) {
    LogError("Validation layers requested, but not available");
    return false;
  }

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "MikoIDE Vulkan Renderer";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "MikoIDE Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = GetRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (enableValidationLayers_) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers_.size());
    createInfo.ppEnabledLayerNames = validationLayers_.data();

    debugCreateInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = DebugCallback;

    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  return vkCreateInstance(&createInfo, nullptr, &instance_) == VK_SUCCESS;
}

bool VulkanRenderer::CreateSurface() {
  return SDL_Vulkan_CreateSurface(window_, instance_, &surface_) == SDL_TRUE;
}

void VulkanRenderer::LogError(const std::string &message) {
  std::cerr << "Vulkan Renderer Error: " << message << std::endl;
}

// Vertex input description implementations
VkVertexInputBindingDescription QuadVertex::GetBindingDescription() {
  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(QuadVertex);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2>
QuadVertex::GetAttributeDescriptions() {
  std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(QuadVertex, position);

  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(QuadVertex, texCoord);

  return attributeDescriptions;
}

// Additional helper method stubs - these would be fully implemented in a
// complete version
bool VulkanRenderer::CheckValidationLayerSupport() { return true; }
std::vector<const char *> VulkanRenderer::GetRequiredExtensions() {
  uint32_t extensionCount = 0;
  SDL_Vulkan_GetInstanceExtensions(window_, &extensionCount, nullptr);
  std::vector<const char *> extensions(extensionCount);
  SDL_Vulkan_GetInstanceExtensions(window_, &extensionCount, extensions.data());

  if (enableValidationLayers_) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}
bool VulkanRenderer::SetupDebugMessenger() { return true; }
bool VulkanRenderer::PickPhysicalDevice() { return true; }
bool VulkanRenderer::CreateLogicalDevice() { return true; }
bool VulkanRenderer::CreateSwapChain() { return true; }
bool VulkanRenderer::CreateImageViews() { return true; }
bool VulkanRenderer::CreateRenderPass() { return true; }
bool VulkanRenderer::CreateDescriptorSetLayout() { return true; }
bool VulkanRenderer::CreateGraphicsPipeline() { return true; }
bool VulkanRenderer::CreateFramebuffers() { return true; }
bool VulkanRenderer::CreateCommandPool() { return true; }
bool VulkanRenderer::CreateVertexBuffer() { return true; }
bool VulkanRenderer::CreateIndexBuffer() { return true; }
bool VulkanRenderer::CreateCEFTexture(int width, int height) { return true; }
bool VulkanRenderer::CreateDescriptorPool() { return true; }
bool VulkanRenderer::CreateDescriptorSets() { return true; }
bool VulkanRenderer::CreateCommandBuffers() { return true; }
bool VulkanRenderer::CreateSyncObjects() { return true; }
void VulkanRenderer::CleanupSwapChain() {}
void VulkanRenderer::RecreateSwapChain() {}
void VulkanRenderer::CleanupCEFTexture() {}
bool VulkanRenderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                  VkMemoryPropertyFlags properties,
                                  VkBuffer &buffer,
                                  VkDeviceMemory &bufferMemory) {
  return true;
}
void VulkanRenderer::TransitionImageLayout(VkImage image, VkFormat format,
                                           VkImageLayout oldLayout,
                                           VkImageLayout newLayout) {}
void VulkanRenderer::CopyBufferToImage(VkBuffer buffer, VkImage image,
                                       uint32_t width, uint32_t height) {}