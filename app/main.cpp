#include <windows.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>

// Undefine conflicting Windows macros
#undef min
#undef max
#undef GetMessage

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/base/cef_bind.h"

#include "client/client.hpp"
#include "utils/config.hpp"
#include "utils/logger.hpp"
#include "internal/simpleipc.hpp"
#include "resources/resources.hpp"
#include "client/app.hpp"
#pragma comment(lib, "shlwapi.lib")

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

#include <string>
#include <memory>
#include <filesystem>

// Global variables
CefRefPtr<SimpleClient> g_client;
std::unique_ptr<SDL3Window> g_sdl_window;
bool g_is_closing = false;

// Application settings now come from config.hpp

// Forward declarations
void LoadApplicationIcon();
void SetPermanentTaskbarIcon();
void SetApplicationUserModelID();
CefRefPtr<CefImage> ConvertIconToCefImage(HICON hIcon);
std::string GetDataURI(const std::string& data, const std::string& mime_type);
std::string GetDownloadPath(const std::string& suggested_name);

// Simple CEF App implementation for OSR


// Handle SDL events and CEF message loop
void HandleEvents() {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        if (g_sdl_window && g_sdl_window->HandleEvent(event)) {
            continue;
        }
        
        // Handle application-level events
        switch (event.type) {
            case SDL_EVENT_QUIT:
                g_is_closing = true;
                if (g_client) {
                    g_client->CloseAllBrowsers(false);
                }
                break;
                
            default:
                break;
        }
    }
    
    // Check if window should close
    if (g_sdl_window && g_sdl_window->ShouldClose()) {
        g_is_closing = true;
        if (g_client) {
            g_client->CloseAllBrowsers(false);
        }
    }
}

// Utility function to get executable directory
std::filesystem::path GetExeDir() {
    wchar_t exePath[MAX_PATH];
    DWORD result = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    if (result == 0 || result == MAX_PATH) {
        // Fallback to current directory
        return std::filesystem::current_path();
    }
    
    std::filesystem::path path(exePath);
    return path.parent_path();
}

// Main application entry point
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize logger
    Logger::Initialize();
    Logger::LogMessage("MikoIDE starting up...");

    // Set application properties
    SetApplicationUserModelID();
    LoadApplicationIcon();
    SetPermanentTaskbarIcon();

    // Enable High DPI awareness
    SetProcessDPIAware();

    // ---- Set CEF DLL directories ----
    Logger::LogMessage("Setting up CEF paths...");
    
    // Compute paths BEFORE CEF
    const auto exeDir = GetExeDir();

#ifdef _WIN32
    const bool is64 = sizeof(void*) == 8;
    const std::wstring platform = is64 ? L"windows64" : L"windows32";
    const std::filesystem::path cefDir     = exeDir / L"bin" / L"cef" / platform;
    const std::filesystem::path helperPath = cefDir / L"mikowebhelper.exe";
    const std::filesystem::path cachePath  = exeDir / L"cache";

    // Set DLL search paths BEFORE CefExecuteProcess
    HMODULE k32 = GetModuleHandleW(L"kernel32.dll");
    auto pSetDefaultDllDirectories = reinterpret_cast<BOOL (WINAPI*)(DWORD)>(
        GetProcAddress(k32, "SetDefaultDllDirectories"));
    auto pAddDllDirectory = reinterpret_cast<LPVOID (WINAPI*)(PCWSTR)>(
        GetProcAddress(k32, "AddDllDirectory"));

    if (pSetDefaultDllDirectories && pAddDllDirectory) {
        pSetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
                                  | LOAD_LIBRARY_SEARCH_USER_DIRS
                                  | LOAD_LIBRARY_SEARCH_APPLICATION_DIR
                                  | LOAD_LIBRARY_SEARCH_SYSTEM32);
        pAddDllDirectory(std::wstring(exeDir.wstring()).c_str()); // SDL3.dll beside exe
        pAddDllDirectory(cefDir.wstring().c_str());               // libcef.dll in cef folder
    } else {
        // Fallback (Win7 without KB2533623)
        DWORD need = GetEnvironmentVariableW(L"PATH", nullptr, 0);
        std::wstring oldPath(need ? need - 1 : 0, L'\0');
        if (need) GetEnvironmentVariableW(L"PATH", oldPath.data(), need);
        std::wstring newPath = cefDir.wstring() + L";" + oldPath;
        SetEnvironmentVariableW(L"PATH", newPath.c_str());
    }
#endif

    // Initialize CEF
    CefMainArgs main_args(hInstance);
    CefRefPtr<SimpleApp> app(new SimpleApp);

    // Execute the secondary process, if any
    int exit_code = CefExecuteProcess(main_args, app, nullptr);
    if (exit_code >= 0) {
        return exit_code;
    }

    // CEF settings
    CefSettings settings;
    settings.no_sandbox = true;
    settings.windowless_rendering_enabled = true;
    settings.multi_threaded_message_loop = false; // We'll use our own message loop
    
#ifdef _WIN32
    CefString(&settings.cache_path) = cachePath.wstring();
    CefString(&settings.browser_subprocess_path) = helperPath.wstring();
#endif
    
    // Set log level
#ifdef _DEBUG
    settings.log_severity = LOGSEVERITY_INFO;
#else
    settings.log_severity = LOGSEVERITY_WARNING;
#endif
    
    // Set locale
    CefString(&settings.locale).FromASCII("en-US");

    // Debug output for paths
    Logger::LogMessage("exeDir=" + exeDir.string());
