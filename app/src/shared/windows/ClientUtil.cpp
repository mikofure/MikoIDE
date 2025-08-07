#include "shared/util/ClientUtil.hpp"

#include <string>
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include "include/cef_browser.h"

// Windows 10 version 1903 and later
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

// Windows 11 and later
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1
#define DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 19
#endif

namespace shared
{

namespace util
{

void ClientUtil::platformTitleChange(CefRefPtr<CefBrowser> browser, const CefString &title)
{
    CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
    SetWindowText(hwnd, std::wstring(title).c_str());
}

void ClientUtil::enableDarkTitlebar(CefRefPtr<CefBrowser> browser)
{
    CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
    if (hwnd) {
        BOOL darkMode = TRUE;
        
        // Try Windows 11/10 version 1903+ first
        HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
        
        // If that fails, try the older attribute for Windows 10 versions before 1903
        if (FAILED(hr)) {
            DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &darkMode, sizeof(darkMode));
        }
    }
}

} // namespace util
} // namespace shared
