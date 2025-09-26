#pragma once

#include "renderer_interface.hpp"
#include <memory>
#include <string>
#include <vector>

// Renderer factory for creating platform-specific renderers
class RendererFactory {
public:
    // Platform detection
    enum class Platform {
        Windows,
        Linux,
        Unknown
    };

    // Renderer preference for platforms that support multiple renderers
    enum class RendererPreference {
        Default,        // Use platform default
        Performance,    // Prefer highest performance (Vulkan > DirectX > OpenGL)
        Compatibility,  // Prefer highest compatibility (OpenGL > DirectX > Vulkan)
        Specific        // Use specific renderer type
    };

    // Factory methods
    static std::unique_ptr<IRenderer> CreateRenderer(
        RendererPreference preference = RendererPreference::Default,
        IRenderer::RendererType specificType = IRenderer::RendererType::DirectX11
    );

    static std::unique_ptr<IRenderer> CreateDefaultRenderer();
    static std::unique_ptr<IRenderer> CreateBestPerformanceRenderer();
    static std::unique_ptr<IRenderer> CreateMostCompatibleRenderer();

    // Platform and capability detection
    static Platform GetCurrentPlatform();
    static bool IsRendererSupported(IRenderer::RendererType type);
    static std::vector<IRenderer::RendererType> GetSupportedRenderers();
    static IRenderer::RendererType GetDefaultRendererForPlatform();
    static IRenderer::RendererType GetBestPerformanceRenderer();
    static IRenderer::RendererType GetMostCompatibleRenderer();

    // Renderer information
    static std::string GetRendererDescription(IRenderer::RendererType type);
    static bool RequiresSpecificDrivers(IRenderer::RendererType type);

private:
    // Platform-specific creation methods
    static std::unique_ptr<IRenderer> CreateDirectX11Renderer();
    static std::unique_ptr<IRenderer> CreateOpenGLRenderer();
    static std::unique_ptr<IRenderer> CreateVulkanRenderer();

    // Capability checking
    static bool CheckDirectX11Support();
    static bool CheckOpenGLSupport();
    static bool CheckVulkanSupport();

    // Utility methods
    static void LogRendererCreation(IRenderer::RendererType type, bool success);
    static std::string PlatformToString(Platform platform);
};