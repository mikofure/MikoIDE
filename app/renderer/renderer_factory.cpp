#include "renderer_factory.hpp"
#include "windows/dx11_renderer.hpp"

#include <iostream>

#ifdef _WIN32
    #include <windows.h>
    #include <d3d11.h>
    #pragma comment(lib, "d3d11.lib")
#endif

#ifdef __linux__
    #include <GL/gl.h>
    #include <vulkan/vulkan.h>
    #include <dlfcn.h>
    #include "linux/opengl_renderer.hpp"
    #include "linux/vulkan_renderer.hpp"
#endif

std::unique_ptr<IRenderer> RendererFactory::CreateRenderer(
    RendererPreference preference, 
    IRenderer::RendererType specificType) {
    
    Platform platform = GetCurrentPlatform();
    IRenderer::RendererType targetType;

    switch (preference) {
        case RendererPreference::Default:
            targetType = GetDefaultRendererForPlatform();
            break;
        case RendererPreference::Performance:
            targetType = GetBestPerformanceRenderer();
            break;
        case RendererPreference::Compatibility:
            targetType = GetMostCompatibleRenderer();
            break;
        case RendererPreference::Specific:
            targetType = specificType;
            break;
        default:
            targetType = GetDefaultRendererForPlatform();
            break;
    }

    // Verify the target renderer is supported
    if (!IsRendererSupported(targetType)) {
        std::cerr << "Requested renderer " << GetRendererDescription(targetType) 
                  << " is not supported on " << PlatformToString(platform) << std::endl;
        
        // Fall back to default
        targetType = GetDefaultRendererForPlatform();
        if (!IsRendererSupported(targetType)) {
            std::cerr << "No supported renderers found!" << std::endl;
            return nullptr;
        }
    }

    std::unique_ptr<IRenderer> renderer;
    bool success = false;

    switch (targetType) {
        case IRenderer::RendererType::DirectX11:
            renderer = CreateDirectX11Renderer();
            success = (renderer != nullptr);
            break;
        case IRenderer::RendererType::OpenGL:
            renderer = CreateOpenGLRenderer();
            success = (renderer != nullptr);
            break;
        case IRenderer::RendererType::Vulkan:
            renderer = CreateVulkanRenderer();
            success = (renderer != nullptr);
            break;
        default:
            std::cerr << "Unknown renderer type requested" << std::endl;
            return nullptr;
    }

    LogRendererCreation(targetType, success);
    return renderer;
}

std::unique_ptr<IRenderer> RendererFactory::CreateDefaultRenderer() {
    return CreateRenderer(RendererPreference::Default);
}

std::unique_ptr<IRenderer> RendererFactory::CreateBestPerformanceRenderer() {
    return CreateRenderer(RendererPreference::Performance);
}

std::unique_ptr<IRenderer> RendererFactory::CreateMostCompatibleRenderer() {
    return CreateRenderer(RendererPreference::Compatibility);
}

RendererFactory::Platform RendererFactory::GetCurrentPlatform() {
#ifdef _WIN32
    return Platform::Windows;
#elif defined(__linux__)
    return Platform::Linux;
#else
    return Platform::Unknown;
#endif
}

bool RendererFactory::IsRendererSupported(IRenderer::RendererType type) {
    switch (type) {
        case IRenderer::RendererType::DirectX11:
            return CheckDirectX11Support();
        case IRenderer::RendererType::OpenGL:
            return CheckOpenGLSupport();
        case IRenderer::RendererType::Vulkan:
            return CheckVulkanSupport();
        default:
            return false;
    }
}

std::vector<IRenderer::RendererType> RendererFactory::GetSupportedRenderers() {
    std::vector<IRenderer::RendererType> supported;
    
    if (IsRendererSupported(IRenderer::RendererType::DirectX11)) {
        supported.push_back(IRenderer::RendererType::DirectX11);
    }
    if (IsRendererSupported(IRenderer::RendererType::OpenGL)) {
        supported.push_back(IRenderer::RendererType::OpenGL);
    }
    if (IsRendererSupported(IRenderer::RendererType::Vulkan)) {
        supported.push_back(IRenderer::RendererType::Vulkan);
    }
    
    return supported;
}

