#pragma once

#include "../renderer_interface.hpp"
#include <GL/gl.h>
#include <GL/glext.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declarations
struct SDL_Window;

// OpenGL Renderer for high-performance CEF + SDL3 integration on Linux
class OpenGLRenderer : public IRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer() override;

    // IRenderer interface implementation
    bool Initialize(void* window_handle, int width, int height) override;
    void Shutdown() override;
    bool IsInitialized() const override { return initialized_; }

    // Rendering operations
    bool BeginFrame() override;
    bool EndFrame() override;
    bool Present() override;
    bool Render() override;

    // Texture management for CEF OSR integration
    bool CreateTextureFromBuffer(const void* buffer, int width, int height) override;
    bool UpdateTexture(const void* buffer, int width, int height) override;
    bool ResizeTextures(int width, int height) override;

    // Performance optimizations
    void EnableVSync(bool enable) override;
    void SetMultiSampleCount(int samples) override;
    void EnablePartialUpdates(bool enable) override;
    void ClearTextureCache() override;
    void SetDirtyRegion(int x, int y, int width, int height) override;
    BufferUpdateStats GetBufferStats() const override { return bufferStats_; }

    // Utility methods
    bool CheckFeatureSupport() override;
    std::string GetAdapterInfo() override;
    void LogPerformanceStats() override;

    // Renderer identification
    RendererType GetRendererType() const override { return RendererType::OpenGL; }
    std::string GetRendererName() const override { return "OpenGL"; }

    // OpenGL-specific getters
    GLuint GetCEFTexture() const { return cefTexture_; }
    GLuint GetFramebuffer() const { return framebuffer_; }

private:
    // OpenGL objects
    GLuint cefTexture_;
    GLuint framebuffer_;
    GLuint renderbuffer_;
    GLuint vertexBuffer_;
    GLuint indexBuffer_;
    GLuint vertexArray_;
    GLuint shaderProgram_;
    GLuint vertexShader_;
    GLuint fragmentShader_;

    // OpenGL context
    void* glContext_;
    SDL_Window* window_;

    // Configuration
    int width_;
    int height_;
    bool initialized_;
    bool vsyncEnabled_;
    int multiSampleCount_;

    // Performance tracking
    mutable uint64_t lastFrameTime_;
    mutable double frameTime_;
    mutable int frameCount_;

    // Performance optimizations - texture caching
    struct TextureCache {
        GLuint texture;
        int width;
        int height;
        GLenum format;
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

    // Private helper methods
    bool CreateContext();
    bool CreateShaders();
    bool CreateGeometry();
    bool CreateFramebuffer();
    bool CreateCEFTexture(int width, int height);

    void CleanupFramebuffer();
    void CleanupCEFTexture();
    void CleanupShaders();
    void CleanupGeometry();

    // Shader compilation helpers
    bool CompileShader(GLenum type, const std::string& source, GLuint& shader);
    bool LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint& program);

    // Error handling
    void LogError(const std::string& message);
    std::string GetGLErrorString(GLenum error);
    bool CheckGLError(const std::string& operation);

    // OpenGL extension loading
    bool LoadExtensions();
    bool CheckExtension(const std::string& extension);
};

// OpenGL shader sources
namespace OpenGLShaders {
    extern const char* VERTEX_SHADER_SOURCE;
    extern const char* FRAGMENT_SHADER_SOURCE;
}