#ifdef _WIN32
    Logger::LogMessage("cefDir=" + cefDir.string());
    Logger::LogMessage("helper=" + helperPath.string());

    if (!std::filesystem::exists(helperPath)) {
        Logger::LogMessage("Helper not found: " + helperPath.string());
        return 1;
    }
    if (!std::filesystem::exists(cefDir / L"libcef.dll")) {
        Logger::LogMessage("libcef.dll not found in: " + cefDir.string());
        return 1;
    }
#endif

    // Initialize CEF
    if (!CefInitialize(main_args, settings, app, nullptr)) {
        Logger::LogMessage("Failed to initialize CEF");
        return 1;
    }

    Logger::LogMessage("CEF initialized successfully");

    // Register scheme handler factory for miko:// protocol
    CefRegisterSchemeHandlerFactory("miko", "", new BinaryResourceProvider());
    Logger::LogMessage("Registered miko:// scheme handler factory");

    // Initialize SDL3 Window
    g_sdl_window = std::make_unique<SDL3Window>();
    if (!g_sdl_window->Initialize(1200, 800)) {
        Logger::LogMessage("Failed to initialize SDL3 window");
        CefShutdown();
        return 1;
    }
    
    Logger::LogMessage("SDL3 window initialized successfully");

    // Create CEF client and browser
    g_client = new SimpleClient(g_sdl_window.get());
    
    // Configure browser settings
    CefBrowserSettings browser_settings;
    browser_settings.windowless_frame_rate = 0; // 60 FPS for smooth rendering
    
    // Configure window info for off-screen rendering
    CefWindowInfo window_info;
    window_info.SetAsWindowless(g_sdl_window->GetHWND());
    
    // Create the browser synchronously to prevent race conditions
    CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(window_info, g_client, AppConfig::GetStartupUrl(), browser_settings, nullptr, nullptr);
    
    if (!browser) {
        Logger::LogMessage("Failed to create CEF browser");
        CefShutdown();
        return 1;
    }
    
    Logger::LogMessage("CEF browser created successfully");

    // Main message loop
    while (!g_is_closing) {
        // Handle SDL events
        HandleEvents();
        
        // Render the window
        if (g_sdl_window) {
            g_sdl_window->Render();
        }
        
        // Process CEF message loop
        CefDoMessageLoopWork();
        
        // Small delay to prevent 100% CPU usage
        SDL_Delay(1);
        
        // Check if all browsers are closed
        if (g_client && !g_client->HasBrowsers() && g_is_closing) {
            break;
        }
    }

    Logger::LogMessage("Shutting down application...");

    // Cleanup
    g_client = nullptr;
    
    if (g_sdl_window) {
        g_sdl_window->Shutdown();
        g_sdl_window.reset();
    }

    // Shutdown CEF
    CefShutdown();

    Logger::LogMessage("Application shutdown complete");
    Logger::Shutdown();

    return 0;
}

// Utility functions implementation
void LoadApplicationIcon() {
    // Load application icon from resources
    HICON hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(101));
    if (hIcon) {
        Logger::LogMessage("Application icon loaded successfully");
    }
}

void SetPermanentTaskbarIcon() {
    // Set taskbar icon properties
    HICON hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(101));
    if (hIcon) {
        // This would typically be used with the window handle
        Logger::LogMessage("Taskbar icon configured");
    }
}

void SetApplicationUserModelID() {
    // Set Application User Model ID for Windows taskbar grouping
    SetCurrentProcessExplicitAppUserModelID(L"MikoIDE.Application.1.0");
    Logger::LogMessage("Application User Model ID set");
}

CefRefPtr<CefImage> ConvertIconToCefImage(HICON hIcon) {
    if (!hIcon) return nullptr;
    
    // Convert Windows HICON to CEF image
    // This is a simplified implementation
    CefRefPtr<CefImage> image = CefImage::CreateImage();
    
    // Get icon info
    ICONINFO iconInfo;
    if (GetIconInfo(hIcon, &iconInfo)) {
        // Get bitmap info
        BITMAP bmp;
        if (GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp)) {
            // Create CEF image from bitmap data
            // This would require proper bitmap data extraction and conversion
            Logger::LogMessage("Icon converted to CEF image");
        }
        
        // Cleanup
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
    }
    
    return image;
}

std::string GetDataURI(const std::string& data, const std::string& mime_type) {
    // Create a data URI from the given data and MIME type
    std::string data_uri = "data:" + mime_type + ";charset=utf-8,";
    
    // URL encode the data (simplified)
    for (char c : data) {
        if (c == ' ') {
            data_uri += "%20";
        } else if (c == '<') {
            data_uri += "%3C";
        } else if (c == '>') {
            data_uri += "%3E";
        } else if (c == '"') {
            data_uri += "%22";
        } else {
            data_uri += c;
        }
    }
    
    return data_uri;
}

std::string GetDownloadPath(const std::string& suggested_name) {
    // Get the Downloads folder path
    PWSTR downloads_path = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Downloads, 0, nullptr, &downloads_path);
    
    std::string download_path;
    if (SUCCEEDED(hr) && downloads_path) {
        // Convert wide string to narrow string
        int size = WideCharToMultiByte(CP_UTF8, 0, downloads_path, -1, nullptr, 0, nullptr, nullptr);
        if (size > 0) {
            std::string temp(size - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, downloads_path, -1, &temp[0], size, nullptr, nullptr);
            download_path = temp + "\\" + suggested_name;
        }
        CoTaskMemFree(downloads_path);
    }
    
    // Fallback to current directory if Downloads folder not found
    if (download_path.empty()) {
        download_path = suggested_name;
    }
    
    Logger::LogMessage("Download path: " + download_path);
    return download_path;
}

// SDL3 main function wrapper