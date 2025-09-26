#pragma once

#include "../renderer_interface.hpp"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <windows.h>
#include <cstdint>

// Link D3D11 and DXGI libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

// Forward declarations
class SDL3Window;

// DX11 Renderer for high-performance CEF + SDL3 integration
class DX11Renderer : public IRenderer {
public:
  // Performance optimization structures
  struct BufferUpdateStats {
    uint64_t totalUpdates;
    uint64_t cacheHits;
    uint64_t cacheMisses;
    double avgUpdateTime;
  };

  DX11Renderer();
  ~DX11Renderer();

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
  IRenderer::BufferUpdateStats GetBufferStats() const override { return {bufferStats_.totalUpdates, bufferStats_.cacheHits, bufferStats_.cacheMisses, bufferStats_.avgUpdateTime}; }

  // Getters for integration
  ID3D11Device *GetDevice() const { return device_.Get(); }
  ID3D11DeviceContext *GetContext() const { return context_.Get(); }
  IDXGISwapChain1 *GetSwapChain() const { return swapChain_.Get(); }

  // Utility methods
  bool CheckFeatureSupport() override;
  std::string GetAdapterInfo() override;
  void LogPerformanceStats() override;

  // Renderer identification
  RendererType GetRendererType() const override { return RendererType::DirectX11; }
  std::string GetRendererName() const override { return "DirectX 11"; }

private:
  // Core D3D11 objects
  ComPtr<ID3D11Device> device_;
  ComPtr<ID3D11DeviceContext> context_;
  ComPtr<IDXGISwapChain1> swapChain_;
  ComPtr<IDXGIFactory2> factory_;

  // Render targets and views
  ComPtr<ID3D11Texture2D> backBuffer_;
  ComPtr<ID3D11RenderTargetView> renderTargetView_;
  ComPtr<ID3D11Texture2D> depthStencilBuffer_;
  ComPtr<ID3D11DepthStencilView> depthStencilView_;

  // CEF texture resources
  ComPtr<ID3D11Texture2D> cefTexture_;
  ComPtr<ID3D11ShaderResourceView> cefTextureView_;
  ComPtr<ID3D11SamplerState> samplerState_;

  // Shaders for texture rendering
  ComPtr<ID3D11VertexShader> vertexShader_;
  ComPtr<ID3D11PixelShader> pixelShader_;
  ComPtr<ID3D11InputLayout> inputLayout_;
  ComPtr<ID3D11Buffer> vertexBuffer_;
  ComPtr<ID3D11Buffer> indexBuffer_;

  // Render states
  ComPtr<ID3D11BlendState> blendState_;
  ComPtr<ID3D11RasterizerState> rasterizerState_;
  ComPtr<ID3D11DepthStencilState> depthStencilState_;

  // Configuration
  HWND hwnd_;
  int width_;
  int height_;
  bool initialized_;
  bool vsyncEnabled_;
  int multiSampleCount_;
  DXGI_FORMAT textureFormat_;
  bool using_high_performance_adapter_;

  // Performance tracking
  mutable LARGE_INTEGER lastFrameTime_;
  mutable double frameTime_;
  mutable int frameCount_;

  // Performance optimizations - texture caching
  struct TextureCache {
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> view;
    int width;
    int height;
    DXGI_FORMAT format;
    UINT64 lastUsed;
    bool isDirty;
  };

  // Custom hash function for std::pair<int, int>
  struct PairHash {
      std::size_t operator()(const std::pair<int, int>& p) const {
          return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
      }
  };

  std::unordered_map<std::pair<int, int>, TextureCache, PairHash> textureCache_;
  static constexpr size_t MAX_CACHED_TEXTURES = 16;
  static constexpr UINT64 CACHE_TIMEOUT_MS = 5000; // 5 seconds

  // Buffer update optimizations
  BufferUpdateStats bufferStats_;

  // Dirty region tracking for partial updates
  struct DirtyRegion {
    int x, y, width, height;
    bool isEmpty() const { return width <= 0 || height <= 0; }
  };
  DirtyRegion lastDirtyRegion_;
  bool enablePartialUpdates_;

  // Private helper methods
  bool CreateDeviceInternal(bool preferHighPerformance);
  bool CreateDevice();
  bool CreateSwapChain();
  bool CreateRenderTargets();
  bool CreateShaders();
  bool CreateGeometry();
  bool CreateStates();
  bool CreateCEFTexture(int width, int height);

  // Add this helper for shared texture creation
  bool CreateSharedTexture(int width, int height, Microsoft::WRL::ComPtr<ID3D11Texture2D>& sharedTexture);

  void CleanupRenderTargets();
  void CleanupCEFTexture();

  // Shader compilation helpers
  bool CompileShaderFromString(const std::string &shaderSource,
                               const std::string &entryPoint,
                               const std::string &target, ID3DBlob **blob);

  // Error handling
  void LogError(const std::string &message, HRESULT hr = S_OK);
  std::string HResultToString(HRESULT hr);
};

// Constants for shader compilation
namespace DX11Shaders {
extern const char *VERTEX_SHADER_SOURCE;
extern const char *PIXEL_SHADER_SOURCE;
} // namespace DX11Shaders
