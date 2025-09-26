#pragma once

#include <memory>
#include <string>

// Forward declarations
struct SDL_Window;

// Abstract renderer interface for cross-platform support
class IRenderer {
public:
    // Performance optimization structures
    struct BufferUpdateStats {
        uint64_t totalUpdates;
        uint64_t cacheHits;
        uint64_t cacheMisses;
        double avgUpdateTime;
    };

    virtual ~IRenderer() = default;

    // Core functionality
    virtual bool Initialize(void* window_handle, int width, int height) = 0;
    virtual void Shutdown() = 0;
    virtual bool IsInitialized() const = 0;

    // Rendering operations
    virtual bool BeginFrame() = 0;
    virtual bool EndFrame() = 0;
    virtual bool Present() = 0;
    virtual bool Render() = 0;

    // Texture management for CEF OSR integration
    virtual bool CreateTextureFromBuffer(const void* buffer, int width, int height) = 0;
    virtual bool UpdateTexture(const void* buffer, int width, int height) = 0;
    virtual bool ResizeTextures(int width, int height) = 0;

    // Performance optimizations
    virtual void EnableVSync(bool enable) = 0;
    virtual void SetMultiSampleCount(int samples) = 0;
    virtual void EnablePartialUpdates(bool enable) = 0;
    virtual void ClearTextureCache() = 0;
    virtual void SetDirtyRegion(int x, int y, int width, int height) = 0;
    virtual BufferUpdateStats GetBufferStats() const = 0;

    // Utility methods
    virtual bool CheckFeatureSupport() = 0;
    virtual std::string GetAdapterInfo() = 0;
    virtual void LogPerformanceStats() = 0;

    // Renderer type identification
    enum class RendererType {
        DirectX11,
        OpenGL,
        Vulkan
    };

    virtual RendererType GetRendererType() const = 0;
    virtual std::string GetRendererName() const = 0;
};

// Vertex structure for fullscreen quad rendering (common across all renderers)
struct QuadVertex {
    float position[3]; // x, y, z
    float texCoord[2]; // u, v
};