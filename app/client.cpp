#include "client.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "internal/simpleipc.hpp"
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
    , width_(1200)
    , height_(800)
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
    , window_start_y_(0) {
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
        const char* error = SDL_GetError();
        Logger::LogMessage("SDL3 initialization failed: " + std::string(error ? error : "No error message"));
        return false;
    }
    
    Logger::LogMessage("SDL initialization successful with driver: " + std::string(SDL_GetCurrentVideoDriver()));
    
    // Check available video drivers for debugging
    int numDrivers = SDL_GetNumVideoDrivers();
    std::string driversInfo = "Available video drivers (" + std::to_string(numDrivers) + "):";
    for (int i = 0; i < numDrivers; i++) {
        driversInfo += "\n  " + std::to_string(i) + ": " + std::string(SDL_GetVideoDriver(i));
    }
    Logger::LogMessage(driversInfo);

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
        Logger::LogMessage("SDL3 window creation failed: " + std::string(SDL_GetError()));
        SDL_Quit();
        return false;
    }

    // Create renderer (SDL3 uses different flags)
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        Logger::LogMessage("SDL3 renderer creation failed: " + std::string(SDL_GetError()));
        SDL_DestroyWindow(window_);
        SDL_Quit();
        return false;
    }

    // Create texture for CEF rendering
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, width_, height_);
    if (!texture_) {
        Logger::LogMessage("SDL3 texture creation failed: " + std::string(SDL_GetError()));
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

    Logger::LogMessage("SDL3Window initialized successfully");
    return true;
}

void SDL3Window::Shutdown() {
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

    // Enable DWM composition
#pragma warning(push)
#pragma warning(disable: 4995) // Disable deprecation warning for DwmEnableComposition
    if (DwmEnableComposition(DWM_EC_ENABLECOMPOSITION) != S_OK) {
        Logger::LogMessage("DwmEnableComposition failed");
    }
#pragma warning(pop)

    // Extend frame into client area for better visual effects
    MARGINS margins = { 0, 0, 0, 1 };
    DwmExtendFrameIntoClientArea(hwnd_, &margins);
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
            return true;

        case SDL_EVENT_WINDOW_RESTORED:
            minimized_ = false;
            maximized_ = false;
            return true;

        case SDL_EVENT_WINDOW_RESIZED:
            width_ = event.window.data1;
            height_ = event.window.data2;
            
            // Recreate texture with new size
            if (texture_) {
                SDL_DestroyTexture(texture_);
                texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32, 
                                           SDL_TEXTUREACCESS_STREAMING, width_, height_);
                if (!texture_) {
                    Logger::LogMessage("Resize: Failed to recreate texture - " + std::string(SDL_GetError()));
                } else {
                    SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_NONE);
                }
            }
            
            // Notify CEF browser of size change
            if (client_) {
                auto browser = client_->GetFirstBrowser();
                if (browser) {
                    browser->GetHost()->WasResized();
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
            SendMouseEvent(event);
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
    if (!client_) return;

    auto browser = client_->GetFirstBrowser();
    if (!browser) return;

    CefMouseEvent mouse_event;
    mouse_event.x = event.button.x;
    mouse_event.y = event.button.y;
    mouse_event.modifiers = 0; // TODO: Handle modifiers

    last_mouse_x_ = event.button.x;
    last_mouse_y_ = event.button.y;

    switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION:
            browser->GetHost()->SendMouseMoveEvent(mouse_event, false);
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
            
            browser->GetHost()->SendMouseClickEvent(mouse_event, button_type, mouse_up, click_count);
            break;
        }
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
    if (!renderer_) {
        Logger::LogMessage("UpdateTexture: Renderer is null");
        return;
    }

    if (!buffer) {
        Logger::LogMessage("UpdateTexture: Buffer is null");
        return;
    }

    Logger::LogMessage("UpdateTexture: Received buffer " + std::to_string(width) + "x" + std::to_string(height));

    // Check if we need to create or recreate the texture
    bool needNewTexture = false;
    
    if (!texture_) {
        needNewTexture = true;
        Logger::LogMessage("UpdateTexture: Creating initial texture " + std::to_string(width) + "x" + std::to_string(height));
    } else {
        // Check if texture dimensions match the incoming data (use float types for SDL3)
        float tex_width = 0.0f, tex_height = 0.0f;
        if (SDL_GetTextureSize(texture_, &tex_width, &tex_height) != 0) {
            Logger::LogMessage("UpdateTexture: Failed to get texture size - " + std::string(SDL_GetError()));
            // Assume we need a new texture if we can't get the size
            needNewTexture = true;
        } else if ((int)tex_width != width || (int)tex_height != height) {
            Logger::LogMessage("UpdateTexture: Recreating texture from " + std::to_string((int)tex_width) + "x" + std::to_string((int)tex_height) + " to " + std::to_string(width) + "x" + std::to_string(height));
            needNewTexture = true;
        }
    }

    // Create or recreate texture if needed
    if (needNewTexture) {
        if (texture_) {
            SDL_DestroyTexture(texture_);
            texture_ = nullptr;
            Logger::LogMessage("UpdateTexture: Destroyed old texture");
        }
        
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, width, height);
        
        if (!texture_) {
            Logger::LogMessage("UpdateTexture: Failed to create texture - " + std::string(SDL_GetError()));
            return;
        }
        // IMPORTANT: Avoid blending with alpha=0 content from OSR
        SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_NONE);

        Logger::LogMessage("UpdateTexture: Successfully created texture " + std::to_string(width) + "x" + std::to_string(height));
    }

    // 🔑 Copy CEF buffer → texture
    void* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(texture_, nullptr, &pixels, &pitch)) {
        Logger::LogMessage("UpdateTexture: Locked texture, pitch=" + std::to_string(pitch) + ", expected=" + std::to_string(width * 4));
        
        const uint8_t* src = static_cast<const uint8_t*>(buffer);
        const int rowBytes = width * 4; // BGRA32
        for (int y = 0; y < height; y++) {
            std::memcpy(static_cast<uint8_t*>(pixels) + y * pitch,
                        src + y * rowBytes,
                        rowBytes);
        }
        SDL_UnlockTexture(texture_);
        Logger::LogMessage("UpdateTexture: Successfully updated texture with buffer data");
    } else {
        Logger::LogMessage("UpdateTexture: Failed to lock texture - " + std::string(SDL_GetError()));
    }
}

