// Remove the manual defines since CEF already defines them on command line
// #define WIN32_LEAN_AND_MEAN
// #define NOMINMAX
#include <windows.h>
#include <shellapi.h>  // Add this for Shell_NotifyIcon

// Undefine Windows macros that conflict with CEF
#ifdef GetFirstChild
#undef GetFirstChild
#endif
#ifdef GetNextSibling
#undef GetNextSibling
#endif
#ifdef GetPrevSibling
#undef GetPrevSibling
#endif
#ifdef GetParent
#undef GetParent
#endif

// Removed SDL includes - using CEF views exclusively
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_crash_util.h"
#include "include/wrapper/cef_helpers.h"
#include "include/views/cef_window.h"
#include "include/views/cef_window_delegate.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_browser_view_delegate.h"
#include "include/cef_image.h"
#include <filesystem>
#include <fstream>

// Local includes
#include "config.hpp"
#include "logger.hpp"
#include "client.hpp"
#include "app.hpp"
#include "binaryresourceprovider.hpp"

// Global variables
CefRefPtr<SimpleClient> g_client;
CefRefPtr<CefWindow> g_cef_window;
CefRefPtr<CefBrowserView> g_browser_view;
bool g_running = true;

// Global icon handle for reuse
static HICON g_app_icon = NULL;

// Load application icon once and cache it
HICON LoadApplicationIcon() {
    if (g_app_icon) {
        return g_app_icon; // Return cached icon
    }
    
    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (!hInstance) {
        Logger::LogMessage("Failed to get module handle for icon loading");
        return NULL;
    }
    
    // Try to load icon from resource (ID 101 as defined in app.rc)
    g_app_icon = LoadIcon(hInstance, MAKEINTRESOURCE(101));
    if (!g_app_icon) {
        Logger::LogMessage("Failed to load application icon from resource ID 101");
        // Fallback to system default application icon
        g_app_icon = LoadIcon(NULL, IDI_APPLICATION);
    }
    
    if (g_app_icon) {
        Logger::LogMessage("Application icon loaded successfully");
    } else {
        Logger::LogMessage("Failed to load any application icon");
    }
    
    return g_app_icon;
}

// Set taskbar icon with proper window class registration
void SetPermanentTaskbarIcon(HWND hwnd) {
    if (!hwnd) {
        Logger::LogMessage("Invalid window handle for taskbar icon");
        return;
    }

    HICON hIcon = LoadApplicationIcon();
    if (!hIcon) {
        Logger::LogMessage("No icon available for taskbar");
        return;
    }
    
    // Set both large and small icons
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    
    // Ensure window appears in taskbar with proper extended style
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_APPWINDOW);
    
    // Force taskbar to update the icon
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    
    Logger::LogMessage("Permanent taskbar icon set successfully");
}

// Alternative method: Set application ID to distinguish from Chromium
void SetApplicationUserModelID(HWND hwnd) {
    // Load Shell32.dll dynamically to avoid dependency issues
    HMODULE hShell32 = LoadLibrary(L"Shell32.dll");
    if (hShell32) {
        typedef HRESULT (WINAPI *SetCurrentProcessExplicitAppUserModelIDProc)(PCWSTR);
        SetCurrentProcessExplicitAppUserModelIDProc SetCurrentProcessExplicitAppUserModelID = 
            (SetCurrentProcessExplicitAppUserModelIDProc)GetProcAddress(hShell32, "SetCurrentProcessExplicitAppUserModelID");
        
        if (SetCurrentProcessExplicitAppUserModelID) {
            // Set unique application ID to separate from Chromium
            HRESULT hr = SetCurrentProcessExplicitAppUserModelID(L"SwipeIDE.Application.1.0");
            if (SUCCEEDED(hr)) {
                Logger::LogMessage("Application User Model ID set successfully");
            } else {
                Logger::LogMessage("Failed to set Application User Model ID");
            }
        }
        FreeLibrary(hShell32);
    }
}

