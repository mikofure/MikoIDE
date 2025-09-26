#include "ui_factory.hpp"
#include "ui_interface.hpp"

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#include "platform/windows/windows_splash.hpp"
#include "platform/windows/windows_dialog.hpp"
#pragma comment(lib, "comctl32.lib")
#elif defined(__linux__)
#include <gtk/gtk.h>
#include "platform/linux/linux_splash.hpp"
#include "platform/linux/linux_dialog.hpp"
#endif

// ISplashScreen factory method
std::unique_ptr<ISplashScreen> ISplashScreen::Create() {
#ifdef _WIN32
    return std::make_unique<WindowsSplashScreen>();
#elif defined(__linux__)
    return std::make_unique<LinuxSplashScreen>();
#else
    return nullptr;
#endif
}

// IModernDialog factory method
std::unique_ptr<IModernDialog> IModernDialog::Create() {
#ifdef _WIN32
    return std::make_unique<WindowsModernDialog>();
#elif defined(__linux__)
    return std::make_unique<LinuxModernDialog>();
#else
    return nullptr;
#endif
}

// UIFactory implementation
std::unique_ptr<ISplashScreen> UIFactory::CreateSplashScreen() {
    return ISplashScreen::Create();
}

std::unique_ptr<IModernDialog> UIFactory::CreateModernDialog() {
    return IModernDialog::Create();
}

bool UIFactory::InitializePlatform() {
#ifdef _WIN32
    // Initialize COM for Windows
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        return false;
    }
    
    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES | ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);
    
    return true;
#elif defined(__linux__)
    // Initialize GTK
    return gtk_init_check(nullptr, nullptr);
#else
    return false;
#endif
}

void UIFactory::ShutdownPlatform() {
#ifdef _WIN32
    CoUninitialize();
#elif defined(__linux__)
    // GTK cleanup is handled automatically
#endif
}