void SDL3Window::Render() {
    if (!renderer_) {
        Logger::LogMessage("Render: Renderer is null");
        return;
    }
    
    if (!texture_) {
        Logger::LogMessage("Render: Texture is null");
        return;
    }

    // Get texture properties for debugging using SDL3 properties API
    SDL_PropertiesID props = SDL_GetTextureProperties(texture_);
    if (props) {
        Uint32 format = (Uint32)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_FORMAT_NUMBER, 0);
        int access = (int)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_ACCESS_NUMBER, 0);
        int w = (int)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0);
        int h = (int)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0);
        Logger::LogMessage("Render: Texture info - format=" + std::to_string(format) + ", access=" + std::to_string(access) + ", size=" + std::to_string(w) + "x" + std::to_string(h));
    } else {
        Logger::LogMessage("Render: Failed to get texture properties - " + std::string(SDL_GetError()));
        return;
    }

    Logger::LogMessage("Render: Starting render cycle");

    // Clear the renderer
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    if (!SDL_RenderClear(renderer_)) {
        Logger::LogMessage("Render: Failed to clear renderer - " + std::string(SDL_GetError()));
        return;
    }
    
    // Render the texture (cover whole window)
    if (SDL_RenderTexture(renderer_, texture_, nullptr, nullptr) != 0) {
        Logger::LogMessage("Render: Failed to render texture - " + std::string(SDL_GetError()));
        
        // Try alternative rendering approach
        SDL_FRect destRect = {0, 0, (float)width_, (float)height_};
        if (SDL_RenderTexture(renderer_, texture_, nullptr, &destRect) != 0) {
            Logger::LogMessage("Render: Alternative render also failed - " + std::string(SDL_GetError()));
        } else {
            Logger::LogMessage("Render: Alternative render succeeded");
        }
    } else {
        Logger::LogMessage("Render: Successfully rendered texture");
    }
    
    // Present the rendered frame
    SDL_RenderPresent(renderer_);
    Logger::LogMessage("Render: Presented frame");
}