// Convert Windows HICON to CEF image for window icon
CefRefPtr<CefImage> ConvertIconToCefImage(HICON hIcon) {
    if (!hIcon) {
        return nullptr;
    }
    
    // Get icon info to extract bitmap data
    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo)) {
        Logger::LogMessage("Failed to get icon info for CEF conversion");
        return nullptr;
    }
    
    // Get bitmap info for the color bitmap
    BITMAP bmp;
    if (!GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp)) {
        Logger::LogMessage("Failed to get bitmap object for CEF conversion");
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        return nullptr;
    }
    
    // Create device context and get bitmap bits
    HDC hdc = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdc);
    
    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = -bmp.bmHeight; // Negative for top-down DIB
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    
    std::vector<uint8_t> bitmapData(bmp.bmWidth * bmp.bmHeight * 4);
    
    if (!GetDIBits(hdcMem, iconInfo.hbmColor, 0, bmp.bmHeight, bitmapData.data(), 
                   (BITMAPINFO*)&bi, DIB_RGB_COLORS)) {
        Logger::LogMessage("Failed to get DIB bits for CEF conversion");
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdc);
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        return nullptr;
    }
    
    // Create CefImage from bitmap data
    CefRefPtr<CefImage> image = CefImage::CreateImage();
    if (!image->AddBitmap(1.0f, bmp.bmWidth, bmp.bmHeight, CEF_COLOR_TYPE_BGRA_8888, 
                         CEF_ALPHA_TYPE_PREMULTIPLIED, bitmapData.data(), bmp.bmWidth * 4)) {
        Logger::LogMessage("Failed to create CefImage from bitmap data");
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdc);
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        return nullptr;
    }
    
    // Cleanup
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdc);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
    
    return image;
}

// Custom browser view delegate to hide browser UI
class CustomBrowserViewDelegate : public CefBrowserViewDelegate {
public:
    CustomBrowserViewDelegate() {}
    
    // Override to customize browser view behavior
    CefRefPtr<CefBrowserViewDelegate> GetDelegateForPopupBrowserView(
        CefRefPtr<CefBrowserView> browser_view,
        const CefBrowserSettings& settings,
        CefRefPtr<CefClient> client,
        bool is_devtools) override {
        return nullptr;
    }
    
    // Override to hide Chrome toolbar and UI elements
    ChromeToolbarType GetChromeToolbarType(
        CefRefPtr<CefBrowserView> browser_view) override {
        return CEF_CTT_NONE; // Hide all Chrome UI elements
    }
    
private:
    IMPLEMENT_REFCOUNTING(CustomBrowserViewDelegate);
};

// Custom window delegate for borderless window with dragging support
class CustomWindowDelegate : public CefWindowDelegate {
public:
    CustomWindowDelegate() {}
    
    // Called when window is created - set the icon here
    void OnWindowCreated(CefRefPtr<CefWindow> window) override {
        // Get window handle
        HWND hwnd = window->GetWindowHandle();
        if (!hwnd) {
            Logger::LogMessage("Failed to get window handle in OnWindowCreated");
            return;
        }
        
        // Set unique Application User Model ID first
        SetApplicationUserModelID(hwnd);
        
        // Load the application icon once
        HICON hIcon = LoadApplicationIcon();
        if (hIcon) {
            // Convert to CEF image for CEF window icon
            CefRefPtr<CefImage> cefIcon = ConvertIconToCefImage(hIcon);
            if (cefIcon) {
                window->SetWindowIcon(cefIcon);
                window->SetWindowAppIcon(cefIcon);
                Logger::LogMessage("CEF window icons set successfully");
            }
            
            // Set native Windows taskbar icon
            SetPermanentTaskbarIcon(hwnd);
        } else {
            Logger::LogMessage("Failed to load application icon in OnWindowCreated");
        }
    }
    
    // Use standard window frame to ensure proper taskbar integration
    bool IsFrameless(CefRefPtr<CefWindow> window) override {
        return true;  // Use standard frame for reliable taskbar icon display
    }
    
    // Allow window to be resizable
    bool CanResize(CefRefPtr<CefWindow> window) override {
        return true;
    }
    
    // Set initial window size
    CefSize GetPreferredSize(CefRefPtr<CefView> view) override {
        return CefSize(1200, 800);
    }
    
    // Handle window close
    bool CanClose(CefRefPtr<CefWindow> window) override {
        g_running = false;
        return true;
    }
    
    // Note: Window dragging is handled automatically by CEF for frameless windows
    
private:
    IMPLEMENT_REFCOUNTING(CustomWindowDelegate);
};

// Handle events
void HandleEvents() {
    // CEF handles all events through its message loop
    // No additional event handling needed
}

