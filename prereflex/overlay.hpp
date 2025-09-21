#pragma once

#include "include/cef_client.h"
#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"
#include <string>
#include <vector>

// Forward declarations
class SDL3Window;

// Overlay management functionality for menu overlays
class OverlayManager {
public:
    explicit OverlayManager(SDL3Window* window);
    ~OverlayManager() = default;

    // Menu overlay management
    void OpenMenuOverlay(const std::string& section, int x, int y, int width, int height);
    void CloseMenuOverlay();
    
    // Overlay state
    bool IsMenuOverlayActive() const { return menu_overlay_active_; }
    CefRefPtr<CefBrowser> GetMenuOverlayBrowser() const { return menu_overlay_browser_; }
    
    // Overlay content generation
    std::string GetMenuOverlayHTML(const std::string& section);
    std::string BuildOverlayURL(const std::string& section, int x, int y, int width, int height);
    
    // Browser tracking for overlay management
    void SetBrowserList(const std::vector<CefRefPtr<CefBrowser>>& browsers) { browser_list_ = browsers; }
    void SetMenuOverlayBrowser(CefRefPtr<CefBrowser> browser) { menu_overlay_browser_ = browser; }
    void ResetMenuOverlayTracking();

private:
    SDL3Window* window_;
    
    // Menu overlay tracking to prevent duplicates
    bool menu_overlay_active_;
    CefRefPtr<CefBrowser> menu_overlay_browser_;
    
    // Browser list reference for overlay operations
    std::vector<CefRefPtr<CefBrowser>> browser_list_;
};