#pragma once

#include <string>
#include <memory>

// Forward declarations
struct SDL_Window;

// Platform-specific handle type
#ifdef _WIN32
    #include <windows.h>
    using PlatformWindowHandle = HWND;
#else
    using PlatformWindowHandle = void*;
#endif

// Platform abstraction interface
class IPlatformWindow {
public:
    virtual ~IPlatformWindow() = default;
    
    // Window management
    virtual bool Initialize(SDL_Window* sdl_window) = 0;
    virtual void Shutdown() = 0;
    virtual PlatformWindowHandle GetNativeHandle() const = 0;
    
    // Window styling
    virtual void SetRoundedCorners(bool enable) = 0;
    virtual void SetDarkMode(bool enable) = 0;
    virtual void SetBorderless(bool borderless) = 0;
    virtual void ExtendFrameIntoClientArea() = 0;
    
    // DPI handling
    virtual float GetDPIScale() const = 0;
    virtual void UpdateDPIScale() = 0;
    
    // Window properties
    virtual void SetLayeredWindow(bool enable, int alpha = 255) = 0;
    virtual void SetTopMost(bool topmost) = 0;
};

// Known folder types
enum class KnownFolder {
    Downloads,
    AppData,
    Temp
};

// Platform-specific file operations
class IPlatformFileSystem {
public:
    virtual ~IPlatformFileSystem() = default;
    
    // Path operations
    virtual std::string GetDownloadsPath() const = 0;
    virtual std::string GetAppDataPath() const = 0;
    virtual std::string GetTempPath() const = 0;
    virtual std::string GetKnownFolderPath(KnownFolder folder) const = 0;
    
    // File operations
    virtual bool FileExists(const std::string& path) const = 0;
    virtual bool CreateDirectory(const std::string& path) const = 0;
};

// Platform factory
class PlatformFactory {
public:
    static std::unique_ptr<IPlatformWindow> CreatePlatformWindow();
    static std::unique_ptr<IPlatformFileSystem> CreatePlatformFileSystem();
};

// Platform utilities
namespace PlatformUtils {
    // Get platform name
    std::string GetPlatformName();
    
    // Check if running on specific platform
    bool IsWindows();
    bool IsLinux();
    bool IsMacOS();
    
    // System information
    std::string GetSystemVersion();
    int GetCPUCoreCount();
    size_t GetTotalMemory();
}