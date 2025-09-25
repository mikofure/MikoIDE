#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <windows.h>
#include <wrl/client.h>

// Link D3D11 and DXGI libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

// Forward declarations
class SDL3Window;

// DX11 Renderer for high-performance CEF + SDL3 integration
class DX11Renderer {
public:
  // Performance optimization structures
  struct BufferUpdateStats {
    UINT64 totalUpdates;
    UINT64 cacheHits;
    UINT64 cacheMisses;
    double avgUpdateTime;
  };

  DX11Renderer();
  ~DX11Renderer();

  // Core functionality
  bool Initialize(HWND hwnd, int width, int height);
  void Shutdown();
  bool IsInitialized() const { return initialized_; }

  // Rendering operations
  bool BeginFrame();
  bool EndFrame();
  bool Present();
  bool Render();

  // Texture management for CEF OSR integration
  bool CreateTextureFromBuffer(const void *buffer, int width, int height);
  bool UpdateTexture(const void *buffer, int width, int height);
  bool ResizeTextures(int width, int height);

  // Performance optimizations
  void EnableVSync(bool enable);
  void SetMultiSampleCount(int samples);
  void EnablePartialUpdates(bool enable);
  void ClearTextureCache();
  void SetDirtyRegion(int x, int y, int width, int height);
  BufferUpdateStats GetBufferStats() const { return bufferStats_; }

  // Getters for integration
  ID3D11Device *GetDevice() const { return device_.Get(); }
  ID3D11DeviceContext *GetContext() const { return context_.Get(); }
  IDXGISwapChain1 *GetSwapChain() const { return swapChain_.Get(); }

  // Utility methods
  bool CheckFeatureSupport();
  std::string GetAdapterInfo();
  void LogPerformanceStats();

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

  std::unordered_map<std::string, TextureCache> textureCache_;
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
  bool CreateDevice();
  bool CreateSwapChain();
  bool CreateRenderTargets();
  bool CreateShaders();
  bool CreateGeometry();
  bool CreateStates();
  bool CreateCEFTexture(int width, int height);

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

// Vertex structure for fullscreen quad rendering
struct QuadVertex {
  float position[3]; // x, y, z
  float texCoord[2]; // u, v
};

// Constants for shader compilation
namespace DX11Shaders {
extern const char *VERTEX_SHADER_SOURCE;
extern const char *PIXEL_SHADER_SOURCE;
} // namespace DX11Shaders