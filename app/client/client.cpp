#include "../client/client.hpp"
#include "../utils/config.hpp"
#include "../utils/logger.hpp"
#include "../internal/simpleipc.hpp"
#include "../resources/resourceutil.hpp"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_app.h"
#include <SDL3/SDL.h>
#include <dwmapi.h>
#include <shellapi.h>

// Link dwmapi library
#pragma comment(lib, "dwmapi.lib")

// CloseBrowserTask implementation
CloseBrowserTask::CloseBrowserTask(CefRefPtr<SimpleClient> client, bool force_close)
    : client_(client), force_close_(force_close) {
}

void CloseBrowserTask::Execute() {
    client_->DoCloseAllBrowsers(force_close_);
}

// SDL3Window implementation
SDL3Window::SDL3Window()
    : window_(nullptr)
    , renderer_(nullptr)
    , texture_(nullptr)
    , hwnd_(nullptr)
    , client_(nullptr)
    , width_(DEFAULT_WINDOW_WIDTH)
, height_(DEFAULT_WINDOW_HEIGHT)
    , minimized_(false)
    , maximized_(false)
    , should_close_(false)
    , borderless_(true)
    , mouse_captured_(false)
    , last_mouse_x_(0)
    , last_mouse_y_(0)
    , is_dragging_(false)
    , drag_start_x_(0)
    , drag_start_y_(0)
    , window_start_x_(0)
    , window_start_y_(0)
    , dx11_renderer_(nullptr)
    , dx11_enabled_(false)
    , dpi_scale_(1.0f)
    , menu_overlay_visible_(false)
    , menu_overlay_x_(0)
    , menu_overlay_y_(0)
    , editor_enabled_(false)
    , editor_rect_({0, 0, 0, 0})
    , editor_browser_(nullptr)
    , editor_texture_(nullptr) {
}

SDL3Window::~SDL3Window() {
    Shutdown();
}

bool SDL3Window::Initialize(int width, int height) {
    width_ = width;
    height_ = height;

    // Initialize SDL3 with video subsystem directly
    SDL_ClearError();
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return false;
    }

    // Create borderless window
    Uint32 window_flags = SDL_WINDOW_RESIZABLE;
    if (borderless_) {
        window_flags |= SDL_WINDOW_BORDERLESS;
    }

    window_ = SDL_CreateWindow(
        "MikoIDE",
        width_, height_,
        window_flags
    );

    if (!window_) {
        SDL_Quit();
        return false;
    }

    // Create renderer (SDL3 uses different flags)
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        SDL_DestroyWindow(window_);
        SDL_Quit();
        return false;
    }

    // Create texture for CEF rendering
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, width_, height_);
    if (!texture_) {
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        return false;
    }
    // IMPORTANT: Avoid blending (OSR may deliver alpha=0 everywhere)
    SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_NONE);

    // Get Windows HWND for dwmapi
    hwnd_ = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window_), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

    // Initialize dwmapi and apply rounded corners
    InitializeDwmApi();
    ApplyRoundedCorners();
    UpdateWindowStyle();

    // Initialize DX11 renderer for performance boost
    dx11_renderer_ = std::make_unique<DX11Renderer>();
    if (dx11_renderer_->Initialize(hwnd_, width_, height_)) {
        dx11_enabled_ = true;
    } else {
        dx11_enabled_ = false;
        dx11_renderer_.reset();
    }

    // Initialize DPI scaling
    UpdateDPIScale();

    return true;
}

void SDL3Window::Shutdown() {
    // Cleanup DX11 renderer first
    if (dx11_renderer_) {
        dx11_renderer_->Shutdown();
        dx11_renderer_.reset();
        dx11_enabled_ = false;
    }
    
    if (editor_texture_) {
        SDL_DestroyTexture(editor_texture_);
        editor_texture_ = nullptr;
    }
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    SDL_Quit();
}

void SDL3Window::Show() {
    if (window_) {
        SDL_ShowWindow(window_);
    }
}

void SDL3Window::Hide() {
    if (window_) {
        SDL_HideWindow(window_);
    }
}

void SDL3Window::Minimize() {
    if (window_) {
        SDL_MinimizeWindow(window_);
        minimized_ = true;
        maximized_ = false;
    }
}

void SDL3Window::Maximize() {
    if (window_) {
        SDL_MaximizeWindow(window_);
        maximized_ = true;
        minimized_ = false;
    }
}

void SDL3Window::Restore() {
    if (window_) {
        SDL_RestoreWindow(window_);
        minimized_ = false;
        maximized_ = false;
    }
}

void SDL3Window::Close() {
    should_close_ = true;
}

HWND SDL3Window::GetHWND() const {
    return hwnd_;
}

void SDL3Window::ApplyRoundedCorners() {
    if (!hwnd_) return;

    // Apply rounded corners using DWM API (Windows 11 style)
    DWM_WINDOW_CORNER_PREFERENCE cornerPreference = DWMWCP_ROUND;
    DwmSetWindowAttribute(hwnd_, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPreference, sizeof(cornerPreference));

    // Enable immersive dark mode if available
    BOOL darkMode = TRUE;
    DwmSetWindowAttribute(hwnd_, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

    Logger::LogMessage("Applied rounded corners and dark mode to window");
}

void SDL3Window::SetBorderless(bool borderless) {
    borderless_ = borderless;
    UpdateWindowStyle();
}

void SDL3Window::UpdateWindowStyle() {
    if (!hwnd_) return;

    LONG_PTR style = GetWindowLongPtr(hwnd_, GWL_STYLE);
    if (borderless_) {
        style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
        style |= WS_POPUP;
    } else {
        style |= (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
        style &= ~WS_POPUP;
    }
    SetWindowLongPtr(hwnd_, GWL_STYLE, style);

    // Update extended style for proper taskbar appearance
    LONG_PTR exStyle = GetWindowLongPtr(hwnd_, GWL_EXSTYLE);
    exStyle |= WS_EX_APPWINDOW;
    SetWindowLongPtr(hwnd_, GWL_EXSTYLE, exStyle);

    // Force window to redraw
    SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0, 
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}

void SDL3Window::InitializeDwmApi() {
    if (!hwnd_) return;

    // Check DWM composition status (always enabled on Windows 10/11)
    BOOL composition_enabled = FALSE;
    DwmIsCompositionEnabled(&composition_enabled);

    // Extend frame into client area for better visual effects
    MARGINS margins = { 0, 0, 0, 1 };
    DwmExtendFrameIntoClientArea(hwnd_, &margins);
}

void SDL3Window::UpdateDPIScale() {
    if (!hwnd_) {
        dpi_scale_ = 1.0f;
        return;
    }

    // Get DPI for the window's monitor
    UINT dpi = GetDpiForWindow(hwnd_);
    dpi_scale_ = static_cast<float>(dpi) / 96.0f; // 96 DPI is the standard baseline
    
    Logger::LogMessage("HiDPI: Detected DPI scale factor: " + std::to_string(dpi_scale_) + 
                      " (DPI: " + std::to_string(dpi) + ")");
    
    // Update CEF browser size if client exists
    if (client_) {
        auto browser = client_->GetFirstBrowser();
        if (browser && browser->GetHost()) {
            // Notify CEF about the DPI change
            browser->GetHost()->NotifyScreenInfoChanged();
            browser->GetHost()->WasResized();
        }
    }
}

bool SDL3Window::HandleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_QUIT:
            should_close_ = true;
            return true;

        case SDL_EVENT_WINDOW_MINIMIZED:
            minimized_ = true;
            maximized_ = false;
            return true;

        case SDL_EVENT_WINDOW_MAXIMIZED:
            maximized_ = true;
            minimized_ = false;
            
            // Update editor size to match maximized window
            if (editor_enabled_ && editor_browser_) {
                // Calculate new editor dimensions based on maximized window size
                int new_width = width_;
                int new_height = height_;
                
                // Account for UI elements: title bar (32px) + navbar (96px) + status bar (24px)
                int margin_x = 0;
                int margin_y = 32 + 96;  // Title bar + navbar at top
                int margin_bottom = 24;  // Status bar at bottom
                
                SetEditorPosition(margin_x, margin_y, new_width - margin_x, new_height - margin_y - margin_bottom);
                
                // Notify editor browser of size change
                editor_browser_->GetHost()->WasResized();
                
                Logger::LogMessage("Maximized: Updated editor size to " + 
                                 std::to_string(new_width - margin_x) + "x" + 
                                 std::to_string(new_height - margin_y - margin_bottom));
            }
            return true;

        case SDL_EVENT_WINDOW_RESTORED:
            minimized_ = false;
            maximized_ = false;
            
            // Update editor size to match restored window
            if (editor_enabled_ && editor_browser_) {
                // Calculate new editor dimensions based on restored window size
                // Account for UI elements: title bar (32px) + navbar (96px) + status bar (24px)
                int margin_x = 0;
                int margin_y = 32 + 91;  // Title bar + navbar at top
                int margin_bottom = 23;  // Status bar at bottom
                
                SetEditorPosition(margin_x, margin_y, width_ - margin_x, height_ - margin_y - margin_bottom);
                
                // Notify editor browser of size change
                editor_browser_->GetHost()->WasResized();
                
                Logger::LogMessage("Restored: Updated editor size to " + 
                                 std::to_string(width_ - margin_x) + "x" + 
                                 std::to_string(height_ - margin_y - margin_bottom));
            }
            return true;

        case SDL_EVENT_WINDOW_RESIZED:
            width_ = event.window.data1;
            height_ = event.window.data2;
            
            Logger::LogMessage("Window resized to " + std::to_string(width_) + "x" + std::to_string(height_));
            
            // Check for DPI changes on resize (common trigger for DPI changes)
            UpdateDPIScale();
            
            // Recreate texture with new size
            if (texture_) {
                SDL_DestroyTexture(texture_);
                texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32, 
                                           SDL_TEXTUREACCESS_STREAMING, width_, height_);
                if (!texture_) {
                    Logger::LogMessage("Resize: Failed to recreate texture - " + std::string(SDL_GetError()));
                } else {
                    SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_NONE);
                    Logger::LogMessage("Resize: Texture recreated successfully");
                }
            }
            
            // Update editor size to match new window dimensions
            if (editor_enabled_ && editor_browser_) {
                // Calculate new editor dimensions based on resized window
                // Account for UI elements: title bar (32px) + navbar (96px) + status bar (24px)
                int margin_x = 0;
                int margin_y = 32 + 91;  // Title bar + navbar at top
                int margin_bottom = 23;  // Status bar at bottom
                
                SetEditorPosition(margin_x, margin_y, width_ - margin_x, height_ - margin_y - margin_bottom);
                
                // Notify editor browser of size change
                editor_browser_->GetHost()->WasResized();
                
                Logger::LogMessage("Resized: Updated editor size to " + 
                                 std::to_string(width_ - margin_x) + "x" + 
                                 std::to_string(height_ - margin_y - margin_bottom));
            }
            
            // Notify CEF browser of size change
            if (client_) {
                auto browser = client_->GetFirstBrowser();
                if (browser) {
                    browser->GetHost()->WasResized();
                    Logger::LogMessage("Resize: Notified CEF browser of size change");
                }
            }
            return true;

        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            // Handle display change events (monitor switching)
            Logger::LogMessage("Display changed event detected");
            UpdateDPIScale();
            
            // Notify CEF browser of potential size/scale change
            if (client_) {
                auto browser = client_->GetFirstBrowser();
                if (browser) {
                    browser->GetHost()->WasResized();
                    Logger::LogMessage("Display change: Notified CEF browser of scale change");
                }
            }
            return true;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_MOTION:
            // Handle window dragging first
            if (HandleWindowDragging(event)) {
                return true;
            }
            // Only send to CEF if the event wasn't consumed by dragging
            try {
                SendMouseEvent(event);
            } catch (const std::exception& ex) {
                Logger::LogMessage("HandleEvent: Exception in SendMouseEvent - " + std::string(ex.what()));
            } catch (...) {
                Logger::LogMessage("HandleEvent: Unknown exception in SendMouseEvent");
            }
            return true;

        case SDL_EVENT_MOUSE_WHEEL:
            SendScrollEvent(event);
            return true;

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            SendKeyEvent(event);
            return true;

        default:
            return false;
    }
}

