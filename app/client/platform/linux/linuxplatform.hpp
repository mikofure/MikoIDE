#pragma once

#include "../sysplatform.hpp"
#include <X11/Xlib.h>
#include <string>

// Linux-specific platform window implementation
class LinuxPlatformWindow : public IPlatformWindow {
public:
    LinuxPlatformWindow();
    ~LinuxPlatformWindow() override;
    
    // IPlatformWindow interface
    bool Initialize(SDL_Window* sdl_window) override;
    void Shutdown() override;
    PlatformWindowHandle GetNativeHandle() const override;
    
    // Window styling
    void SetRoundedCorners(bool enable) override;
    void SetDarkMode(bool enable) override;
    void SetBorderless(bool borderless) override;
    void ExtendFrameIntoClientArea() override;
    
    // DPI handling
    float GetDPIScale() const override;
    void UpdateDPIScale() override;
    
    // Window properties
    void SetLayeredWindow(bool enable, int alpha = 255) override;
    void SetTopMost(bool topmost) override;

private:
    Window x11_window_;
    Display* x11_display_;
    SDL_Window* sdl_window_;
    float dpi_scale_;
    
    void InitializeX11Properties();
    void SetWindowProperty(const char* property_name, const char* value);
    void SetWindowType(const char* type);
};

// Linux-specific file system implementation
class LinuxPlatformFileSystem : public IPlatformFileSystem {
public:
    LinuxPlatformFileSystem() = default;
    ~LinuxPlatformFileSystem() override = default;
    
    // File system operations
    std::string GetDownloadsPath() const override;
    std::string GetAppDataPath() const override;
    std::string GetTempPath() const override;
    std::string GetKnownFolderPath(KnownFolder folder) const override;
    bool FileExists(const std::string& path) const override;
    bool CreateDirectory(const std::string& path) const override;

private:
    std::string GetHomeDirectory() const;
    std::string GetXDGDirectory(const char* xdg_var, const char* fallback) const;
};