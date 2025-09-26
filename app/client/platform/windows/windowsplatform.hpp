#pragma once

#include "../sysplatform.hpp"
#include <windows.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <shlobj.h>

// Windows-specific platform window implementation
class WindowsPlatformWindow : public IPlatformWindow {
public:
    WindowsPlatformWindow();
    ~WindowsPlatformWindow() override;
    
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
    HWND hwnd_;
    SDL_Window* sdl_window_;
    float dpi_scale_;
    
    void InitializeDwmApi();
};

// Windows-specific file system implementation
class WindowsPlatformFileSystem : public IPlatformFileSystem {
public:
    WindowsPlatformFileSystem() = default;
    ~WindowsPlatformFileSystem() override = default;
    
    // File system operations
    std::string GetDownloadsPath() const override;
    std::string GetAppDataPath() const override;
    std::string GetTempPath() const override;
    std::string GetKnownFolderPath(KnownFolder folder) const override;
    bool FileExists(const std::string& path) const override;
    bool CreateDirectory(const std::string& path) const override;

private:
    std::string GetKnownFolderPath(const KNOWNFOLDERID& folder_id) const;
};