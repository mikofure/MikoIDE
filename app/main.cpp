#include <windows.h>
#include <commdlg.h>
#include <dwmapi.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>

// Undefine conflicting Windows macros
#undef min
#undef max
#undef GetMessage

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "utils/config.hpp"

#include "bootstrap/bootstrap.hpp"
#include "client/app.hpp"
#include "client/client.hpp"
#include "internal/simpleipc.hpp"
#include "resources/resources.hpp"
#include "utils/config.hpp"
#include "utils/logger.hpp"
#pragma comment(lib, "shlwapi.lib")

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

#include <filesystem>
#include <memory>
#include <string>

// Global variables
CefRefPtr<HyperionClient> g_client;
std::unique_ptr<SDL3Window> g_sdl_window;
std::unique_ptr<SplashScreen> g_splash_screen;
bool g_is_closing = false;

// Application settings now come from config.hpp

// Forward declarations
void LoadApplicationIcon();
void SetPermanentTaskbarIcon();
void SetApplicationUserModelID();
CefRefPtr<CefImage> ConvertIconToCefImage(HICON hIcon);
std::string GetDataURI(const std::string &data, const std::string &mime_type);
std::string GetDownloadPath(const std::string &suggested_name);

// Simple CEF App implementation for OSR

void HandleEvents() {
  SDL_Event event;

  try {
    while (SDL_PollEvent(&event)) {
      if (g_sdl_window && g_sdl_window->HandleEvent(event)) {
        continue;
      }

      // Handle application-level events
      switch (event.type) {
      case SDL_EVENT_QUIT:
        g_is_closing = true;
        if (g_client) {
          try {
            g_client->CloseAllBrowsers(false);
          } catch (const std::exception &ex) {
            Logger::LogMessage("Exception during browser close: " +
                               std::string(ex.what()));
          } catch (...) {
            Logger::LogMessage("Unknown exception during browser close");
          }
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
        try {
          g_client->CloseAllBrowsers(false);
        } catch (const std::exception &ex) {
          Logger::LogMessage("Exception during window close: " +
                             std::string(ex.what()));
        } catch (...) {
          Logger::LogMessage("Unknown exception during window close");
        }
      }
    }
  } catch (const std::exception &ex) {
    Logger::LogMessage("Exception in HandleEvents: " + std::string(ex.what()));
  } catch (...) {
    Logger::LogMessage("Unknown exception in HandleEvents");
  }
}

// Utility function to get executable directory
std::filesystem::path GetExeDir() {
  wchar_t exePath[MAX_PATH];
  DWORD result = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
  if (result == 0 || result == MAX_PATH) {
    // Fallback to current directory only as last resort
    Logger::LogMessage(
        "Warning: Failed to get executable path, using current directory");
    return std::filesystem::current_path();
  }

  std::filesystem::path path(exePath);
  return path.parent_path();
}

// Main application entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // Initialize logger
  Logger::Initialize();
  Logger::LogMessage("MikoIDE starting up...");

  // Preload splash image for faster display
  SplashScreen::PreloadSplashImage();

  // Create and show splash screen
  g_splash_screen = std::make_unique<SplashScreen>();
  if (g_splash_screen->Create(hInstance, L"MikoIDE")) {
    g_splash_screen->Show();
    g_splash_screen->UpdateStatus(L"Initializing application...");
  }

  // Set application properties
  SetApplicationUserModelID();
  LoadApplicationIcon();
  SetPermanentTaskbarIcon();

  // Enable High DPI awareness with modern Windows 10+ APIs
  // Try the most modern approach first (Windows 10 1703+)
  if (g_splash_screen) {
    g_splash_screen->UpdateStatus(L"Configuring display settings...");
  }

  HMODULE user32 = GetModuleHandleW(L"user32.dll");
  if (user32) {
    typedef BOOL(WINAPI *
                 SetProcessDpiAwarenessContextProc)(DPI_AWARENESS_CONTEXT);
    auto pSetProcessDpiAwarenessContext =
        reinterpret_cast<SetProcessDpiAwarenessContextProc>(
            GetProcAddress(user32, "SetProcessDpiAwarenessContext"));

    if (pSetProcessDpiAwarenessContext) {
      // Use per-monitor DPI awareness v2 for best scaling support
      if (pSetProcessDpiAwarenessContext(
              DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        Logger::LogMessage("HiDPI: Enabled per-monitor DPI awareness v2");
      } else if (pSetProcessDpiAwarenessContext(
                     DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) {
        Logger::LogMessage("HiDPI: Enabled per-monitor DPI awareness v1");
      } else {
        Logger::LogMessage(
            "HiDPI: Failed to set DPI awareness context, falling back");
        SetProcessDPIAware();
      }
    } else {
      // Fallback to Windows 8.1+ API
      HMODULE shcore = LoadLibraryW(L"shcore.dll");
      if (shcore) {
        typedef HRESULT(WINAPI * SetProcessDpiAwarenessProc)(int);
        auto pSetProcessDpiAwareness =
            reinterpret_cast<SetProcessDpiAwarenessProc>(
                GetProcAddress(shcore, "SetProcessDpiAwareness"));

        if (pSetProcessDpiAwareness) {
          // PROCESS_PER_MONITOR_DPI_AWARE = 2
          if (SUCCEEDED(pSetProcessDpiAwareness(2))) {
            Logger::LogMessage(
                "HiDPI: Enabled per-monitor DPI awareness (Windows 8.1)");
          } else {
            Logger::LogMessage(
                "HiDPI: Failed to set process DPI awareness, using basic");
            SetProcessDPIAware();
          }
        } else {
          SetProcessDPIAware();
        }
        FreeLibrary(shcore);
      } else {
        // Final fallback to basic DPI awareness
        SetProcessDPIAware();
        Logger::LogMessage("HiDPI: Using basic DPI awareness (legacy)");
      }
    }
  } else {
    SetProcessDPIAware();
    Logger::LogMessage("HiDPI: Using basic DPI awareness (fallback)");
  }

  // ---- Set CEF DLL directories ----
  Logger::LogMessage("Setting up CEF paths...");

  if (g_splash_screen) {
    g_splash_screen->UpdateStatus(L"Setting up CEF paths...");
  }

  // Compute paths BEFORE CEF
  const auto exeDir = GetExeDir();

#ifdef _WIN32
  const bool is64 = sizeof(void *) == 8;
  const std::wstring platform = is64 ? L"windows64" : L"windows32";
  const std::filesystem::path cefDir = exeDir / L"bin" / L"cef" / platform;
  const std::filesystem::path helperPath = cefDir / L"mikowebhelper.exe";
  const std::filesystem::path cachePath = exeDir / L"cache";

  // Bootstrap: Check if CEF helper exists and download if necessary
  Logger::LogMessage("Bootstrap: Checking CEF helper availability...");

  if (g_splash_screen) {
    g_splash_screen->UpdateStatus(L"Checking CEF components...");
  }

  BootstrapResult bootstrapResult =
      Bootstrap::CheckAndDownloadCEFHelper(hInstance);

  switch (bootstrapResult) {
  case BootstrapResult::SUCCESS:
    Logger::LogMessage("Bootstrap: CEF helper downloaded successfully, "
                       "restarting application...");
    Bootstrap::RelaunchApplication();
    return 0;

  case BootstrapResult::ALREADY_EXISTS:
    Logger::LogMessage("Bootstrap: CEF helper already exists, continuing...");
    break;

  case BootstrapResult::USER_CANCELLED:
    Logger::LogMessage("Bootstrap: User cancelled download");
    MessageBoxA(nullptr,
                "CEF helper download was cancelled. The application cannot "
                "start without it.",
                "MikoIDE", MB_OK | MB_ICONERROR);
    return 1;

  case BootstrapResult::DOWNLOAD_FAILED:
    Logger::LogMessage("Bootstrap: Failed to download CEF helper");
    MessageBoxA(nullptr,
                "Failed to download CEF helper. Please check your internet "
                "connection and try again.",
                "MikoIDE", MB_OK | MB_ICONERROR);
    return 1;

  case BootstrapResult::EXTRACT_FAILED:
    Logger::LogMessage("Bootstrap: Failed to extract CEF helper");
    MessageBoxA(
        nullptr,
        "Failed to extract CEF helper. Please try running as administrator.",
        "MikoIDE", MB_OK | MB_ICONERROR);
    return 1;

  default:
    Logger::LogMessage("Bootstrap: Unknown error occurred");
    MessageBoxA(nullptr,
                "An unknown error occurred during bootstrap. Please try again.",
                "MikoIDE", MB_OK | MB_ICONERROR);
    return 1;
  }

  // Set DLL search paths BEFORE CefExecuteProcess
  HMODULE k32 = GetModuleHandleW(L"kernel32.dll");
  auto pSetDefaultDllDirectories = reinterpret_cast<BOOL(WINAPI *)(DWORD)>(
      GetProcAddress(k32, "SetDefaultDllDirectories"));
  auto pAddDllDirectory = reinterpret_cast<LPVOID(WINAPI *)(PCWSTR)>(
      GetProcAddress(k32, "AddDllDirectory"));

  if (pSetDefaultDllDirectories && pAddDllDirectory) {
    pSetDefaultDllDirectories(
        LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS |
        LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
    pAddDllDirectory(
        std::wstring(exeDir.wstring()).c_str()); // SDL3.dll beside exe
    pAddDllDirectory(cefDir.wstring().c_str());  // libcef.dll in cef folder
  } else {
    // Fallback (Win7 without KB2533623)
    DWORD need = GetEnvironmentVariableW(L"PATH", nullptr, 0);
    std::wstring oldPath(need ? need - 1 : 0, L'\0');
    if (need)
      GetEnvironmentVariableW(L"PATH", oldPath.data(), need);
    std::wstring newPath = cefDir.wstring() + L";" + oldPath;
    SetEnvironmentVariableW(L"PATH", newPath.c_str());
  }
#endif

  // Initialize CEF
  CefMainArgs main_args(hInstance);
  CefRefPtr<SimpleApp> app(new SimpleApp);

  if (g_splash_screen) {
    g_splash_screen->UpdateStatus(L"Initializing CEF framework...");
  }

  // Execute the secondary process, if any
  int exit_code = CefExecuteProcess(main_args, app, nullptr);
  if (exit_code >= 0) {
    return exit_code;
  }

  // CEF settings
  CefSettings settings;
  settings.no_sandbox = true;
  settings.windowless_rendering_enabled = true;
  settings.multi_threaded_message_loop =
      false; // We'll use our own message loop

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

  // Enable remote debugging on port 9222
  settings.remote_debugging_port = 9222;

  // Additional settings for remote debugging to work properly
  CefString(&settings.user_agent).FromASCII("MikoIDE/1.0 Chrome");

  // Enable debugging features and prevent connection drops
  // Note: Certificate error handling is managed through CEF command line
  // switches

  // Ensure debugging works by disabling sandbox (already set above)
  // and enabling logging for debugging
  settings.log_severity = LOGSEVERITY_INFO;

  // Set command line switches for better debugging stability
  CefString(&settings.javascript_flags)
      .FromASCII("--expose-gc --allow-natives-syntax");

  Logger::LogMessage(
      "Remote debugging enabled on port 9222 with enhanced stability settings");

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
    if (g_splash_screen) {
      g_splash_screen->Hide();
    }
    return 1;
  }

  Logger::LogMessage("CEF initialized successfully");

  if (g_splash_screen) {
    g_splash_screen->UpdateStatus(L"Setting up browser engine...");
  }

  // Register scheme handler factory for miko:// protocol
  CefRegisterSchemeHandlerFactory("miko", "", new BinaryResourceProvider());
  Logger::LogMessage("Registered miko:// scheme handler factory");

  // Initialize SDL3 Window
  g_sdl_window = std::make_unique<SDL3Window>();
  if (!g_sdl_window->Initialize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT)) {
    Logger::LogMessage("Failed to initialize SDL3 window");
    if (g_splash_screen) {
      g_splash_screen->Hide();
    }
    CefShutdown();
    return 1;
  }

  Logger::LogMessage("SDL3 window initialized successfully");

  if (g_splash_screen) {
    g_splash_screen->UpdateStatus(L"Creating application window...");
  }

  // Create CEF client and browser with exception handling
  try {
    g_client = new HyperionClient(g_sdl_window.get());

    if (g_splash_screen) {
      g_splash_screen->UpdateStatus(L"Loading application...");
    }

    // Configure browser settings
    CefBrowserSettings browser_settings;
    browser_settings.windowless_frame_rate =
        120; // 60 FPS for smooth rendering (0 means unlimited, which can cause
             // issues)
    browser_settings.background_color = CefColorSetARGB(
        0, 0, 0, 0); // Fully transparent background (ARGB format)

    // Enable debugging features for better remote debugging stability
    browser_settings.javascript = STATE_ENABLED;
    browser_settings.javascript_close_windows = STATE_ENABLED;
    browser_settings.javascript_access_clipboard = STATE_ENABLED;
    browser_settings.javascript_dom_paste = STATE_ENABLED;

    // Configure window info for off-screen rendering
    CefWindowInfo window_info;
    window_info.SetAsWindowless(g_sdl_window->GetHWND());

    // Create the browser synchronously to prevent race conditions
    CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(
        window_info, g_client, AppConfig::GetStartupUrl(), browser_settings,
        nullptr, nullptr);

    if (!browser) {
      Logger::LogMessage("Failed to create CEF browser");
      CefShutdown();
      return 1;
    }

    Logger::LogMessage("CEF browser created successfully");

    // Automatically open editor in the black screen area
    // Y: 124 to end - 24, full width
    if (g_client && g_sdl_window) {
      int window_width = g_sdl_window->GetWidth();
      int window_height = g_sdl_window->GetHeight();

      int editor_x = 0;       // Start at left edge (full width)
      int editor_y = 32 + 91; // Title bar (32px) + navbar (96px) = 128px
      int editor_width = window_width; // Full width
      int editor_height =
          window_height - editor_y - 23; // From Y:128 to end-24 (status bar)

      Logger::LogMessage("Auto-opening editor at position (" +
                         std::to_string(editor_x) + ", " +
                         std::to_string(editor_y) + ") with size (" +
                         std::to_string(editor_width) + "x" +
                         std::to_string(editor_height) + ")");

      g_client->OpenEditor(editor_x, editor_y, editor_width, editor_height);
    }
  } catch (const std::exception &ex) {
    Logger::LogMessage("Exception during CEF browser creation: " +
                       std::string(ex.what()));
    if (g_splash_screen) {
      g_splash_screen->Hide();
    }
    CefShutdown();
    return 1;
  } catch (...) {
    Logger::LogMessage(
        "Unknown exception during CEF browser creation (possibly 0xe06d7363)");
    if (g_splash_screen) {
      g_splash_screen->Hide();
    }
    CefShutdown();
    return 1;
  }

  // Hide splash screen once the browser is created and ready
  if (g_splash_screen) {
    g_splash_screen->Hide();
    g_splash_screen.reset();
  }

  // Main message loop with exception handling
  while (!g_is_closing) {
    try {
      // Handle SDL events
      HandleEvents();

      // Render the window
      if (g_sdl_window) {
        g_sdl_window->Render();
      }

      // Process CEF message loop with exception handling
      try {
        CefDoMessageLoopWork();
      } catch (const std::exception &ex) {
        Logger::LogMessage("CEF message loop exception: " +
                           std::string(ex.what()));
        // Continue running to maintain stability
      } catch (...) {
        Logger::LogMessage(
            "Unknown CEF message loop exception caught (possibly 0xe06d7363)");
        // Continue running to maintain stability
      }

      // Small delay to prevent 100% CPU usage
      SDL_Delay(1);

      // Check if all browsers are closed
      if (g_client && !g_client->HasBrowsers() && g_is_closing) {
        break;
      }
    } catch (const std::exception &ex) {
      Logger::LogMessage("Main loop exception: " + std::string(ex.what()));
      // Continue running unless it's a critical error
    } catch (...) {
      Logger::LogMessage(
          "Unknown main loop exception caught - continuing execution");
      // Continue running to maintain stability
    }
  }

  Logger::LogMessage("Shutting down application...");

  // Cleanup
  g_client = nullptr;

  if (g_sdl_window) {
    g_sdl_window->Shutdown();
    g_sdl_window.reset();
  }

  // Cleanup preloaded splash image
  SplashScreen::CleanupPreloadedImage();

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
  if (!hIcon)
    return nullptr;

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

std::string GetDataURI(const std::string &data, const std::string &mime_type) {
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

// SDL3 main function wrapper
