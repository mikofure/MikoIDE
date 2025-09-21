#pragma once

// Windows includes for dwmapi
#include <windows.h>
#include <dwmapi.h>

// SDL3 includes
#include <SDL3/SDL.h>

// CEF includes
#include "include/cef_client.h"
#include "include/cef_browser.h"
#include "include/cef_render_handler.h"

#include "../renderer/dx11_renderer.hpp"
#include <memory>
#include <string>
#include <mutex>

// Forward declarations
class SimpleClient;

// SDL3 Window wrapper with CEF OSR integration and dwmapi rounded edges
class SDL3Window {
public:
    SDL3Window();
    ~SDL3Window();

    bool Initialize(int width, int height);
    void Shutdown();
    
    // Window management
    void Show();
    void Hide();
    void Minimize();
    void Maximize();
    void Restore();
    void Close();
    
    // Rounded corners using dwmapi
    void ApplyRoundedCorners();
    void SetBorderless(bool borderless);
    
    // Event handling
    bool HandleEvent(const SDL_Event& event);
    bool HandleWindowDragging(const SDL_Event& event);
    void Render();
    
    // Getters
    SDL_Window* GetSDLWindow() const { return window_; }
    SDL_Renderer* GetRenderer() const { return renderer_; }
    SDL_Texture* GetTexture() const { return texture_; }
    HWND GetHWND() const;
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    
    // CEF OSR integration
    void UpdateTexture(const void* buffer, int width, int height);
    void Resize(int width, int height);
    void SetClient(CefRefPtr<SimpleClient> client) { client_ = client; }
    
    // Menu overlay management
    void ShowMenuOverlay(const std::string& section, int x, int y);
    void HideMenuOverlay();
    bool IsMenuOverlayVisible() const;
    void ResizeMenuOverlay(int height);
    
    // Menu overlay position and state setters
    void SetMenuOverlayPosition(int x, int y);
    void SetMenuOverlayVisible(bool visible);
    void SetCurrentMenuSection(const std::string& section);
    
    // Menu overlay position and state getters
    int GetMenuOverlayX() const { return menu_overlay_x_; }
    int GetMenuOverlayY() const { return menu_overlay_y_; }
    std::string GetCurrentMenuSection() const { return current_menu_section_; }
    
    // Editor sublayer management
    void EnableEditor(bool enable);
    void SetEditorPosition(int x, int y, int width, int height);
    void SetEditorBrowser(CefRefPtr<CefBrowser> browser);
    bool IsEditorEnabled() const { return editor_enabled_; }
    CefRect GetEditorRect() const { return editor_rect_; }
    CefRefPtr<CefBrowser> GetEditorBrowser() const { return editor_browser_; }
    void UpdateEditorTexture(const void* buffer, int width, int height);
    
    // Thread safety for editor texture updates
    std::mutex editor_texture_mutex_;
    
    // DX11 Renderer integration
    bool IsDX11Available() const { return dx11_renderer_ && dx11_renderer_->IsInitialized(); }
    void EnableDX11Rendering(bool enable);
    DX11Renderer* GetDX11Renderer() const { return dx11_renderer_.get(); }
    
    // Window state
    bool IsMinimized() const { return minimized_; }
    bool IsMaximized() const { return maximized_; }
    bool ShouldClose() const { return should_close_; }
    
    // DPI and scaling support
    float GetDPIScale() const { return dpi_scale_; }
    void UpdateDPIScale();
    int GetScaledWidth() const { return static_cast<int>(width_ * dpi_scale_); }
    int GetScaledHeight() const { return static_cast<int>(height_ * dpi_scale_); }
    
    // Mouse and keyboard input for CEF
    void SendMouseEvent(const SDL_Event& event);
    void SendKeyEvent(const SDL_Event& event);
    void SendScrollEvent(const SDL_Event& event);

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
    HWND hwnd_;
    
    CefRefPtr<SimpleClient> client_;
    
    int width_;
    int height_;
    bool minimized_;
    bool maximized_;
    bool should_close_;
    bool borderless_;
    
    // Mouse state
    bool mouse_captured_;
    int last_mouse_x_;
    int last_mouse_y_;
    
    // Window dragging state
    bool is_dragging_;
    int drag_start_x_;
    int drag_start_y_;
    int window_start_x_;
    int window_start_y_;
    
    // DX11 Renderer for performance optimization
    std::unique_ptr<DX11Renderer> dx11_renderer_;
    bool dx11_enabled_;
    
    // DPI scaling support
    float dpi_scale_;
    
    // Menu overlay state
    bool menu_overlay_visible_;
    std::string current_menu_section_;
    int menu_overlay_x_;
    int menu_overlay_y_;
    
    // Editor sublayer management
    bool editor_enabled_;
    CefRect editor_rect_;
    CefRefPtr<CefBrowser> editor_browser_;
    SDL_Texture* editor_texture_;
    bool editor_texture_locked_;
    
    void InitializeDwmApi();
    void UpdateWindowStyle();
};

// Default window dimensions
constexpr int DEFAULT_WINDOW_WIDTH = 1200;
constexpr int DEFAULT_WINDOW_HEIGHT = 800;