void SDL3Window::SendMouseEvent(const SDL_Event& event) {
    if (!client_) {
        Logger::LogMessage("SendMouseEvent: Client is null");
        return;
    }

    auto browser = client_->GetFirstBrowser();
    if (!browser) {
        Logger::LogMessage("SendMouseEvent: Browser is null");
        return;
    }

    auto host = browser->GetHost();
    if (!host) {
        Logger::LogMessage("SendMouseEvent: Browser host is null");
        return;
    }

    CefMouseEvent mouse_event;
    mouse_event.x = event.button.x;
    mouse_event.y = event.button.y;
    mouse_event.modifiers = 0; // TODO: Handle modifiers

    // Validate coordinates are within reasonable bounds
    if (mouse_event.x < 0 || mouse_event.y < 0 || 
        mouse_event.x > width_ || mouse_event.y > height_) {
        Logger::LogMessage("SendMouseEvent: Mouse coordinates out of bounds (" + 
                          std::to_string(mouse_event.x) + ", " + std::to_string(mouse_event.y) + ")");
        return;
    }

    last_mouse_x_ = event.button.x;
    last_mouse_y_ = event.button.y;

    try {
        switch (event.type) {
            case SDL_EVENT_MOUSE_MOTION:
                host->SendMouseMoveEvent(mouse_event, false);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                CefBrowserHost::MouseButtonType button_type = MBT_LEFT;
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    button_type = MBT_RIGHT;
                } else if (event.button.button == SDL_BUTTON_MIDDLE) {
                    button_type = MBT_MIDDLE;
                }

                bool mouse_up = (event.type == SDL_EVENT_MOUSE_BUTTON_UP);
                int click_count = event.button.clicks;
                
                Logger::LogMessage("SendMouseEvent: Sending click event at (" + 
                                  std::to_string(mouse_event.x) + ", " + std::to_string(mouse_event.y) + 
                                  ") button=" + std::to_string(button_type) + " up=" + std::to_string(mouse_up));
                
                host->SendMouseClickEvent(mouse_event, button_type, mouse_up, click_count);
                break;
            }
        }
    } catch (const std::exception& ex) {
        Logger::LogMessage("SendMouseEvent: Exception caught - " + std::string(ex.what()));
    } catch (...) {
        Logger::LogMessage("SendMouseEvent: Unknown exception caught");
    }
}

void SDL3Window::SendKeyEvent(const SDL_Event& event) {
    if (!client_) return;

    auto browser = client_->GetFirstBrowser();
    if (!browser) return;

    CefKeyEvent key_event;
    key_event.windows_key_code = event.key.key;
    key_event.native_key_code = event.key.scancode;
    key_event.modifiers = 0; // TODO: Handle modifiers properly

    if (event.type == SDL_EVENT_KEY_DOWN) {
        key_event.type = KEYEVENT_KEYDOWN;
    } else {
        key_event.type = KEYEVENT_KEYUP;
    }

    browser->GetHost()->SendKeyEvent(key_event);
}

void SDL3Window::SendScrollEvent(const SDL_Event& event) {
    if (!client_) return;

    auto browser = client_->GetFirstBrowser();
    if (!browser) return;

    CefMouseEvent mouse_event;
    mouse_event.x = last_mouse_x_;
    mouse_event.y = last_mouse_y_;
    mouse_event.modifiers = 0;

    int delta_x = 0;
    int delta_y = static_cast<int>(event.wheel.y * 120); // Standard wheel delta

    browser->GetHost()->SendMouseWheelEvent(mouse_event, delta_x, delta_y);
}

void SDL3Window::UpdateTexture(const void* buffer, int width, int height) {
    if (!buffer) {
        return;
    }

    // Try DX11 texture update first if available and enabled
    if (dx11_enabled_ && dx11_renderer_) {
        if (dx11_renderer_->UpdateTexture(buffer, width, height)) {
            return;
        } else {
            // Fall back to SDL3 texture update
        }
    }

    // SDL3 fallback texture update
    if (!renderer_) {
        return;
    }

    // Check if we need to create or recreate the texture
    bool needNewTexture = false;
    
    if (!texture_) {
        needNewTexture = true;
    } else {
        // Check if texture dimensions match the incoming data (use float types for SDL3)
        float tex_width = 0.0f, tex_height = 0.0f;
        if (SDL_GetTextureSize(texture_, &tex_width, &tex_height) != 0) {
            // Assume we need a new texture if we can't get the size
            needNewTexture = true;
        } else if ((int)tex_width != width || (int)tex_height != height) {
            needNewTexture = true;
        }
    }

    // Create or recreate texture if needed
    if (needNewTexture) {
        if (texture_) {
            SDL_DestroyTexture(texture_);
            texture_ = nullptr;
        }
        
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, width, height);
        
        if (!texture_) {
            return;
        }
        // IMPORTANT: Use NONE blend mode for main texture to avoid interference with editor overlay
        SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_NONE);
    }

    // 🔑 Copy CEF buffer → texture
    void* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(texture_, nullptr, &pixels, &pitch)) {
        const uint8_t* src = static_cast<const uint8_t*>(buffer);
        const int rowBytes = width * 4; // BGRA32
        for (int y = 0; y < height; y++) {
            std::memcpy(static_cast<uint8_t*>(pixels) + y * pitch,
                        src + y * rowBytes,
                        rowBytes);
        }
        SDL_UnlockTexture(texture_);
    }
}

void SDL3Window::Resize(int width, int height) {
    width_ = width;
    height_ = height;

    // Recreate texture with new dimensions
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }

    // Notify CEF browser of size change
    if (client_ && client_->GetFirstBrowser()) {
        client_->GetFirstBrowser()->GetHost()->WasResized();
    }

    // Trigger display change event
    if (client_ && client_->GetFirstBrowser()) {
        client_->GetFirstBrowser()->GetHost()->NotifyScreenInfoChanged();
    }
}

void SDL3Window::Render() {
    // 1. Main render (DX11 or SDL)
    bool main_rendered = false;

    if (dx11_enabled_ && dx11_renderer_) {
        if (dx11_renderer_->Render()) {
            main_rendered = true;
        }
    }

    if (!main_rendered) {
        if (!renderer_ || !texture_) {
            return;
        }

        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);

        if (SDL_RenderTexture(renderer_, texture_, nullptr, nullptr) != 0) {
            SDL_FRect destRect = {0, 0, (float)width_, (float)height_};
            SDL_RenderTexture(renderer_, texture_, nullptr, &destRect);
        }
    }

    // 2. Editor overlay (always try on top)
    if (editor_enabled_ && editor_texture_) {
        // Ensure editor texture has proper alpha blending enabled
        SDL_SetTextureBlendMode(editor_texture_, SDL_BLENDMODE_BLEND);

        SDL_FRect editorDestRect = {
            (float)editor_rect_.x,
            (float)editor_rect_.y,
            (float)editor_rect_.width,
            (float)editor_rect_.height
        };

        SDL_RenderTexture(renderer_, editor_texture_, nullptr, &editorDestRect);
    }

    // 3. Present final frame
    if (renderer_) {
        SDL_RenderPresent(renderer_);
    }
}