// OSRRenderHandler implementation
OSRRenderHandler::OSRRenderHandler(SDL3Window* window) : window_(window) {
}

void OSRRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
    if (window_) {
        rect.x = 0;
        rect.y = 0;
        rect.width = 1200; // Default width
        rect.height = 800; // Default height
        
        // Get actual window size
        if (window_->GetSDLWindow()) {
            int w, h;
            SDL_GetWindowSize(window_->GetSDLWindow(), &w, &h);
            rect.width = w;
            rect.height = h;
            Logger::LogMessage("GetViewRect: " + std::to_string(w) + "x" + std::to_string(h));
        } else {
            Logger::LogMessage("GetViewRect: Using default size " + std::to_string(rect.width) + "x" + std::to_string(rect.height));
        }
    } else {
        Logger::LogMessage("GetViewRect: Window is null, using default 1200x800");
        rect.x = 0;
        rect.y = 0;
        rect.width = 1200;
        rect.height = 800;
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
    
    Logger::LogMessage("OnPaint: Received paint event " + std::to_string(width) + "x" + std::to_string(height) + ", dirty rects=" + std::to_string(dirtyRects.size()));

    // UpdateTexture now handles create/recreate + copy
    window_->UpdateTexture(buffer, width, height);
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
SimpleClient::SimpleClient(SDL3Window* window) : window_(window) {
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
                    return true; // Consume the event
                }
            }
            break;
            
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event.button.button == SDL_BUTTON_LEFT && is_dragging_) {
                is_dragging_ = false;
                return true; // Consume the event
            }
            break;
            
        case SDL_EVENT_MOUSE_MOTION:
            if (is_dragging_) {
                // Calculate new window position
                int new_x = window_start_x_ + (event.motion.x - drag_start_x_);
                int new_y = window_start_y_ + (event.motion.y - drag_start_y_);
                
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
    
    Logger::LogMessage("Draggable regions updated: " + std::to_string(regions.size()) + " regions");
}

bool SimpleClient::IsPointInDragRegion(int x, int y) const {
    for (const auto& region : draggable_regions_) {
        if (region.draggable &&
            x >= region.bounds.x &&
            y >= region.bounds.y &&
            x < region.bounds.x + region.bounds.width &&
            y < region.bounds.y + region.bounds.height) {
            return true;
        }
    }
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
    }
}

bool SimpleClient::DoClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();
    return false;
}

void SimpleClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();
    
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
            windowInfo.bounds.width = 800;
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
    if (browser) {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        mouse_event.modifiers = 0;
        browser->GetHost()->SendMouseClickEvent(mouse_event, button, mouse_up, click_count);
    }
}

void SimpleClient::SendMouseMoveEvent(int x, int y, bool mouse_leave) {
    auto browser = GetFirstBrowser();
    if (browser) {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        mouse_event.modifiers = 0;
        browser->GetHost()->SendMouseMoveEvent(mouse_event, mouse_leave);
    }
}

void SimpleClient::SendMouseWheelEvent(int x, int y, int delta_x, int delta_y) {
    auto browser = GetFirstBrowser();
    if (browser) {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        mouse_event.modifiers = 0;
        browser->GetHost()->SendMouseWheelEvent(mouse_event, delta_x, delta_y);
    }
}

void SimpleClient::SendKeyEvent(const CefKeyEvent& event) {
    auto browser = GetFirstBrowser();
    if (browser) {
        browser->GetHost()->SendKeyEvent(event);
    }
}

void SimpleClient::SendFocusEvent(bool set_focus) {
    auto browser = GetFirstBrowser();
    if (browser) {
        browser->GetHost()->SetFocus(set_focus);
    }
}
