#include "shared/util/BrowserWindowDelegate.hpp"

#if defined(OS_WIN)
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

// Windows 10 version 1903 and later
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

// Windows 11 and later
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1
#define DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 19
#endif
#endif

namespace shared
{

namespace util
{

void BrowserWindowDelegate::OnWindowCreated(CefRefPtr<CefWindow> window)
{
    // add the browser view and show the window
    window->AddChildView(browserView);
    
#if defined(OS_WIN)
    // Enable dark titlebar on Windows
    HWND hwnd = window->GetWindowHandle();
    if (hwnd) {
        BOOL darkMode = TRUE;
        
        // Try Windows 11/10 version 1903+ first
        HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
        
        // If that fails, try the older attribute for Windows 10 versions before 1903
        if (FAILED(hr)) {
            DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &darkMode, sizeof(darkMode));
        }
    }
#endif
    
    window->Show();

    // give keyboard focus to the browser view
    browserView->RequestFocus();
}

void BrowserWindowDelegate::OnWindowDestroyed(CefRefPtr<CefWindow> window)
{
    browserView = nullptr;
}

bool BrowserWindowDelegate::CanClose(CefRefPtr<CefWindow> window)
{
    // allow the window to close if the browser says it's OK
    CefRefPtr<CefBrowser> browser = browserView->GetBrowser();

    if (browser)
    {
        return browser->GetHost()->TryCloseBrowser();
    }

    return true;
}

CefSize BrowserWindowDelegate::GetPreferredSize(CefRefPtr<CefView> view)
{
    // preferred window size
    return CefSize(800, 600);
}

CefSize BrowserWindowDelegate::GetMinimumSize(CefRefPtr<CefView> view)
{
    // minimum window size
    return CefSize(200, 100);
}

} // namespace util
} // namespace shared
