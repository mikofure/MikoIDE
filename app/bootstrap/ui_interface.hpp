#pragma once

#include <functional>
#include <memory>
#include <string>

// Forward declarations for platform-specific types
#ifdef _WIN32
#include <windows.h>
using NativeWindowHandle = HWND;
using NativeInstanceHandle = HINSTANCE;
using PlatformWindow = HWND;
using PlatformInstance = HINSTANCE;
#elif defined(__linux__)
#include <gtk/gtk.h>
using NativeWindowHandle = GtkWidget*;
using NativeInstanceHandle = void*;
using PlatformWindow = GtkWidget*;
using PlatformInstance = void*;
#endif

// Progress callback function type
using ProgressCallback = std::function<void(int percentage, const std::string& status, 
                                          size_t bytesDownloaded, size_t totalBytes)>;

// Abstract interface for splash screen
class ISplashScreen {
public:
    virtual ~ISplashScreen() = default;
    
    virtual bool Create(NativeInstanceHandle instance, const std::string& title = "MikoIDE") = 0;
    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void UpdateStatus(const std::string& status) = 0;
    virtual void SetTitle(const std::string& title) = 0;
    virtual NativeWindowHandle GetNativeHandle() const = 0;
    virtual bool IsVisible() const = 0;
    
    // Static factory method
    static std::unique_ptr<ISplashScreen> Create();
};

// Abstract interface for modern dialog
class IModernDialog {
public:
    virtual ~IModernDialog() = default;
    virtual bool Create(PlatformInstance instance, PlatformWindow parent, const std::wstring& title) = 0;
    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void SetProgress(int percentage) = 0;
    virtual void SetStatus(const std::string& status) = 0;
    virtual void SetDownloadInfo(size_t bytesDownloaded, size_t totalBytes, size_t speed) = 0;
    virtual void UpdateProgress(int percentage, const std::wstring& status, size_t bytesDownloaded, size_t totalBytes) = 0;
    virtual bool IsCancelled() const = 0;
    virtual NativeWindowHandle GetNativeHandle() const = 0;
    
    // Static factory method
    static std::unique_ptr<IModernDialog> Create();
};