// OSRRenderHandler implementation
OSRRenderHandler::OSRRenderHandler(SDL3Window* window) : window_(window) {
}

void OSRRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
    if (window_) {
        // Check if this is the editor browser by URL
        std::string url = browser->GetMainFrame()->GetURL().ToString();
        if (url.find("miko://monaco/") == 0) {
            // This is the editor browser - use editor rect dimensions
            CefRect editor_rect = window_->GetEditorRect();
            if (editor_rect.width > 0 && editor_rect.height > 0) {
                rect.x = 0;
                rect.y = 0;
                rect.width = editor_rect.width;
                rect.height = editor_rect.height;
                Logger::LogMessage("GetViewRect: Editor browser using editor rect " + 
                                 std::to_string(rect.width) + "x" + std::to_string(rect.height));
                return;
            } else {
                Logger::LogMessage("GetViewRect: Editor rect has invalid dimensions, using default");
            }
        }
        
        // For main browser or if editor rect is invalid, use full window size
        rect.x = 0;
        rect.y = 0;
        rect.width = DEFAULT_WINDOW_WIDTH; // Default width
        rect.height = DEFAULT_WINDOW_HEIGHT; // Default height
        
        // Get actual window size and apply DPI scaling
        if (window_->GetSDLWindow()) {
            int w, h;
            SDL_GetWindowSize(window_->GetSDLWindow(), &w, &h);
            
            // Apply DPI scaling to get logical size for CEF
            float dpi_scale = window_->GetDPIScale();
            rect.width = static_cast<int>(w / dpi_scale);
            rect.height = static_cast<int>(h / dpi_scale);
            
            Logger::LogMessage("GetViewRect: Main browser - Physical size " + std::to_string(w) + "x" + std::to_string(h) + 
                             ", DPI scale " + std::to_string(dpi_scale) + 
                             ", Logical size " + std::to_string(rect.width) + "x" + std::to_string(rect.height));
        } else {
            Logger::LogMessage("GetViewRect: Using default size " + std::to_string(rect.width) + "x" + std::to_string(rect.height));
        }
    } else {
        Logger::LogMessage("GetViewRect: Window is null, using default " + std::to_string(DEFAULT_WINDOW_WIDTH) + "x" + std::to_string(DEFAULT_WINDOW_HEIGHT));
        rect.x = 0;
        rect.y = 0;
        rect.width = DEFAULT_WINDOW_WIDTH;
        rect.height = DEFAULT_WINDOW_HEIGHT;
    }
}

void OSRRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                              PaintElementType type,
                              const RectList& dirtyRects,
                              const void* buffer,
                              int width,
                              int height) {
    if (type != PET_VIEW || !window_ || !buffer) {
        Logger::LogMessage("OnPaint: Invalid parameters - type=" + std::to_string(type) + ", window=" + (window_ ? "valid" : "null") + ", buffer=" + (buffer ? "valid" : "null"));
        return;
    }

    // Check if this is the editor browser by URL
    std::string url = browser->GetMainFrame()->GetURL().ToString();
    if (url.find("miko://monaco/") == 0) {
        // This is the editor browser - route to editor texture
        window_->UpdateEditorTexture(buffer, width, height);
    } else {
        // This is the main browser - route to main texture
        window_->UpdateTexture(buffer, width, height);
    }
}

bool SimpleClient::OnCursorChange(CefRefPtr<CefBrowser> browser,
                                 CefCursorHandle cursor,
                                 cef_cursor_type_t type,
                                 const CefCursorInfo& custom_cursor_info) {
    if (window_) {
        SDL_SystemCursor cursor_id = SDL_SYSTEM_CURSOR_DEFAULT;
        
        switch (type) {
            case 0: // CT_POINTER
                cursor_id = SDL_SYSTEM_CURSOR_DEFAULT;
                break;
            case 1: // CT_CROSS
                cursor_id = SDL_SYSTEM_CURSOR_CROSSHAIR;
                break;
            case 2: // CT_HAND
                cursor_id = SDL_SYSTEM_CURSOR_POINTER;
                break;
            case 3: // CT_IBEAM
                cursor_id = SDL_SYSTEM_CURSOR_TEXT;
                break;
            case 4: // CT_WAIT
                cursor_id = SDL_SYSTEM_CURSOR_WAIT;
                break;
            case 15: // CT_EWRESIZE (EastWestResize)
                cursor_id = SDL_SYSTEM_CURSOR_EW_RESIZE;
                break;
            case 17: // CT_NSRESIZE (NorthSouthResize)
                cursor_id = SDL_SYSTEM_CURSOR_NS_RESIZE;
                break;
            case 18: // CT_NWSERESIZE (NorthwestSoutheastResize)
                cursor_id = SDL_SYSTEM_CURSOR_NWSE_RESIZE;
                break;
            case 19: // CT_NESWRESIZE (NortheastSouthwestResize)
                cursor_id = SDL_SYSTEM_CURSOR_NESW_RESIZE;
                break;
            default:
                cursor_id = SDL_SYSTEM_CURSOR_DEFAULT;
                break;
        }
        
        SDL_Cursor* sdl_cursor = SDL_CreateSystemCursor(cursor_id);
        if (sdl_cursor) {
            SDL_SetCursor(sdl_cursor);
        }
    }
    return true;
}

bool OSRRenderHandler::StartDragging(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefDragData> drag_data,
                                    CefRenderHandler::DragOperationsMask allowed_ops,
                                    int x, int y) {
    // Handle drag and drop operations
    return false; // Let CEF handle it
}

void OSRRenderHandler::UpdateDragCursor(CefRefPtr<CefBrowser> browser,
                                       CefRenderHandler::DragOperation operation) {
    // Update cursor during drag operations
}

// SimpleClient implementation
SimpleClient::SimpleClient(SDL3Window* window) : window_(window), menu_overlay_active_(false), menu_overlay_browser_(nullptr) {
    // Create message router for JavaScript-to-C++ communication
    CefMessageRouterConfig config;
    message_router_ = CefMessageRouterBrowserSide::Create(config);
    message_router_->AddHandler(this, false);
    
    // Create the binary resource provider for miko:// protocol
    resource_provider_ = new BinaryResourceProvider();
    
    // Create OSR render handler
    render_handler_ = new OSRRenderHandler(window_);
    
    // Set client reference in window
    if (window_) {
        window_->SetClient(this);
    }
}

CefRefPtr<CefDisplayHandler> SimpleClient::GetDisplayHandler() {
    return this;
}

CefRefPtr<CefLifeSpanHandler> SimpleClient::GetLifeSpanHandler() {
    return this;
}

CefRefPtr<CefLoadHandler> SimpleClient::GetLoadHandler() {
    return this;
}

CefRefPtr<CefContextMenuHandler> SimpleClient::GetContextMenuHandler() {
    return this;
}

CefRefPtr<CefDragHandler> SimpleClient::GetDragHandler() {
    return this;
}

CefRefPtr<CefRequestHandler> SimpleClient::GetRequestHandler() {
    return this;
}

CefRefPtr<CefKeyboardHandler> SimpleClient::GetKeyboardHandler() {
    return this;
}

CefRefPtr<CefDownloadHandler> SimpleClient::GetDownloadHandler() {
    return this;
}

CefRefPtr<CefRenderHandler> SimpleClient::GetRenderHandler() {
    return render_handler_;
}

