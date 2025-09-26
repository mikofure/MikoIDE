#include "windowsplatform.hpp"
#include <SDL3/SDL.h>
#include <iostream>
#include <filesystem>

// Link required Windows libraries
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shell32.lib")

// WindowsPlatformWindow implementation
WindowsPlatformWindow::WindowsPlatformWindow() 
    : hwnd_(nullptr), sdl_window_(nullptr), dpi_scale_(1.0f) {
}

WindowsPlatformWindow::~WindowsPlatformWindow() {
    Shutdown();
}

bool WindowsPlatformWindow::Initialize(SDL_Window* sdl_window) {
    if (!sdl_window) {
        return false;
    }
    
    sdl_window_ = sdl_window;
    
    // Get Windows HWND from SDL
    hwnd_ = (HWND)SDL_GetPointerProperty(
        SDL_GetWindowProperties(sdl_window_),
        SDL_PROP_WINDOW_WIN32_HWND_POINTER, 
        nullptr
    );
    
    if (!hwnd_) {
        std::cerr << "Failed to get HWND from SDL window" << std::endl;
        return false;
    }
    
    // Initialize DWM API and apply styling
    InitializeDwmApi();
    UpdateDPIScale();
    
    return true;
}

void WindowsPlatformWindow::Shutdown() {
    hwnd_ = nullptr;
    sdl_window_ = nullptr;
}

PlatformWindowHandle WindowsPlatformWindow::GetNativeHandle() const {
    return hwnd_;
}

void WindowsPlatformWindow::SetRoundedCorners(bool enable) {
    if (!hwnd_) return;
    
    DWM_WINDOW_CORNER_PREFERENCE cornerPreference = enable ? 
        DWMWCP_ROUND : DWMWCP_DONOTROUND;
    
    DwmSetWindowAttribute(
        hwnd_, 
        DWMWA_WINDOW_CORNER_PREFERENCE,
        &cornerPreference, 
        sizeof(cornerPreference)
    );
}

void WindowsPlatformWindow::SetDarkMode(bool enable) {
    if (!hwnd_) return;
    
    BOOL darkMode = enable ? TRUE : FALSE;
    DwmSetWindowAttribute(
        hwnd_, 
        DWMWA_USE_IMMERSIVE_DARK_MODE, 
        &darkMode,
        sizeof(darkMode)
    );
}

void WindowsPlatformWindow::SetBorderless(bool borderless) {
    if (!hwnd_) return;
    
    LONG_PTR style = GetWindowLongPtr(hwnd_, GWL_STYLE);
    
    if (borderless) {
        // Remove window decorations
        style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
    } else {
        // Add window decorations back
        style |= (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
    }
    
    SetWindowLongPtr(hwnd_, GWL_STYLE, style);
    
    // Update extended style for borderless
    LONG_PTR exStyle = GetWindowLongPtr(hwnd_, GWL_EXSTYLE);
    if (borderless) {
        exStyle |= WS_EX_TOOLWINDOW;
    } else {
        exStyle &= ~WS_EX_TOOLWINDOW;
    }
    SetWindowLongPtr(hwnd_, GWL_EXSTYLE, exStyle);
    
    // Force window to redraw
    SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

void WindowsPlatformWindow::ExtendFrameIntoClientArea() {
    if (!hwnd_) return;
    
    // Extend frame into client area for custom title bar
    MARGINS margins = {0, 0, 0, 1};
    DwmExtendFrameIntoClientArea(hwnd_, &margins);
}

float WindowsPlatformWindow::GetDPIScale() const {
    return dpi_scale_;
}

void WindowsPlatformWindow::UpdateDPIScale() {
    if (!hwnd_) {
        dpi_scale_ = 1.0f;
        return;
    }
    
    UINT dpi = GetDpiForWindow(hwnd_);
    dpi_scale_ = static_cast<float>(dpi) / 96.0f; // 96 DPI is 100% scaling
}

void WindowsPlatformWindow::SetLayeredWindow(bool enable, int alpha) {
    if (!hwnd_) return;
    
    LONG_PTR exStyle = GetWindowLongPtr(hwnd_, GWL_EXSTYLE);
    
    if (enable) {
        exStyle |= WS_EX_LAYERED;
        SetWindowLongPtr(hwnd_, GWL_EXSTYLE, exStyle);
        SetLayeredWindowAttributes(hwnd_, RGB(0, 0, 0), alpha, LWA_ALPHA);
    } else {
        exStyle &= ~WS_EX_LAYERED;
        SetWindowLongPtr(hwnd_, GWL_EXSTYLE, exStyle);
    }
}

void WindowsPlatformWindow::SetTopMost(bool topmost) {
    if (!hwnd_) return;
    
    HWND insertAfter = topmost ? HWND_TOPMOST : HWND_NOTOPMOST;
    SetWindowPos(hwnd_, insertAfter, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void WindowsPlatformWindow::InitializeDwmApi() {
    if (!hwnd_) return;
    
    // Enable rounded corners by default
    SetRoundedCorners(true);
    
    // Extend frame into client area for custom styling
    ExtendFrameIntoClientArea();
}

// WindowsPlatformFileSystem implementation
std::string WindowsPlatformFileSystem::GetDownloadsPath() const {
    return GetKnownFolderPath(FOLDERID_Downloads);
}

std::string WindowsPlatformFileSystem::GetAppDataPath() const {
    return GetKnownFolderPath(FOLDERID_RoamingAppData);
}

std::string WindowsPlatformFileSystem::GetTempPath() const {
    wchar_t temp_path[MAX_PATH];
    DWORD result = ::GetTempPathW(MAX_PATH, temp_path);
    
    if (result == 0 || result > MAX_PATH) {
        return "";
    }
    
    // Convert wide string to UTF-8
    int size = WideCharToMultiByte(CP_UTF8, 0, temp_path, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return "";
    
    std::string utf8_path(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, temp_path, -1, &utf8_path[0], size, nullptr, nullptr);
    
    return utf8_path;
}

bool WindowsPlatformFileSystem::FileExists(const std::string& path) const {
    return std::filesystem::exists(path);
}

bool WindowsPlatformFileSystem::CreateDirectory(const std::string& path) const {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::exception&) {
        return false;
    }
}

std::string WindowsPlatformFileSystem::GetKnownFolderPath(KnownFolder folder) const {
    switch (folder) {
        case KnownFolder::Downloads:
            return GetDownloadsPath();
        case KnownFolder::AppData:
            return GetAppDataPath();
        case KnownFolder::Temp:
            return GetTempPath();
        default:
            return "";
    }
}

std::string WindowsPlatformFileSystem::GetKnownFolderPath(const KNOWNFOLDERID& folder_id) const {
    PWSTR path_tmp = nullptr;
    
    if (SUCCEEDED(SHGetKnownFolderPath(folder_id, 0, NULL, &path_tmp))) {
        // Convert wide string to UTF-8
        int size = WideCharToMultiByte(CP_UTF8, 0, path_tmp, -1, nullptr, 0, nullptr, nullptr);
        if (size > 0) {
            std::string utf8_path(size - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, path_tmp, -1, &utf8_path[0], size, nullptr, nullptr);
            CoTaskMemFree(path_tmp);
            return utf8_path;
        }
        CoTaskMemFree(path_tmp);
    }
    
    return "";
}