IRenderer::RendererType RendererFactory::GetDefaultRendererForPlatform() {
    Platform platform = GetCurrentPlatform();
    
    switch (platform) {
        case Platform::Windows:
            // DirectX 11 is the default on Windows
            if (IsRendererSupported(IRenderer::RendererType::DirectX11)) {
                return IRenderer::RendererType::DirectX11;
            }
            // Fall back to OpenGL if DirectX is not available
            if (IsRendererSupported(IRenderer::RendererType::OpenGL)) {
                return IRenderer::RendererType::OpenGL;
            }
            // Last resort: Vulkan
            if (IsRendererSupported(IRenderer::RendererType::Vulkan)) {
                return IRenderer::RendererType::Vulkan;
            }
            break;
            
        case Platform::Linux:
            // Vulkan is preferred on Linux for performance
            if (IsRendererSupported(IRenderer::RendererType::Vulkan)) {
                return IRenderer::RendererType::Vulkan;
            }
            // Fall back to OpenGL
            if (IsRendererSupported(IRenderer::RendererType::OpenGL)) {
                return IRenderer::RendererType::OpenGL;
            }
            break;
            
        default:
            // Try OpenGL as the most compatible option
            if (IsRendererSupported(IRenderer::RendererType::OpenGL)) {
                return IRenderer::RendererType::OpenGL;
            }
            break;
    }
    
    // This should not happen if the platform detection works correctly
    return IRenderer::RendererType::OpenGL;
}

IRenderer::RendererType RendererFactory::GetBestPerformanceRenderer() {
    // Priority: Vulkan > DirectX11 > OpenGL
    if (IsRendererSupported(IRenderer::RendererType::Vulkan)) {
        return IRenderer::RendererType::Vulkan;
    }
    if (IsRendererSupported(IRenderer::RendererType::DirectX11)) {
        return IRenderer::RendererType::DirectX11;
    }
    if (IsRendererSupported(IRenderer::RendererType::OpenGL)) {
        return IRenderer::RendererType::OpenGL;
    }
    
    return IRenderer::RendererType::OpenGL; // Fallback
}

IRenderer::RendererType RendererFactory::GetMostCompatibleRenderer() {
    // Priority: OpenGL > DirectX11 > Vulkan
    if (IsRendererSupported(IRenderer::RendererType::OpenGL)) {
        return IRenderer::RendererType::OpenGL;
    }
    if (IsRendererSupported(IRenderer::RendererType::DirectX11)) {
        return IRenderer::RendererType::DirectX11;
    }
    if (IsRendererSupported(IRenderer::RendererType::Vulkan)) {
        return IRenderer::RendererType::Vulkan;
    }
    
    return IRenderer::RendererType::OpenGL; // Fallback
}

std::string RendererFactory::GetRendererDescription(IRenderer::RendererType type) {
    switch (type) {
        case IRenderer::RendererType::DirectX11:
            return "DirectX 11 (Windows)";
        case IRenderer::RendererType::OpenGL:
            return "OpenGL (Cross-platform)";
        case IRenderer::RendererType::Vulkan:
            return "Vulkan (High-performance)";
        default:
            return "Unknown Renderer";
    }
}

bool RendererFactory::RequiresSpecificDrivers(IRenderer::RendererType type) {
    switch (type) {
        case IRenderer::RendererType::DirectX11:
            return false; // Built into Windows
        case IRenderer::RendererType::OpenGL:
            return false; // Usually available with basic graphics drivers
        case IRenderer::RendererType::Vulkan:
            return true;  // Requires modern graphics drivers with Vulkan support
        default:
            return true;
    }
}

std::unique_ptr<IRenderer> RendererFactory::CreateDirectX11Renderer() {
#ifdef _WIN32
    try {
        return std::make_unique<DX11Renderer>();
    } catch (const std::exception& e) {
        std::cerr << "Failed to create DirectX 11 renderer: " << e.what() << std::endl;
        return nullptr;
    }
#else
    std::cerr << "DirectX 11 renderer not available on this platform" << std::endl;
    return nullptr;
#endif
}