bool SimpleClient::OnQuery(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          int64_t query_id,
                          const CefString& request,
                          bool persistent,
                          CefRefPtr<CefMessageRouterBrowserSide::Callback> callback) {
    CEF_REQUIRE_UI_THREAD();
    
    std::string request_str = request.ToString();
    
    // Handle JSON-based menu overlay requests
    if (request_str.find("{") == 0) {
        try {
            // Parse JSON request
            size_t type_pos = request_str.find("\"type\":");
            if (type_pos != std::string::npos) {
                size_t type_start = request_str.find("\"", type_pos + 7) + 1;
                size_t type_end = request_str.find("\"", type_start);
                std::string type = request_str.substr(type_start, type_end - type_start);
                
                if (type == "open_menu_overlay") {
                    Logger::LogMessage("DEBUG: Starting open_menu_overlay handler");
                    Logger::LogMessage("DEBUG: Request string: " + request_str);
                    
                    try {
                        Logger::LogMessage("DEBUG: Extracting JSON fields");
                        // Extract section, x, y from JSON
                        size_t section_pos = request_str.find("\"section\":");
                        size_t x_pos = request_str.find("\"x\":");
                        size_t y_pos = request_str.find("\"y\":");
                        
                        Logger::LogMessage("DEBUG: Found positions - section: " + std::to_string(section_pos) + 
                                         ", x: " + std::to_string(x_pos) + ", y: " + std::to_string(y_pos));
                        
                        if (section_pos != std::string::npos && x_pos != std::string::npos && y_pos != std::string::npos) {
                            Logger::LogMessage("DEBUG: Extracting section");
                            // Extract section
                            size_t section_start = request_str.find("\"", section_pos + 10) + 1;
                            size_t section_end = request_str.find("\"", section_start);
                            std::string section = request_str.substr(section_start, section_end - section_start);
                            Logger::LogMessage("DEBUG: Extracted section: " + section);
                            
                            Logger::LogMessage("DEBUG: Extracting x coordinate");
                            // Extract x coordinate (handle numeric values without quotes)
                            size_t x_colon = request_str.find(":", x_pos);
                            size_t x_start = x_colon + 1;
                            // Skip any whitespace
                            while (x_start < request_str.length() && (request_str[x_start] == ' ' || request_str[x_start] == '\t')) {
                                x_start++;
                            }
                            size_t x_end = request_str.find(",", x_start);
                            if (x_end == std::string::npos) x_end = request_str.find("}", x_start);
                            std::string x_str = request_str.substr(x_start, x_end - x_start);
                            // Trim whitespace
                            x_str.erase(0, x_str.find_first_not_of(" \t"));
                            x_str.erase(x_str.find_last_not_of(" \t") + 1);
                            Logger::LogMessage("DEBUG: x_str before conversion: '" + x_str + "'");
                            int x = static_cast<int>(std::stod(x_str));
                            Logger::LogMessage("DEBUG: Extracted x: " + std::to_string(x));
                            
                            Logger::LogMessage("DEBUG: Extracting y coordinate");
                            // Extract y coordinate (handle numeric values without quotes)
                            size_t y_colon = request_str.find(":", y_pos);
                            size_t y_start = y_colon + 1;
                            // Skip any whitespace
                            while (y_start < request_str.length() && (request_str[y_start] == ' ' || request_str[y_start] == '\t')) {
                                y_start++;
                            }
                            size_t y_end = request_str.find("}", y_start);
                            if (y_end == std::string::npos) y_end = request_str.find(",", y_start);
                            std::string y_str = request_str.substr(y_start, y_end - y_start);
                            // Trim whitespace
                            y_str.erase(0, y_str.find_first_not_of(" \t"));
                            y_str.erase(y_str.find_last_not_of(" \t") + 1);
                            Logger::LogMessage("DEBUG: y_str before conversion: '" + y_str + "'");
                            int y = static_cast<int>(std::stod(y_str));
                            Logger::LogMessage("DEBUG: Extracted y: " + std::to_string(y));
                            
                            Logger::LogMessage("Processing menu overlay request for section: " + section + " at (" + std::to_string(x) + ", " + std::to_string(y) + ")");
                            
                            Logger::LogMessage("DEBUG: About to call OpenMenuOverlay or CloseMenuOverlay");
                            // Handle menu overlay opening
                            if (section == "close") {
                                Logger::LogMessage("DEBUG: Calling CloseMenuOverlay");
                                CloseMenuOverlay();
                            } else {
                                Logger::LogMessage("DEBUG: Calling OpenMenuOverlay");
                                OpenMenuOverlay(section, x, y);
                            }
                            Logger::LogMessage("DEBUG: Successfully completed menu overlay operation");
                        } else {
                            Logger::LogMessage("DEBUG: Missing required JSON fields");
                        }
                        
                        callback->Success("success");
                        return true;
                    } catch (const std::exception& ex) {
                        Logger::LogMessage("Exception in open_menu_overlay handler: " + std::string(ex.what()));
                        callback->Failure(500, "Internal error in menu overlay");
                        return true;
                    } catch (...) {
                        Logger::LogMessage("Unknown exception in open_menu_overlay handler (possibly 0xe06d7363)");
                        callback->Failure(500, "Unknown error in menu overlay");
                        return true;
                    }
                }
                else if (type == "menu_item_click") {
                    try {
                        // Extract section and action from JSON
                        size_t section_pos = request_str.find("\"section\":");
                        size_t action_pos = request_str.find("\"action\":");
                        
                        if (section_pos != std::string::npos && action_pos != std::string::npos) {
                            // Extract section
                            size_t section_start = request_str.find("\"", section_pos + 10) + 1;
                            size_t section_end = request_str.find("\"", section_start);
                            std::string section = request_str.substr(section_start, section_end - section_start);
                            
                            // Extract action
                            size_t action_start = request_str.find("\"", action_pos + 9) + 1;
                            size_t action_end = request_str.find("\"", action_start);
                            std::string action = request_str.substr(action_start, action_end - action_start);
                            
                            // Log menu item click for debugging
                            Logger::LogMessage("Menu item clicked - Section: " + section + ", Action: " + action);
                            
                            // Close menu overlay after click
                            CloseMenuOverlay();
                            
                            callback->Success("success");
                            return true;
                        } else {
                            Logger::LogMessage("Invalid menu item click parameters");
                            callback->Failure(400, "Invalid parameters");
                            return true;
                        }
                    } catch (const std::exception& ex) {
                        Logger::LogMessage("Exception in menu_item_click handler: " + std::string(ex.what()));
                        callback->Failure(500, "Internal error in menu item click");
                        return true;
                    } catch (...) {
                        Logger::LogMessage("Unknown exception in menu_item_click handler (possibly 0xe06d7363)");
                        callback->Failure(500, "Unknown error in menu item click");
                        return true;
                    }
                }
                else if (type == "open_editor") {
                    try {
                        // Extract position and dimensions from JSON
                        size_t x_pos = request_str.find("\"x\":");
                        size_t y_pos = request_str.find("\"y\":");
                        size_t width_pos = request_str.find("\"width\":");
                        size_t height_pos = request_str.find("\"height\":");
                        
                        if (x_pos != std::string::npos && y_pos != std::string::npos && 
                            width_pos != std::string::npos && height_pos != std::string::npos) {
                            
                            // Extract x coordinate
                            size_t x_start = request_str.find(":", x_pos) + 1;
                            size_t x_end = request_str.find_first_of(",}", x_start);
                            int x = std::stoi(request_str.substr(x_start, x_end - x_start));
                            
                            // Extract y coordinate
                            size_t y_start = request_str.find(":", y_pos) + 1;
                            size_t y_end = request_str.find_first_of(",}", y_start);
                            int y = std::stoi(request_str.substr(y_start, y_end - y_start));
                            
                            // Extract width
                            size_t width_start = request_str.find(":", width_pos) + 1;
                            size_t width_end = request_str.find_first_of(",}", width_start);
                            int width = std::stoi(request_str.substr(width_start, width_end - width_start));
                            
                            // Extract height
                            size_t height_start = request_str.find(":", height_pos) + 1;
                            size_t height_end = request_str.find_first_of(",}", height_start);
                            int height = std::stoi(request_str.substr(height_start, height_end - height_start));
                            
                            Logger::LogMessage("Opening editor at position: " + std::to_string(x) + "," + std::to_string(y) + 
                                             " with size: " + std::to_string(width) + "x" + std::to_string(height));
                            
                            // Open editor using the same technique as menu overlay
                            OpenEditor(x, y, width, height);
                            
                            callback->Success("success");
                            return true;
                        } else {
                            Logger::LogMessage("DEBUG: Missing required JSON fields for editor");
                            callback->Failure(400, "Missing required parameters");
                            return true;
                        }
                    } catch (const std::exception& ex) {
                        Logger::LogMessage("Exception in open_editor handler: " + std::string(ex.what()));
                        callback->Failure(500, "Internal error in editor");
                        return true;
                    } catch (...) {
                        Logger::LogMessage("Unknown exception in open_editor handler (possibly 0xe06d7363)");
                        callback->Failure(500, "Unknown error in editor");
                        return true;
                    }
                }
                else if (type == "close_editor") {
                    try {
                        Logger::LogMessage("Closing editor");
                        CloseEditor();
                        callback->Success("success");
                        return true;
                    } catch (const std::exception& ex) {
                        Logger::LogMessage("Exception in close_editor handler: " + std::string(ex.what()));
                        callback->Failure(500, "Internal error closing editor");
                        return true;
                    } catch (...) {
                        Logger::LogMessage("Unknown exception in close_editor handler (possibly 0xe06d7363)");
                        callback->Failure(500, "Unknown error closing editor");
                        return true;
                    }
                }
                else if (type == "editor_position_update") {
                    try {
                        // Extract new position and dimensions from JSON
                        size_t x_pos = request_str.find("\"x\":");
                        size_t y_pos = request_str.find("\"y\":");
                        size_t width_pos = request_str.find("\"width\":");
                        size_t height_pos = request_str.find("\"height\":");
                        
                        if (x_pos != std::string::npos && y_pos != std::string::npos && 
                            width_pos != std::string::npos && height_pos != std::string::npos) {
                            
                            // Extract coordinates and dimensions
                            size_t x_start = request_str.find(":", x_pos) + 1;
                            size_t x_end = request_str.find_first_of(",}", x_start);
                            int x = std::stoi(request_str.substr(x_start, x_end - x_start));
                            
                            size_t y_start = request_str.find(":", y_pos) + 1;
                            size_t y_end = request_str.find_first_of(",}", y_start);
                            int y = std::stoi(request_str.substr(y_start, y_end - y_start));
                            
                            size_t width_start = request_str.find(":", width_pos) + 1;
                            size_t width_end = request_str.find_first_of(",}", width_start);
                            int width = std::stoi(request_str.substr(width_start, width_end - width_start));
                            
                            size_t height_start = request_str.find(":", height_pos) + 1;
                            size_t height_end = request_str.find_first_of(",}", height_start);
                            int height = std::stoi(request_str.substr(height_start, height_end - height_start));
                            
                            Logger::LogMessage("Updating editor position to: " + std::to_string(x) + "," + std::to_string(y) + 
                                             " with size: " + std::to_string(width) + "x" + std::to_string(height));
                            
                            // Update editor position using window method
                            if (window_) {
                                window_->SetEditorPosition(x, y, width, height);
                            }
                            
                            callback->Success("success");
                            return true;
                        } else {
                            Logger::LogMessage("DEBUG: Missing required JSON fields for editor position update");
                            callback->Failure(400, "Missing required parameters");
                            return true;
                        }
                    } catch (const std::exception& ex) {
                        Logger::LogMessage("Exception in editor_position_update handler: " + std::string(ex.what()));
                        callback->Failure(500, "Internal error updating editor position");
                        return true;
                    } catch (...) {
                        Logger::LogMessage("Unknown exception in editor_position_update handler (possibly 0xe06d7363)");
                        callback->Failure(500, "Unknown error updating editor position");
                        return true;
                    }
                }
            }
        } catch (const std::exception&) {
            callback->Failure(-1, "Failed to parse menu overlay request");
            return true;
        }
    }
    
    // Handle IPC calls
    if (request_str.find("ipc_call:") == 0) {
        // Parse IPC call format: "ipc_call:method:message"
        size_t first_colon = request_str.find(':', 9); // Skip "ipc_call:"
        if (first_colon != std::string::npos) {
            std::string method = request_str.substr(9, first_colon - 9);
            std::string message = request_str.substr(first_colon + 1);
            
            // Handle the IPC call
            std::string response = SimpleIPC::IPCHandler::GetInstance().HandleCall(method, message);
            callback->Success(response);
            return true;
        }
    }
    
    // Legacy window control handlers (kept for backward compatibility)
    if (request_str == "minimize_window") {
        if (window_) {
            window_->Minimize();
            callback->Success("");
            return true;
        }
    }
    else if (request_str == "maximize_window") {
        if (window_) {
            window_->Maximize();
            callback->Success("");
            return true;
        }
    }
    else if (request_str == "restore_window") {
        if (window_) {
            window_->Restore();
            callback->Success("");
            return true;
        }
    }
    else if (request_str == "close_window") {
        if (window_) {
            window_->Close();
            callback->Success("");
            return true;
        }
    }
    else if (request_str == "toggle_borderless") {
        if (window_) {
            window_->SetBorderless(!window_->ShouldClose()); // Toggle borderless mode
            callback->Success("");
            return true;
        }
    }
    
    return false;
}

