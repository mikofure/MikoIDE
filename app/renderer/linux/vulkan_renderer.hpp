#pragma once

#include "../renderer_interface.hpp"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

// Forward declarations
struct SDL_Window;

// Vulkan Renderer for high-performance CEF + SDL3 integration on Linux
class VulkanRenderer : public IRenderer {
public:
  // Performance optimization structures
  struct BufferUpdateStats {
    uint64_t totalUpdates;
    uint64_t cacheHits;
    uint64_t cacheMisses;
    double avgUpdateTime;
  };

  // Queue family indices
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  // Swap chain support details
  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  VulkanRenderer();
  ~VulkanRenderer();

  // Core functionality
  bool Initialize(void* window_handle, int width, int height) override;
  void Shutdown() override;
  bool IsInitialized() const override { return initialized_; }

  // Rendering operations
  bool BeginFrame() override;
  bool EndFrame() override;
  bool Present() override;
  bool Render() override;

  // Texture management for CEF OSR integration
  bool CreateTextureFromBuffer(const void *buffer, int width, int height) override;
  bool UpdateTexture(const void *buffer, int width, int height) override;
  bool ResizeTextures(int width, int height) override;

  // Performance optimizations
  void EnableVSync(bool enable) override;
  void SetMultiSampleCount(int samples) override;
  void EnablePartialUpdates(bool enable) override;
  void ClearTextureCache() override;
  void SetDirtyRegion(int x, int y, int width, int height) override;
  BufferUpdateStats GetBufferStats() const override { return bufferStats_; }

  // Renderer identification
  RendererType GetRendererType() const override { return RendererType::Vulkan; }
  std::string GetRendererName() const override { return "Vulkan"; }

  // Utility methods
  bool CheckFeatureSupport() override;
  std::string GetAdapterInfo() override;
  void LogPerformanceStats() override;

  // Getters for Vulkan-specific integration
  VkDevice GetDevice() const { return device_; }
  VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice_; }
  VkCommandPool GetCommandPool() const { return commandPool_; }
  VkQueue GetGraphicsQueue() const { return graphicsQueue_; }

private:
  // Core Vulkan objects
  VkInstance instance_;
  VkDebugUtilsMessengerEXT debugMessenger_;
  VkSurfaceKHR surface_;
  VkPhysicalDevice physicalDevice_;
  VkDevice device_;
  VkQueue graphicsQueue_;
  VkQueue presentQueue_;

  // Swap chain
  VkSwapchainKHR swapChain_;
  std::vector<VkImage> swapChainImages_;
  VkFormat swapChainImageFormat_;
  VkExtent2D swapChainExtent_;
  std::vector<VkImageView> swapChainImageViews_;
  std::vector<VkFramebuffer> swapChainFramebuffers_;

  // Render pass and pipeline
  VkRenderPass renderPass_;
  VkDescriptorSetLayout descriptorSetLayout_;
  VkPipelineLayout pipelineLayout_;
  VkPipeline graphicsPipeline_;

  // Command buffers
  VkCommandPool commandPool_;
  std::vector<VkCommandBuffer> commandBuffers_;

  // Synchronization objects
  std::vector<VkSemaphore> imageAvailableSemaphores_;
  std::vector<VkSemaphore> renderFinishedSemaphores_;
  std::vector<VkFence> inFlightFences_;

  // CEF texture resources
  VkImage cefImage_;
  VkDeviceMemory cefImageMemory_;
  VkImageView cefImageView_;
  VkSampler cefSampler_;

  // Vertex buffer
  VkBuffer vertexBuffer_;
  VkDeviceMemory vertexBufferMemory_;
  VkBuffer indexBuffer_;
  VkDeviceMemory indexBufferMemory_;

  // Descriptor pool and sets
  VkDescriptorPool descriptorPool_;
  std::vector<VkDescriptorSet> descriptorSets_;

  // Configuration
  SDL_Window *window_;
  int width_;
  int height_;
  bool initialized_;
  bool vsyncEnabled_;
  int multiSampleCount_;
  uint32_t currentFrame_;

  // Performance tracking
  mutable uint64_t lastFrameTime_;
  mutable double frameTime_;
  mutable int frameCount_;

  // Performance optimizations - texture caching
  struct TextureCache {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
    int width;
    int height;
    VkFormat format;
    uint64_t lastUsed;
    bool isDirty;
  };

  std::unordered_map<std::string, TextureCache> textureCache_;
  static constexpr size_t MAX_CACHED_TEXTURES = 16;
  static constexpr uint64_t CACHE_TIMEOUT_MS = 5000; // 5 seconds

  // Buffer update optimizations
  BufferUpdateStats bufferStats_;

  // Dirty region tracking for partial updates
  struct DirtyRegion {
    int x, y, width, height;
    bool isEmpty() const { return width <= 0 || height <= 0; }
  };
  DirtyRegion lastDirtyRegion_;
  bool enablePartialUpdates_;

  // Validation layers
  const std::vector<const char *> validationLayers_ = {
      "VK_LAYER_KHRONOS_validation"};

  const std::vector<const char *> deviceExtensions_ = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
  const bool enableValidationLayers_ = false;
#else
  const bool enableValidationLayers_ = true;
#endif

  // Private helper methods
  bool CreateInstance();
  bool SetupDebugMessenger();
  bool CreateSurface();
  bool PickPhysicalDevice();
  bool CreateLogicalDevice();
  bool CreateSwapChain();
  bool CreateImageViews();
  bool CreateRenderPass();
  bool CreateDescriptorSetLayout();
  bool CreateGraphicsPipeline();
  bool CreateFramebuffers();
  bool CreateCommandPool();
  bool CreateVertexBuffer();
  bool CreateIndexBuffer();
  bool CreateCEFTexture(int width, int height);
  bool CreateDescriptorPool();
  bool CreateDescriptorSets();
  bool CreateCommandBuffers();
  bool CreateSyncObjects();

  void CleanupSwapChain();
  void RecreateSwapChain();
  void CleanupCEFTexture();

  // Validation and device selection helpers
  bool CheckValidationLayerSupport();
  std::vector<const char *> GetRequiredExtensions();
  bool IsDeviceSuitable(VkPhysicalDevice device);
  bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
  SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

  // Swap chain helpers
  VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR ChooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  // Shader compilation helpers
  VkShaderModule CreateShaderModule(const std::vector<char> &code);
  std::vector<char> ReadFile(const std::string &filename);

  // Buffer and image helpers
  bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer &buffer,
                    VkDeviceMemory &bufferMemory);
  bool CreateImage(uint32_t width, uint32_t height, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage &image,
                   VkDeviceMemory &imageMemory);
  VkImageView CreateImageView(VkImage image, VkFormat format);
  uint32_t FindMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
  void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void TransitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout);
  void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                         uint32_t height);

  // Command buffer helpers
  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

  // Error handling
  void LogError(const std::string &message);

  // Debug callback
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData);
};

// Vertex structure for fullscreen quad rendering
struct QuadVertex {
  float position[3]; // x, y, z
  float texCoord[2]; // u, v

  static VkVertexInputBindingDescription GetBindingDescription();
  static std::array<VkVertexInputAttributeDescription, 2>
  GetAttributeDescriptions();
};

// Constants for shader compilation
namespace VulkanShaders {
extern const std::vector<char> VERTEX_SHADER_SPV;
extern const std::vector<char> FRAGMENT_SHADER_SPV;
} // namespace VulkanShaders