std::unique_ptr<IRenderer> RendererFactory::CreateOpenGLRenderer() {
#ifdef __linux__
    try {
        return std::make_unique<OpenGLRenderer>();
    } catch (const std::exception& e) {
        std::cerr << "Failed to create OpenGL renderer: " << e.what() << std::endl;
        return nullptr;
    }
#else
    std::cerr << "OpenGL renderer not available on this platform" << std::endl;
    return nullptr;
#endif
}

std::unique_ptr<IRenderer> RendererFactory::CreateVulkanRenderer() {
#ifdef __linux__
    try {
        return std::make_unique<VulkanRenderer>();
    } catch (const std::exception& e) {
        std::cerr << "Failed to create Vulkan renderer: " << e.what() << std::endl;
        return nullptr;
    }
#else
    std::cerr << "Vulkan renderer not available on this platform" << std::endl;
    return nullptr;
#endif
}

bool RendererFactory::CheckDirectX11Support() {
#ifdef _WIN32
    // Try to create a D3D11 device to check support
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    
    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // Adapter
        D3D_DRIVER_TYPE_HARDWARE,   // Driver type
        nullptr,                    // Software
        0,                          // Flags
        nullptr,                    // Feature levels
        0,                          // Num feature levels
        D3D11_SDK_VERSION,          // SDK version
        &device,                    // Device
        nullptr,                    // Feature level
        &context                    // Context
    );
    
    if (SUCCEEDED(hr)) {
        if (device) device->Release();
        if (context) context->Release();
        return true;
    }
    
    return false;
#else
    return false;
#endif
}

bool RendererFactory::CheckOpenGLSupport() {
#ifdef __linux__
    // On Linux, we assume OpenGL is available if we can load the library
    void* handle = dlopen("libGL.so.1", RTLD_LAZY);
    if (handle) {
        dlclose(handle);
        return true;
    }
    
    // Try alternative names
    handle = dlopen("libGL.so", RTLD_LAZY);
    if (handle) {
        dlclose(handle);
        return true;
    }
    
    return false;
#else
    return false;
#endif
}

bool RendererFactory::CheckVulkanSupport() {
#ifdef __linux__
    // Try to load Vulkan library and create an instance
    void* handle = dlopen("libvulkan.so.1", RTLD_LAZY);
    if (!handle) {
        handle = dlopen("libvulkan.so", RTLD_LAZY);
    }
    
    if (handle) {
        // Try to get the vkCreateInstance function
        typedef VkResult (*PFN_vkCreateInstance)(const VkInstanceCreateInfo*, const VkAllocationCallbacks*, VkInstance*);
        PFN_vkCreateInstance vkCreateInstance = (PFN_vkCreateInstance)dlsym(handle, "vkCreateInstance");
        
        if (vkCreateInstance) {
            // Try to create a minimal Vulkan instance
            VkApplicationInfo appInfo = {};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Hyperion Renderer Test";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "Hyperion";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            VkInstance instance;
            VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
            
            if (result == VK_SUCCESS) {
                // Clean up
                typedef void (*PFN_vkDestroyInstance)(VkInstance, const VkAllocationCallbacks*);
                PFN_vkDestroyInstance vkDestroyInstance = (PFN_vkDestroyInstance)dlsym(handle, "vkDestroyInstance");
                if (vkDestroyInstance) {
                    vkDestroyInstance(instance, nullptr);
                }
                dlclose(handle);
                return true;
            }
        }
        
        dlclose(handle);
    }
    
    return false;
#else
    return false;
#endif
}

void RendererFactory::LogRendererCreation(IRenderer::RendererType type, bool success) {
    std::string rendererName = GetRendererDescription(type);
    if (success) {
        std::cout << "Successfully created " << rendererName << " renderer" << std::endl;
    } else {
        std::cerr << "Failed to create " << rendererName << " renderer" << std::endl;
    }
}

std::string RendererFactory::PlatformToString(Platform platform) {
    switch (platform) {
        case Platform::Windows:
            return "Windows";
        case Platform::Linux:
            return "Linux";
        case Platform::Unknown:
        default:
            return "Unknown";
    }
}