bool SDL3Window::HandleWindowDragging(const SDL_Event& event) {
    if (!client_) return false;
    
    switch (event.type) {
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                // Check if the click is in a draggable region
                if (client_->IsPointInDragRegion(event.button.x, event.button.y)) {
                    is_dragging_ = true;
                    drag_start_x_ = event.button.x;
                    drag_start_y_ = event.button.y;
                    
                    // Get current window position
                    SDL_GetWindowPosition(window_, &window_start_x_, &window_start_y_);
                    
                    // Capture mouse to ensure we get all mouse events
                    SDL_CaptureMouse(true);
                    
                    Logger::LogMessage("Window dragging started at (" + std::to_string(event.button.x) + ", " + std::to_string(event.button.y) + ") - in draggable region");
                    return true; // Consume the event
                } else {
                    // Point is not in a draggable region, don't start dragging
                    // Let the event pass through to CEF for normal interaction
                    Logger::LogMessage("Mouse click at (" + std::to_string(event.button.x) + ", " + std::to_string(event.button.y) + ") - not in draggable region, passing to CEF");
                    return false;
                }
            }
            break;
            
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event.button.button == SDL_BUTTON_LEFT && is_dragging_) {
                is_dragging_ = false;
                
                // Release mouse capture
                SDL_CaptureMouse(false);
                
                Logger::LogMessage("Window dragging stopped");
                return true; // Consume the event
            }
            break;
            
        case SDL_EVENT_MOUSE_MOTION:
            if (is_dragging_) {
                // Use global mouse position for more accurate dragging
                float global_x, global_y;
                SDL_GetGlobalMouseState(&global_x, &global_y);
                
                // Calculate offset from drag start position
                int offset_x = (int)global_x - (window_start_x_ + drag_start_x_);
                int offset_y = (int)global_y - (window_start_y_ + drag_start_y_);
                
                // Calculate new window position
                int new_x = window_start_x_ + offset_x;
                int new_y = window_start_y_ + offset_y;
                
                // Move the window
                SDL_SetWindowPosition(window_, new_x, new_y);
                return true; // Consume the event
            }
            break;
    }
    
    return false;
}

void SimpleClient::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                const CefString& title) {
    CEF_REQUIRE_UI_THREAD();
    
    if (window_ && window_->GetSDLWindow()) {
        SDL_SetWindowTitle(window_->GetSDLWindow(), title.ToString().c_str());
    }
}

void SimpleClient::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefContextMenuParams> params,
                                     CefRefPtr<CefMenuModel> model) {
    CEF_REQUIRE_UI_THREAD();
    
    // Clear the context menu to disable right-click menu
    model->Clear();
}

bool SimpleClient::OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefContextMenuParams> params,
                                      int command_id,
                                      EventFlags event_flags) {
    CEF_REQUIRE_UI_THREAD();
    return false;
}

bool SimpleClient::OnDragEnter(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefDragData> dragData,
                             CefDragHandler::DragOperationsMask mask) {
    CEF_REQUIRE_UI_THREAD();
    return false;
}

void SimpleClient::OnDraggableRegionsChanged(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const std::vector<CefDraggableRegion>& regions) {
    CEF_REQUIRE_UI_THREAD();
    
    // Store the draggable regions for window movement
    draggable_regions_ = regions;
    
    // Enhanced logging for debugging
    Logger::LogMessage("Draggable regions updated. Total regions: " + std::to_string(regions.size()));
    
    for (size_t i = 0; i < regions.size(); ++i) {
        const auto& region = regions[i];
        std::string region_type = region.draggable ? "draggable" : "non-draggable";
        Logger::LogMessage("Region " + std::to_string(i) + ": " + region_type + 
                          " at (" + std::to_string(region.bounds.x) + ", " + std::to_string(region.bounds.y) + 
                          ") size " + std::to_string(region.bounds.width) + "x" + std::to_string(region.bounds.height));
    }
}

bool SimpleClient::IsPointInDragRegion(int x, int y) const {
    // First, check if point is in any non-draggable region
    // Non-draggable regions take priority over draggable ones
    for (const auto& region : draggable_regions_) {
        if (!region.draggable &&
            x >= region.bounds.x &&
            x < region.bounds.x + region.bounds.width &&
            y >= region.bounds.y &&
            y < region.bounds.y + region.bounds.height) {
            // Point is in a non-draggable region (app-region: no-drag)
            return false;
        }
    }
    
    // Then check if point is in any draggable region
    for (const auto& region : draggable_regions_) {
        if (region.draggable &&
            x >= region.bounds.x &&
            x < region.bounds.x + region.bounds.width &&
            y >= region.bounds.y &&
            y < region.bounds.y + region.bounds.height) {
            // Point is in a draggable region (app-region: drag)
            return true;
        }
    }
    
    // Point is not in any defined region, default to non-draggable
    return false;
}

bool SimpleClient::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                bool user_gesture,
                                bool is_redirect) {
    CEF_REQUIRE_UI_THREAD();
    
    message_router_->OnBeforeBrowse(browser, frame);
    return false;
}

bool SimpleClient::OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   const CefString& target_url,
                                   CefRequestHandler::WindowOpenDisposition target_disposition,
                                   bool user_gesture) {
    CEF_REQUIRE_UI_THREAD();
    
    // Handle external URL opening
    if (target_disposition == 3 || // WOD_NEW_FOREGROUND_TAB
        target_disposition == 4 || // WOD_NEW_BACKGROUND_TAB
        target_disposition == 5 || // WOD_NEW_POPUP
        target_disposition == 6) { // WOD_NEW_WINDOW
        
        // Open in system default browser
        ShellExecuteA(nullptr, "open", target_url.ToString().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        return true;
    }
    
    return false;
}

bool SimpleClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           CefProcessId source_process,
                                           CefRefPtr<CefProcessMessage> message) {
    CEF_REQUIRE_UI_THREAD();
    
    return message_router_->OnProcessMessageReceived(browser, frame, source_process, message);
}

CefRefPtr<CefResourceHandler> SimpleClient::GetResourceHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request) {
    CEF_REQUIRE_IO_THREAD();
    
    return resource_provider_->Create(browser, frame, "miko", request);
}

