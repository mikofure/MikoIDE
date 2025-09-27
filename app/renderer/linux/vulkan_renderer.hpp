#pragma once

#include "../renderer_interface.hpp"

class VulkanRenderer : public IRenderer {
public:
  VulkanRenderer() = default;
  ~VulkanRenderer() override = default;

  bool Initialize(void *, int, int) override { return false; }
  void Shutdown() override {}
  bool IsInitialized() const override { return false; }

  bool BeginFrame() override { return false; }
  bool EndFrame() override { return false; }
  bool Present() override { return false; }
  bool Render() override { return false; }

  bool CreateTextureFromBuffer(const void *, int, int) override { return false; }
  bool UpdateTexture(const void *, int, int) override { return false; }
  bool ResizeTextures(int, int) override { return false; }

  void EnableVSync(bool) override {}
  void SetMultiSampleCount(int) override {}
  void EnablePartialUpdates(bool) override {}
  void ClearTextureCache() override {}
  void SetDirtyRegion(int, int, int, int) override {}

  BufferUpdateStats GetBufferStats() const override { return {}; }

  bool CheckFeatureSupport() override { return false; }
  std::string GetAdapterInfo() override { return "Vulkan renderer unavailable"; }
  void LogPerformanceStats() override {}

  RendererType GetRendererType() const override { return RendererType::Vulkan; }
  std::string GetRendererName() const override { return "Vulkan"; }
};