// Use WinMain instead of main for Windows applications without console
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Pre-load application icon to ensure it's available
    LoadApplicationIcon();
    
    // Set Application User Model ID early in the process
    SetApplicationUserModelID(NULL);
    
    void* sandbox_info = nullptr;
    CefMainArgs main_args(GetModuleHandle(nullptr));

    // Create app instance for both main and sub-processes
    CefRefPtr<SimpleApp> app(new SimpleApp);
    
    // CEF sub-process check
    int exit_code = CefExecuteProcess(main_args, app.get(), sandbox_info);
    if (exit_code >= 0) {
        return exit_code;
    }

    std::string windowTitle = AppConfig::IsDebugMode() ? 
        "SwipeIDE - Development Mode" : "SwipeIDE - Release Mode";

    // CEF settings with security enhancements
    CefSettings settings;
    settings.no_sandbox = false;  // Enable sandboxing for security
    settings.multi_threaded_message_loop = false;
    settings.windowless_rendering_enabled = false;
    settings.log_severity = LOGSEVERITY_DISABLE;  // Disable logging to reduce overhead
    settings.remote_debugging_port = -1;  // Disable remote debugging
    
    // Set absolute cache paths to avoid singleton warnings
    std::filesystem::path cache_dir = std::filesystem::current_path() / "cache";
    std::string cache_path = cache_dir.string();
    CefString(&settings.cache_path).FromASCII(cache_path.c_str());
    CefString(&settings.root_cache_path).FromASCII(cache_path.c_str());
    
    // Use empty subprocess path to let CEF handle it automatically
    CefString(&settings.browser_subprocess_path).FromASCII("");

    CefInitialize(main_args, settings, app.get(), sandbox_info);

    // Initialize crash reporting if enabled
    if (CefCrashReportingEnabled()) {
        // Set crash keys for debugging purposes
        CefSetCrashKeyValue("app_version", "1.0.0");
        CefSetCrashKeyValue("component", "main_process");
        CefSetCrashKeyValue("user_action", "startup");
        Logger::LogMessage("Crash reporting enabled");
    } else {
        Logger::LogMessage("Crash reporting disabled - check crash_reporter.cfg");
    }

    // Register scheme handler factory for miko:// protocol
    CefRegisterSchemeHandlerFactory("miko", "", new BinaryResourceProvider());

    // Create CEF views-based borderless window
    g_client = new SimpleClient();
    std::string startupUrl = AppConfig::GetStartupUrl();
    
    // Create browser view with hidden UI elements
    CefBrowserSettings browser_settings;
    
    // Configure browser settings with security restrictions
     browser_settings.javascript_access_clipboard = STATE_DISABLED;  // Disable clipboard access for security
     browser_settings.javascript_dom_paste = STATE_DISABLED;  // Disable DOM paste for security
     browser_settings.local_storage = STATE_ENABLED;
     browser_settings.javascript_close_windows = STATE_DISABLED;  // Prevent JavaScript from closing windows

    // Create browser view delegate to hide UI elements
    CefRefPtr<CustomBrowserViewDelegate> browser_view_delegate = new CustomBrowserViewDelegate();
    
    g_browser_view = CefBrowserView::CreateBrowserView(g_client, startupUrl, browser_settings, nullptr, nullptr, browser_view_delegate);
    
    // Create window with custom delegate for borderless functionality
    CefRefPtr<CustomWindowDelegate> window_delegate = new CustomWindowDelegate();
    g_cef_window = CefWindow::CreateTopLevelWindow(window_delegate);
    
    // Add browser view to window
    g_cef_window->AddChildView(g_browser_view);
    
    // Set window title
    g_cef_window->SetTitle(windowTitle);
    
    // Show the window
    g_cef_window->Show();
    
    // Center the window
    g_cef_window->CenterWindow(CefSize(1200, 800));

    // Final taskbar icon verification after window is fully shown
    Sleep(200); // Brief delay for window initialization
    HWND hwnd = g_cef_window->GetWindowHandle();
    if (hwnd) {
        SetPermanentTaskbarIcon(hwnd);
        Logger::LogMessage("Final taskbar icon verification completed");
    }

    // Log startup information
    Logger::LogMessage("=== SwipeIDE CEF + SDL Application ===");
    Logger::LogMessage("Mode: " + std::string(AppConfig::IsDebugMode() ? "DEBUG" : "RELEASE"));
    Logger::LogMessage("URL: " + startupUrl);
    if (AppConfig::IsDebugMode()) {
        Logger::LogMessage("Remote debugging: http://localhost:9222");
        Logger::LogMessage("Make sure React dev server is running: cd renderer && bun run dev");
    }
    Logger::LogMessage("======================================");

    // Main loop
    while (g_running && g_cef_window && !g_cef_window->IsClosed()) {
        HandleEvents();
        CefDoMessageLoopWork();
        Sleep(1); // Small delay to prevent 100% CPU usage
    }

    // Cleanup
    CefShutdown();

    return 0;
}