bool SimpleClient::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 int popup_id,
                                 const CefString& target_url,
                                 const CefString& target_frame_name,
                                 CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                                 bool user_gesture,
                                 const CefPopupFeatures& popupFeatures,
                                 CefWindowInfo& windowInfo,
                                 CefRefPtr<CefClient>& client,
                                 CefBrowserSettings& settings,
                                 CefRefPtr<CefDictionaryValue>& extra_info,
                                 bool* no_javascript_access) {
    CEF_REQUIRE_UI_THREAD();
    
    // Block popups and open in system browser instead
    ShellExecuteA(nullptr, "open", target_url.ToString().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return true;
}

void SimpleClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();
    
    browser_list_.push_back(browser);
    
    if (browser_list_.size() == 1) {
        // First CEF browser created
        Logger::LogMessage("First CEF browser created");
    } else if (browser_list_.size() == 2) {
        // Second browser is the editor browser (created by OpenEditor)
        if (window_) {
            window_->SetEditorBrowser(browser);
            Logger::LogMessage("Editor browser (2nd browser) created and stored for OSR rendering");
        }
    } else {
        // Third+ browser - check if this is a menu overlay browser and apply transparency
        std::string url = browser->GetMainFrame()->GetURL().ToString();
        Logger::LogMessage("OnAfterCreated: Checking URL for browser #" + std::to_string(browser_list_.size()) + ": " + url);
        if (url.find("miko://menuoverlay/") == 0) {
            // Get the native window handle for the overlay
            HWND overlay_hwnd = browser->GetHost()->GetWindowHandle();
            if (overlay_hwnd) {
                // Apply full transparency settings to the overlay window
                // Use RGB(0,0,0) as transparent color key and set alpha to 0 for full transparency
                SetLayeredWindowAttributes(overlay_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY | LWA_ALPHA);
                Logger::LogMessage("Applied full transparency to menu overlay window (alpha=0)");
                
                // Ensure the window stays on top
                SetWindowPos(overlay_hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
                           SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                Logger::LogMessage("Set menu overlay window to stay on top");
            } else {
                Logger::LogMessage("Failed to get overlay window handle for transparency");
            }
        }
    }
}

bool SimpleClient::DoClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();
    return false;
}

void SimpleClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();
    
    // Check if this is the menu overlay browser being closed
    if (menu_overlay_browser_ && menu_overlay_browser_->IsSame(browser)) {
        menu_overlay_active_ = false;
        menu_overlay_browser_ = nullptr;
        Logger::LogMessage("Menu overlay browser closed, tracking reset");
    }
    
    // Remove from the list of existing browsers
    BrowserList::iterator bit = browser_list_.begin();
    for (; bit != browser_list_.end(); ++bit) {
        if ((*bit)->IsSame(browser)) {
            browser_list_.erase(bit);
            break;
        }
    }
    
    if (browser_list_.empty()) {
        // All browsers closed, quit the application
        if (window_) {
            window_->Close();
        }
    }
}

void SimpleClient::OnLoadError(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              ErrorCode errorCode,
                              const CefString& errorText,
                              const CefString& failedUrl) {
    CEF_REQUIRE_UI_THREAD();
    
    // Don't display an error for downloaded files
    if (errorCode == ERR_ABORTED)
        return;
    
    // Display a load error message using a data: URI
    std::ostringstream loadError;
    loadError << "<html><body bgcolor=\"white\">"
                 "<h2>Failed to load URL " << std::string(failedUrl) << 
                 " with error " << std::string(errorText) << " (" << errorCode <<
                 ").</h2></body></html>";
    
    frame->LoadURL(GetDataURI(loadError.str(), "text/html"));
}

void SimpleClient::OnLoadStart(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              TransitionType transition_type) {
    CEF_REQUIRE_UI_THREAD();
    
    if (frame->IsMain()) {
        // Main frame started loading
        Logger::LogMessage("Main frame started loading");
    }
}

void SimpleClient::CloseAllBrowsers(bool force_close) {
    if (!CefCurrentlyOn(TID_UI)) {
        // Execute on the UI thread
        CefPostTask(TID_UI, new CloseBrowserTask(this, force_close));
        return;
    }
    
    DoCloseAllBrowsers(force_close);
}

void SimpleClient::DoCloseAllBrowsers(bool force_close) {
    CEF_REQUIRE_UI_THREAD();
    
    if (browser_list_.empty())
        return;
    
    BrowserList::const_iterator it = browser_list_.begin();
    for (; it != browser_list_.end(); ++it) {
        (*it)->GetHost()->CloseBrowser(force_close);
    }
}

CefRefPtr<CefBrowser> SimpleClient::GetFirstBrowser() {
    CEF_REQUIRE_UI_THREAD();
    
    if (!browser_list_.empty())
        return browser_list_.front();
    return nullptr;
}

bool SimpleClient::HasBrowsers() {
    CEF_REQUIRE_UI_THREAD();
    return !browser_list_.empty();
}

bool SimpleClient::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                                const CefKeyEvent& event,
                                CefEventHandle os_event,
                                bool* is_keyboard_shortcut) {
    CEF_REQUIRE_UI_THREAD();
    
    // Handle keyboard shortcuts
    if (event.type == KEYEVENT_KEYDOWN) {
        // F11 for fullscreen toggle
        if (event.windows_key_code == VK_F11) {
            if (window_) {
                if (window_->IsMaximized()) {
                    window_->Restore();
                } else {
                    window_->Maximize();
                }
            }
            return true;
        }
        
        // F12 for developer tools
        if (event.windows_key_code == VK_F12) {
            CefWindowInfo windowInfo;
            CefBrowserSettings settings;
            
            // Configure the window for DevTools
            windowInfo.SetAsPopup(nullptr, "DevTools");
            windowInfo.bounds.x = 100;
            windowInfo.bounds.y = 100;
            windowInfo.bounds.width = MIN_WINDOW_WIDTH;
            windowInfo.bounds.height = 600;
            
            browser->GetHost()->ShowDevTools(windowInfo, this, settings, CefPoint());
            return true;
        }
        
        // Ctrl+R or F5 for refresh
        if ((event.modifiers & EVENTFLAG_CONTROL_DOWN && event.windows_key_code == 'R') ||
            event.windows_key_code == VK_F5) {
            browser->Reload();
            return true;
        }
        
        // Ctrl+Shift+R for hard refresh
        if (event.modifiers & EVENTFLAG_CONTROL_DOWN && 
            event.modifiers & EVENTFLAG_SHIFT_DOWN && 
            event.windows_key_code == 'R') {
            browser->ReloadIgnoreCache();
            return true;
        }
    }
    
    return false;
}

bool SimpleClient::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefDownloadItem> download_item,
                                   const CefString& suggested_name,
                                   CefRefPtr<CefBeforeDownloadCallback> callback) {
    CEF_REQUIRE_UI_THREAD();
    
    // Set the download path
    std::string download_path = GetDownloadPath(suggested_name.ToString());
    callback->Continue(download_path, false);
    
    return true;
}

void SimpleClient::OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefDownloadItem> download_item,
                                    CefRefPtr<CefDownloadItemCallback> callback) {
    CEF_REQUIRE_UI_THREAD();
    
    if (download_item->IsComplete()) {
        Logger::LogMessage("Download completed: " + download_item->GetFullPath().ToString());
    }
}

void SimpleClient::SpawnNewWindow() {
    // This would create a new window instance
    // Implementation depends on application architecture
}

// OSR specific methods
void SimpleClient::SendMouseClickEvent(int x, int y, CefBrowserHost::MouseButtonType button, bool mouse_up, int click_count) {
    auto browser = GetFirstBrowser();
    if (browser && window_) {
        CefMouseEvent mouse_event;
        // Apply DPI scaling to mouse coordinates for CEF
        float dpi_scale = window_->GetDPIScale();
        mouse_event.x = static_cast<int>(x / dpi_scale);
        mouse_event.y = static_cast<int>(y / dpi_scale);
        mouse_event.modifiers = 0;
        browser->GetHost()->SendMouseClickEvent(mouse_event, button, mouse_up, click_count);
    }
}

void SimpleClient::SendMouseMoveEvent(int x, int y, bool mouse_leave) {
    auto browser = GetFirstBrowser();
    if (browser && window_) {
        CefMouseEvent mouse_event;
        // Apply DPI scaling to mouse coordinates for CEF
        float dpi_scale = window_->GetDPIScale();
        mouse_event.x = static_cast<int>(x / dpi_scale);
        mouse_event.y = static_cast<int>(y / dpi_scale);
        mouse_event.modifiers = 0;
        browser->GetHost()->SendMouseMoveEvent(mouse_event, mouse_leave);
    }
}

void SimpleClient::SendMouseWheelEvent(int x, int y, int delta_x, int delta_y) {
    auto browser = GetFirstBrowser();
    if (browser && window_) {
        CefMouseEvent mouse_event;
        // Apply DPI scaling to mouse coordinates for CEF
        float dpi_scale = window_->GetDPIScale();
        mouse_event.x = static_cast<int>(x / dpi_scale);
        mouse_event.y = static_cast<int>(y / dpi_scale);
        mouse_event.modifiers = 0;
        browser->GetHost()->SendMouseWheelEvent(mouse_event, delta_x, delta_y);
    }
}

// Editor sublayer management methods
void SDL3Window::EnableEditor(bool enable) {
    editor_enabled_ = enable;
    Logger::LogMessage("Editor sublayer " + std::string(enable ? "enabled" : "disabled"));
}

void SDL3Window::SetEditorPosition(int x, int y, int width, int height) {
    editor_rect_.x = x;
    editor_rect_.y = y;
    editor_rect_.width = width;
    editor_rect_.height = height;
    Logger::LogMessage("Editor position set to (" + std::to_string(x) + ", " + std::to_string(y) + 
                      ") with size " + std::to_string(width) + "x" + std::to_string(height));
}

void SDL3Window::SetEditorBrowser(CefRefPtr<CefBrowser> browser) {
    editor_browser_ = browser;
    Logger::LogMessage("Editor browser reference set");
}

