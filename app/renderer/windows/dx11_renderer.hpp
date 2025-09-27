#pragma once

#include "../renderer_interface.hpp"
#include <string>

#ifdef _WIN32
#include <cstdint>
#include <chrono>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxgi1_6.h>
#include <memory>
#include <thread>
#include <unordered_map>
#include <windows.h>
#include <wrl/client.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

class SDL3Window;

class DX11Renderer : public IRenderer {
public:
  struct BufferUpdateStats {
    uint64_t totalUpdates;
    uint64_t cacheHits;
    uint64_t cacheMisses;
    double avgUpdateTime;
  };

  DX11Renderer();
  ~DX11Renderer();

  bool Initialize(void *window_handle, int width, int height) override;
  void Shutdown() override;
  bool IsInitialized() const override { return initialized_; }

  bool BeginFrame() override;
  bool EndFrame() override;
  bool Present() override;
  bool Render() override;

  bool CreateTextureFromBuffer(const void *buffer, int width, int height) override;
  bool UpdateTexture(const void *buffer, int width, int height) override;
  bool ResizeTextures(int width, int height) override;

  void EnableVSync(bool enable) override;
  void SetMultiSampleCount(int samples) override;
  void EnablePartialUpdates(bool enable) override;
  void ClearTextureCache() override;
  void SetDirtyRegion(int x, int y, int width, int height) override;
  IRenderer::BufferUpdateStats GetBufferStats() const override {
    return {bufferStats_.totalUpdates, bufferStats_.cacheHits,
            bufferStats_.cacheMisses, bufferStats_.avgUpdateTime};
  }

  ID3D11Device *GetDevice() const { return device_.Get(); }
  ID3D11DeviceContext *GetContext() const { return context_.Get(); }
  IDXGISwapChain1 *GetSwapChain() const { return swapChain_.Get(); }

  bool CheckFeatureSupport() override;
  std::string GetAdapterInfo() override;
  void LogPerformanceStats() override;

  RendererType GetRendererType() const override { return RendererType::DirectX11; }
  std::string GetRendererName() const override { return "DirectX 11"; }

private:
  ComPtr<ID3D11Device> device_;
  ComPtr<ID3D11DeviceContext> context_;
  ComPtr<IDXGISwapChain1> swapChain_;
  ComPtr<IDXGIFactory2> factory_;

  ComPtr<ID3D11Texture2D> backBuffer_;
  ComPtr<ID3D11RenderTargetView> renderTargetView_;
  ComPtr<ID3D11Texture2D> depthStencilBuffer_;
  ComPtr<ID3D11DepthStencilView> depthStencilView_;

  ComPtr<ID3D11Texture2D> cefTexture_;
  ComPtr<ID3D11ShaderResourceView> cefTextureView_;
  ComPtr<ID3D11SamplerState> samplerState_;

  ComPtr<ID3D11VertexShader> vertexShader_;
  ComPtr<ID3D11PixelShader> pixelShader_;
  ComPtr<ID3D11InputLayout> inputLayout_;
  ComPtr<ID3D11Buffer> vertexBuffer_;
  ComPtr<ID3D11Buffer> indexBuffer_;

  ComPtr<ID3D11BlendState> blendState_;
  ComPtr<ID3D11RasterizerState> rasterizerState_;
  ComPtr<ID3D11DepthStencilState> depthStencilState_;

  HWND hwnd_;
  int width_;
  int height_;
  bool initialized_;
  bool vsyncEnabled_;
  int multiSampleCount_;
  DXGI_FORMAT textureFormat_;
  bool using_high_performance_adapter_;

  mutable LARGE_INTEGER lastFrameTime_;
  mutable double frameTime_;
  mutable int frameCount_;

  struct TextureCache {
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> view;
    int width;
    int height;
    DXGI_FORMAT format;
    UINT64 lastUsed;
    bool isDirty;
  };

  struct PairHash {
    std::size_t operator()(const std::pair<int, int> &p) const {
      return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
  };

  std::unordered_map<std::pair<int, int>, TextureCache, PairHash> textureCache_;
  static constexpr size_t MAX_CACHED_TEXTURES = 16;
  static constexpr UINT64 CACHE_TIMEOUT_MS = 5000;

  BufferUpdateStats bufferStats_;

  struct DirtyRegion {
    int x, y, width, height;
    bool isEmpty() const { return width <= 0 || height <= 0; }
  };
  DirtyRegion lastDirtyRegion_;
  bool enablePartialUpdates_;

  bool CreateDeviceInternal(bool preferHighPerformance);
  bool CreateDevice();
  bool CreateSwapChain();
  bool CreateRenderTargets();
  bool CreateShaders();
  bool CreateGeometry();
  bool CreateStates();
  bool CreateCEFTexture(int width, int height);
  bool CreateSharedTexture(int width, int height,
                           Microsoft::WRL::ComPtr<ID3D11Texture2D> &sharedTexture);

  void CleanupRenderTargets();
  void CleanupCEFTexture();

  bool CompileShaderFromString(const std::string &shaderSource,
                               const std::string &entryPoint,
                               const std::string &target, ID3DBlob **blob);

  void LogError(const std::string &message, HRESULT hr = S_OK);
  std::string HResultToString(HRESULT hr);
};

namespace DX11Shaders {
extern const char *VERTEX_SHADER_SOURCE;
extern const char *PIXEL_SHADER_SOURCE;
} // namespace DX11Shaders

#else

class DX11Renderer : public IRenderer {
public:
  DX11Renderer() = default;
  ~DX11Renderer() override = default;

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
  std::string GetAdapterInfo() override {
    return "DirectX 11 renderer not available on this platform";
  }
  void LogPerformanceStats() override {}

  RendererType GetRendererType() const override { return RendererType::DirectX11; }
  std::string GetRendererName() const override { return "DirectX 11"; }
};

#endif