void SDL3Window::UpdateEditorTexture(const void* buffer, int width, int height) {
    Logger::LogMessage("UpdateEditorTexture called: " + std::to_string(width) + "x" + std::to_string(height) + 
                      ", editor_enabled_=" + (editor_enabled_ ? "true" : "false") + 
                      ", editor_browser_=" + (editor_browser_ ? "valid" : "null"));
    
    if (!buffer) {
        Logger::LogMessage("UpdateEditorTexture: Buffer is null!");
        return;
    }
    
    if (width <= 0 || height <= 0) {
        Logger::LogMessage("UpdateEditorTexture: Invalid dimensions - width: " + std::to_string(width) + ", height: " + std::to_string(height));
        return;
    }
    
    if (!editor_enabled_ || !editor_browser_) {
        Logger::LogMessage("UpdateEditorTexture: Early return - editor not enabled or browser null");
        return;
    }
    
    if (!renderer_) {
        Logger::LogMessage("UpdateEditorTexture: Renderer is null!");
        return;
    }
    
    // Store current texture dimensions for comparison
    static int current_texture_width = 0;
    static int current_texture_height = 0;
    
    // Create or recreate editor texture if dimensions changed
    if (!editor_texture_ || 
        current_texture_width != width || 
        current_texture_height != height) {
        
        Logger::LogMessage("UpdateEditorTexture: Creating new texture (old: " + 
                          std::to_string(current_texture_width) + "x" + std::to_string(current_texture_height) + 
                          ", new: " + std::to_string(width) + "x" + std::to_string(height) + ")");
        
        if (editor_texture_) {
            SDL_DestroyTexture(editor_texture_);
            Logger::LogMessage("UpdateEditorTexture: Destroyed old texture");
        }
        
        // Try RGBA32 format first, then fallback to BGRA32
        editor_texture_ = SDL_CreateTexture(renderer_, 
                                          SDL_PIXELFORMAT_BGRA32,
                                          SDL_TEXTUREACCESS_STREAMING, 
                                          width, height);
        
        if (!editor_texture_) {
            Logger::LogMessage("UpdateEditorTexture: RGBA32 failed, trying BGRA32");
            editor_texture_ = SDL_CreateTexture(renderer_, 
                                              SDL_PIXELFORMAT_BGRA32, 
                                              SDL_TEXTUREACCESS_STREAMING, 
                                              width, height);
        }
        
        if (!editor_texture_) {
            const char* error = SDL_GetError();
            Logger::LogMessage("UpdateEditorTexture: Failed to create editor texture with both formats: " + std::string(error ? error : "Unknown error"));
            return;
        }
        
        // Enable alpha blending for the editor texture
        int blend_result = SDL_SetTextureBlendMode(editor_texture_, SDL_BLENDMODE_BLEND);
        if (blend_result != 0) {
            const char* error = SDL_GetError();
            Logger::LogMessage("UpdateEditorTexture: Failed to set blend mode (result=" + std::to_string(blend_result) + "): " + std::string(error ? error : "Unknown error"));
            // Continue anyway - blend mode failure is not critical
        } else {
            Logger::LogMessage("UpdateEditorTexture: Successfully set blend mode");
        }
        
        // Update stored dimensions
        current_texture_width = width;
        current_texture_height = height;
        
        Logger::LogMessage("UpdateEditorTexture: Successfully created editor texture: " + std::to_string(width) + "x" + std::to_string(height));
    }
    
    // Update texture with CEF buffer data
    void* pixels;
    int pitch;
    
    // Clear any previous SDL errors
    SDL_ClearError();
    
    bool lock_success = SDL_LockTexture(editor_texture_, nullptr, &pixels, &pitch);
    if (lock_success) {
        Logger::LogMessage("UpdateEditorTexture: Successfully locked texture - pitch=" + std::to_string(pitch) + 
                          ", expected_row_bytes=" + std::to_string(width * 4));
        
        if (!pixels) {
            Logger::LogMessage("UpdateEditorTexture: Locked texture but pixels pointer is null");
            SDL_UnlockTexture(editor_texture_);
            return;
        }
        
        // Copy row by row to handle potential pitch differences
        const uint8_t* src = static_cast<const uint8_t*>(buffer);
        uint8_t* dst = static_cast<uint8_t*>(pixels);
        const int rowBytes = width * 4; // 4 bytes per pixel (BGRA)
        
        if (pitch < rowBytes) {
            Logger::LogMessage("UpdateEditorTexture: Invalid pitch " + std::to_string(pitch) + " < " + std::to_string(rowBytes));
            SDL_UnlockTexture(editor_texture_);
            return;
        }
        
        for (int y = 0; y < height; y++) {
            memcpy(dst + y * pitch, src + y * rowBytes, rowBytes);
        }
        
        SDL_UnlockTexture(editor_texture_);
        Logger::LogMessage("UpdateEditorTexture: Successfully updated texture data");
    } else {
        const char* error = SDL_GetError();
        Logger::LogMessage("UpdateEditorTexture: Failed to lock editor texture: " + std::string(error ? error : "Unknown error"));
        
        // If texture is already locked, this might be a threading issue
        // Try to unlock it first and then retry once
        if (error && std::string(error).find("already locked") != std::string::npos) {
            Logger::LogMessage("UpdateEditorTexture: Texture already locked, attempting to unlock and retry");
            SDL_UnlockTexture(editor_texture_);
            SDL_ClearError();
            
            // Retry lock once
            lock_success = SDL_LockTexture(editor_texture_, nullptr, &pixels, &pitch);
            if (lock_success) {
                Logger::LogMessage("UpdateEditorTexture: Retry lock successful - pitch=" + std::to_string(pitch));
                
                if (pixels) {
                    const uint8_t* src = static_cast<const uint8_t*>(buffer);
                    uint8_t* dst = static_cast<uint8_t*>(pixels);
                    const int rowBytes = width * 4;
                    
                    if (pitch >= rowBytes) {
                        for (int y = 0; y < height; y++) {
                            memcpy(dst + y * pitch, src + y * rowBytes, rowBytes);
                        }
                        Logger::LogMessage("UpdateEditorTexture: Successfully updated texture data on retry");
                    } else {
                        Logger::LogMessage("UpdateEditorTexture: Invalid pitch on retry " + std::to_string(pitch) + " < " + std::to_string(rowBytes));
                    }
                }
                SDL_UnlockTexture(editor_texture_);
            } else {
                const char* retry_error = SDL_GetError();
                Logger::LogMessage("UpdateEditorTexture: Retry lock also failed: " + std::string(retry_error ? retry_error : "Unknown error"));
            }
        }
    }
}

// Menu overlay management methods
void SimpleClient::OpenEditor(int x, int y, int width, int height) {
    CEF_REQUIRE_UI_THREAD();
    
    try {
        Logger::LogMessage("OpenEditor called - x: " + std::to_string(x) + ", y: " + std::to_string(y) + 
                          ", width: " + std::to_string(width) + ", height: " + std::to_string(height));
        
        if (window_) {
            Logger::LogMessage("Creating OSR editor browser");
            
            // Enable editor in the window
            window_->EnableEditor(true);
            window_->SetEditorPosition(x, y, width, height);
            
            // Create OSR browser for the editor
            CefWindowInfo window_info;
            window_info.SetAsWindowless(nullptr); // Use OSR mode
            
            Logger::LogMessage("Editor OSR bounds - x: " + std::to_string(x) + 
                             ", y: " + std::to_string(y) + ", w: " + std::to_string(width) + 
                             ", h: " + std::to_string(height));
            
            CefBrowserSettings browser_settings;
            browser_settings.javascript_close_windows = STATE_ENABLED;
            browser_settings.background_color = CefColorSetARGB(0, 0, 0, 0); // Fully transparent background for consistency
            
            // Create editor URL using monaco resource
            std::string editor_url = "miko://monaco/index.html";
            Logger::LogMessage("Editor URL: " + editor_url);
            
            // Create OSR render handler for the editor
            CefRefPtr<OSRRenderHandler> render_handler = new OSRRenderHandler(window_);
            
            bool browser_created = CefBrowserHost::CreateBrowser(window_info, this, editor_url, browser_settings, nullptr, nullptr);
            Logger::LogMessage("CreateBrowser result: " + std::string(browser_created ? "true" : "false"));
            
            Logger::LogMessage("Monaco editor opened with OSR at position (" + std::to_string(x) + ", " + std::to_string(y) + ") " +
                             "with size (" + std::to_string(width) + "x" + std::to_string(height) + 
                             ") and URL: " + editor_url);
        } else {
            Logger::LogMessage("Failed to open editor: window is null");
        }
    } catch (const std::exception& ex) {
        Logger::LogMessage("Exception in OpenEditor: " + std::string(ex.what()));
    } catch (...) {
        Logger::LogMessage("Unknown exception in OpenEditor");
    }
}

void SimpleClient::CloseEditor() {
    CEF_REQUIRE_UI_THREAD();
    
    try {
        // Find and close any editor browsers
        for (auto it = browser_list_.begin(); it != browser_list_.end(); ++it) {
            CefRefPtr<CefBrowser> browser = *it;
            if (browser && browser->GetHost()) {
                std::string url = browser->GetMainFrame()->GetURL().ToString();
                // Check for miko:// protocol URLs with monaco
                if (url.find("miko://monaco/") == 0) {
                    browser->GetHost()->CloseBrowser(true);
                    if (window_) {
                        window_->EnableEditor(false);
                    }
                    Logger::LogMessage("Monaco editor closed");
                    break;
                }
            }
        }
    } catch (const std::exception& ex) {
        Logger::LogMessage("Exception in CloseEditor: " + std::string(ex.what()));
    } catch (...) {
        Logger::LogMessage("Unknown exception in CloseEditor");
    }
}

void SimpleClient::OpenMenuOverlay(const std::string& section, int x, int y) {
    CEF_REQUIRE_UI_THREAD();
    
    try {
        // Check if menu overlay is already active
        if (menu_overlay_active_) {
            Logger::LogMessage("Menu overlay already active, ignoring request");
            return;
        }

        Logger::LogMessage("OpenMenuOverlay called - section: " + section + ", x: " + std::to_string(x) + ", y: " + std::to_string(y));
        
        if (!browser_list_.empty()) {
            CefRefPtr<CefBrowser> browser = browser_list_.front();
            if (browser && browser->GetHost()) {
                Logger::LogMessage("Browser available, creating overlay window");
                
                // Get screen dimensions for positioning calculations
                int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                int screenHeight = GetSystemMetrics(SM_CYSCREEN);
                
                // Get current cursor position for smart positioning
                POINT cursor_pos;
                GetCursorPos(&cursor_pos);
                
                // Use truly free size - remove fixed dimensions
                // Let the browser content determine its own size
                int overlayWidth = 0;   // No fixed width - let content decide
                int overlayHeight = 0;  // No fixed height - let content decide
                
                // Smart positioning - use provided coordinates directly if given
                int overlayX, overlayY;
                if (x != 0 || y != 0) {
                    // Use provided coordinates directly
                    overlayX = x;
                    overlayY = y;
                    Logger::LogMessage("Using provided coordinates - x: " + std::to_string(x) + ", y: " + std::to_string(y));
                } else {
                    // Use cursor position if no specific coordinates provided
                    overlayX = cursor_pos.x + 10; // Offset slightly from cursor
                    overlayY = cursor_pos.y + 10;
                    Logger::LogMessage("Using cursor position - cursor_x: " + std::to_string(cursor_pos.x) + ", cursor_y: " + std::to_string(cursor_pos.y));
                }
                
                // Adjust position if overlay would go off-screen (but respect provided coordinates)
                if (overlayX + overlayWidth > screenWidth) {
                    if (x == 0 && y == 0) {
                        // Only adjust for cursor positioning
                        overlayX = cursor_pos.x - overlayWidth - 10; // Position to the left of cursor
                    } else {
                        // For provided coordinates, just ensure it fits on screen
                        overlayX = std::max(0, screenWidth - overlayWidth);
                    }
                }
                if (overlayX < 0) {
                    overlayX = 0; // Align to left edge
                }
                
                // Adjust Y position if overlay would go off-screen (but respect provided coordinates)
                if (overlayY + overlayHeight > screenHeight) {
                    if (x == 0 && y == 0) {
                        // Only adjust for cursor positioning
                        overlayY = cursor_pos.y - overlayHeight - 10; // Position above cursor
                    } else {
                        // For provided coordinates, just ensure it fits on screen
                        overlayY = std::max(0, screenHeight - overlayHeight);
                    }
                }
                if (overlayY < 0) {
                    overlayY = 0; // Align to top edge
                }
                
                // Create a new browser window for the menu overlay
                CefWindowInfo window_info;
                window_info.SetAsPopup(window_->GetHWND(), "MenuOverlay");
                
                // Set window style for transparency and layered window
                window_info.ex_style = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT;
                window_info.style = WS_POPUP | WS_VISIBLE;
                
                // Position the overlay window with calculated bounds
                window_info.bounds.x = overlayX;
                window_info.bounds.y = overlayY;
                window_info.bounds.width = overlayWidth;
                window_info.bounds.height = overlayHeight;
                
                Logger::LogMessage("Free-sized window bounds - x: " + std::to_string(overlayX) + 
                                 ", y: " + std::to_string(overlayY) + 
                                 ", w: " + std::to_string(overlayWidth) + 
                                 ", h: " + std::to_string(overlayHeight));
                
                CefBrowserSettings browser_settings;
                browser_settings.javascript_close_windows = STATE_ENABLED;
                // Set transparent background using BGRA format (Alpha = 0 for transparency)
                browser_settings.background_color = CefColorSetARGB(0, 0, 0, 0); // Fully transparent BGRA
                
                // Create Steam-like overlay routing URL
                std::string overlay_url = BuildOverlayURL(section, overlayX, overlayY, overlayWidth, overlayHeight);
                Logger::LogMessage("Overlay URL created, length: " + std::to_string(overlay_url.length()));
                Logger::LogMessage("URL preview (first 200 chars): " + overlay_url.substr(0, 200));
                
                bool browser_created = CefBrowserHost::CreateBrowser(window_info, this, overlay_url, browser_settings, nullptr, nullptr);
                Logger::LogMessage("CreateBrowser result: " + std::string(browser_created ? "true" : "false"));
                
                // Mark overlay as active if browser creation was successful
                if (browser_created) {
                    menu_overlay_active_ = true;
                    Logger::LogMessage("Menu overlay marked as active");
                }
                
                Logger::LogMessage("Menu overlay opened for section: " + section + 
                                 " at auto-positioned (" + std::to_string(overlayX) + ", " + std::to_string(overlayY) + 
                                 ") with size (" + std::to_string(overlayWidth) + "x" + std::to_string(overlayHeight) + 
                                 ") and URL: " + overlay_url);
            } else {
                Logger::LogMessage("Failed to open menu overlay: browser or host is null");
            }
        } else {
            Logger::LogMessage("Failed to open menu overlay: no browsers available");
        }
    } catch (const std::exception& ex) {
        Logger::LogMessage("Exception in OpenMenuOverlay: " + std::string(ex.what()));
    } catch (...) {
        Logger::LogMessage("Unknown exception in OpenMenuOverlay (possibly 0xe06d7363)");
    }
}

void SimpleClient::CloseMenuOverlay() {
    CEF_REQUIRE_UI_THREAD();
    
    // Check if there's an active overlay to close
    if (!menu_overlay_active_) {
        Logger::LogMessage("No active menu overlay to close");
        return;
    }
    
    try {
        // Close the tracked overlay browser if it exists
        if (menu_overlay_browser_ && menu_overlay_browser_->GetHost()) {
            menu_overlay_browser_->GetHost()->CloseBrowser(true);
            Logger::LogMessage("Menu overlay closed");
        } else {
            // Fallback: Find and close any overlay browsers
            for (auto it = browser_list_.begin(); it != browser_list_.end(); ++it) {
                CefRefPtr<CefBrowser> browser = *it;
                if (browser && browser->GetHost()) {
                    std::string url = browser->GetMainFrame()->GetURL().ToString();
                    // Check for miko:// protocol URLs with menuoverlay
                    if (url.find("miko://menuoverlay/") == 0) {
                        browser->GetHost()->CloseBrowser(true);
                        Logger::LogMessage("Menu overlay closed (fallback)");
                        break;
                    }
                }
            }
        }
        
        // Reset tracking variables
        menu_overlay_active_ = false;
        menu_overlay_browser_ = nullptr;
        
    } catch (const std::exception& ex) {
        Logger::LogMessage("Exception in CloseMenuOverlay: " + std::string(ex.what()));
        // Reset tracking variables even on exception
        menu_overlay_active_ = false;
        menu_overlay_browser_ = nullptr;
    } catch (...) {
        Logger::LogMessage("Unknown exception in CloseMenuOverlay (possibly 0xe06d7363)");
        // Reset tracking variables even on exception
        menu_overlay_active_ = false;
        menu_overlay_browser_ = nullptr;
    }
}



std::string SimpleClient::GetMenuOverlayHTML(const std::string& section) {
    Logger::LogMessage("GetMenuOverlayHTML called for section: " + section);
    
    // Use embedded HTML resource from resourceutil
    std::vector<uint8_t> html_data = ResourceUtil::LoadBinaryResource(ResourceUtil::IDR_HTML_MENUOVERLAY);
    Logger::LogMessage("LoadBinaryResource returned " + std::to_string(html_data.size()) + " bytes");
    
    if (!html_data.empty()) {
        std::string html_content(html_data.begin(), html_data.end());
        Logger::LogMessage("HTML content converted to string, length: " + std::to_string(html_content.length()));
        Logger::LogMessage("HTML preview (first 100 chars): " + html_content.substr(0, 100));
        return html_content;
    }
    
    // Fallback if resource is not available
    Logger::LogMessage("WARNING: Using fallback HTML - resource not available");
    return "<html><body>Menu overlay not available</body></html>";
}

std::string SimpleClient::BuildOverlayURL(const std::string& section, int x, int y, int width, int height) {
    Logger::LogMessage("BuildOverlayURL called - section: " + section);
    
    // Get current process ID and other parameters for URL parameters
    DWORD pid = GetCurrentProcessId();
    int browser_id = -1;
    if (!browser_list_.empty()) {
        browser_id = browser_list_.front()->GetIdentifier();
    }
    
    // Get screen dimensions
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    int create_flags = 4538634;
    
    Logger::LogMessage("Building miko:// URL with parameters - pid: " + std::to_string(pid) + ", browser_id: " + std::to_string(browser_id));
    
    // Use miko:// protocol to serve HTML directly from binary resources with parameters
    std::string overlay_url = "miko://menuoverlay/index.html"
        "?createflags=" + std::to_string(create_flags) +
        "&pid=" + std::to_string(pid) +
        "&browser=" + std::to_string(browser_id) +
        "&screenavailwidth=" + std::to_string(screen_width) +
        "&screenavailheight=" + std::to_string(screen_height) +
        "&section=" + section +
        "&x=" + std::to_string(x) +
        "&y=" + std::to_string(y) +
        "&width=" + std::to_string(width) +
        "&height=" + std::to_string(height);
    
    Logger::LogMessage("Miko URL created: " + overlay_url);
    
    return overlay_url;
}

