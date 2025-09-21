#include "windowed.hpp"
#include "../utils/logger.hpp"
#include <dwmapi.h>
#include <SDL3/SDL.h>

#pragma comment(lib, "dwmapi.lib")

SDL3Window::SDL3Window()
    : window_(nullptr), renderer_(nullptr), texture_(nullptr), hwnd_(nullptr),
      client_(nullptr), width_(DEFAULT_WINDOW_WIDTH),
      height_(DEFAULT_WINDOW_HEIGHT), minimized_(false), maximized_(false),
      should_close_(false), borderless_(true), mouse_captured_(false),
      last_mouse_x_(0), last_mouse_y_(0), is_dragging_(false), drag_start_x_(0),
      drag_start_y_(0), window_start_x_(0), window_start_y_(0),
      dx11_renderer_(nullptr), dx11_enabled_(false), dpi_scale_(1.0f),
      menu_overlay_visible_(false), menu_overlay_x_(0), menu_overlay_y_(0),
      editor_enabled_(false), editor_rect_({0, 0, 0, 0}),
      editor_texture_(nullptr), editor_browser_(nullptr) {}

SDL3Window::~SDL3Window() { Shutdown(); }

bool SDL3Window::Initialize(int width, int height) {
  width_ = width;
  height_ = height;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    Logger::LogMessage("Failed to initialize SDL: " + std::string(SDL_GetError()));
    return false;
  }

  // Create window with borderless style
  window_ = SDL_CreateWindow("MikoIDE", width_, height_,
                             SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
  if (!window_) {
    Logger::LogMessage("Failed to create window: " + std::string(SDL_GetError()));
    return false;
  }

  // Create renderer
  renderer_ = SDL_CreateRenderer(window_, nullptr);
  if (!renderer_) {
    Logger::LogMessage("Failed to create renderer: " + std::string(SDL_GetError()));
    return false;
  }

  // Get native window handle
  hwnd_ = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window_),
                                       SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
  if (!hwnd_) {
    Logger::LogMessage("Failed to get native window handle");
    return false;
  }

  // Initialize DWM API for rounded corners
  InitializeDwmApi();

  // Apply rounded corners
  ApplyRoundedCorners();

  // Update DPI scale
  UpdateDPIScale();

  Logger::LogMessage("SDL3Window initialized successfully");
  return true;
}

void SDL3Window::Shutdown() {
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
  }
}

void SDL3Window::Maximize() {
  if (window_) {
    SDL_MaximizeWindow(window_);
    maximized_ = true;
  }
}

void SDL3Window::Restore() {
  if (window_) {
    SDL_RestoreWindow(window_);
    minimized_ = false;
    maximized_ = false;
  }
}

void SDL3Window::Close() { should_close_ = true; }

HWND SDL3Window::GetHWND() const { return hwnd_; }

void SDL3Window::ApplyRoundedCorners() {
  if (!hwnd_) {
    Logger::LogMessage("Cannot apply rounded corners: HWND is null");
    return;
  }

  // Apply rounded corners using DWM
  DWM_WINDOW_CORNER_PREFERENCE cornerPref = DWMWCP_ROUND;
  HRESULT hr = DwmSetWindowAttribute(hwnd_, DWMWA_WINDOW_CORNER_PREFERENCE,
                                     &cornerPref, sizeof(cornerPref));

  if (SUCCEEDED(hr)) {
    Logger::LogMessage("Successfully applied rounded corners");
  } else {
    Logger::LogMessage("Failed to apply rounded corners: HRESULT = " +
                       std::to_string(hr));
  }
}

void SDL3Window::SetBorderless(bool borderless) {
  borderless_ = borderless;
  UpdateWindowStyle();
}

void SDL3Window::UpdateWindowStyle() {
  if (!hwnd_) {
    Logger::LogMessage("Cannot update window style: HWND is null");
    return;
  }

  LONG_PTR style = GetWindowLongPtr(hwnd_, GWL_STYLE);
  LONG_PTR exStyle = GetWindowLongPtr(hwnd_, GWL_EXSTYLE);

  if (borderless_) {
    // Remove window decorations for borderless mode
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
               WS_SYSMENU);
    exStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE |
                 WS_EX_STATICEDGE);
  } else {
    // Add window decorations for windowed mode
    style |= (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
              WS_SYSMENU);
  }

  SetWindowLongPtr(hwnd_, GWL_STYLE, style);
  SetWindowLongPtr(hwnd_, GWL_EXSTYLE, exStyle);

  // Force window to redraw with new style
  SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
               SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                   SWP_NOOWNERZORDER);
}

void SDL3Window::InitializeDwmApi() {
  // Enable DWM composition if not already enabled
  BOOL enabled = FALSE;
  HRESULT hr = DwmIsCompositionEnabled(&enabled);
  if (SUCCEEDED(hr) && !enabled) {
    DwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
    Logger::LogMessage("DWM composition enabled");
  }
}

void SDL3Window::UpdateDPIScale() {
  if (!hwnd_) {
    dpi_scale_ = 1.0f;
    return;
  }

  // Get DPI for the window
  UINT dpi = GetDpiForWindow(hwnd_);
  if (dpi == 0) {
    // Fallback to system DPI
    HDC hdc = GetDC(nullptr);
    if (hdc) {
      dpi = GetDeviceCaps(hdc, LOGPIXELSX);
      ReleaseDC(nullptr, hdc);
    } else {
      dpi = 96; // Default DPI
    }
  }

  dpi_scale_ = static_cast<float>(dpi) / 96.0f;
  Logger::LogMessage("DPI scale updated to: " + std::to_string(dpi_scale_) +
                     " (DPI: " + std::to_string(dpi) + ")");
}

bool SDL3Window::HandleEvent(const SDL_Event &event) {
  switch (event.type) {
  case SDL_EVENT_QUIT:
    should_close_ = true;
    return true;

  case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
    should_close_ = true;
    return true;

  case SDL_EVENT_WINDOW_MINIMIZED:
    minimized_ = true;
    Logger::LogMessage("Window minimized");
    return true;

  case SDL_EVENT_WINDOW_RESTORED:
    minimized_ = false;
    maximized_ = false;
    Logger::LogMessage("Window restored");
    return true;

  case SDL_EVENT_WINDOW_MAXIMIZED:
    maximized_ = true;
    Logger::LogMessage("Window maximized");
    return true;

  case SDL_EVENT_WINDOW_RESIZED:
    width_ = event.window.data1;
    height_ = event.window.data2;
    Resize(width_, height_);
    Logger::LogMessage("Window resized to " + std::to_string(width_) + "x" +
                       std::to_string(height_));
    return true;

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
  case SDL_EVENT_MOUSE_BUTTON_UP:
  case SDL_EVENT_MOUSE_MOTION:
    // Handle window dragging first
    if (HandleWindowDragging(event)) {
      return true; // Event was consumed by dragging
    }
    // If not consumed by dragging, send to CEF
    SendMouseEvent(event);
    return true;

  case SDL_EVENT_MOUSE_WHEEL:
    SendScrollEvent(event);
    return true;

  case SDL_EVENT_KEY_DOWN:
  case SDL_EVENT_KEY_UP:
    SendKeyEvent(event);
    return true;

  case SDL_EVENT_TEXT_INPUT:
    // Handle text input for CEF
    if (client_) {
      auto browser = client_->GetFirstBrowser();
      if (browser) {
        CefKeyEvent key_event;
        key_event.type = KEYEVENT_CHAR;
        key_event.character = event.text.text[0];
        key_event.unmodified_character = event.text.text[0];
        key_event.modifiers = 0;
        browser->GetHost()->SendKeyEvent(key_event);
      }
    }
    return true;

  default:
    return false;
  }
}

void SDL3Window::SendMouseEvent(const SDL_Event &event) {
  if (!client_) {
    return;
  }

  auto browser = client_->GetFirstBrowser();
  if (!browser) {
    return;
  }

  CefMouseEvent mouse_event;
  mouse_event.modifiers = 0;

  // Apply DPI scaling to mouse coordinates for CEF
  mouse_event.x = static_cast<int>(event.button.x / dpi_scale_);
  mouse_event.y = static_cast<int>(event.button.y / dpi_scale_);

  switch (event.type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    CefBrowserHost::MouseButtonType button_type = MBT_LEFT;
    switch (event.button.button) {
    case SDL_BUTTON_LEFT:
      button_type = MBT_LEFT;
      break;
    case SDL_BUTTON_MIDDLE:
      button_type = MBT_MIDDLE;
      break;
    case SDL_BUTTON_RIGHT:
      button_type = MBT_RIGHT;
      break;
    }

    bool mouse_up = (event.type == SDL_EVENT_MOUSE_BUTTON_UP);
    int click_count = event.button.clicks;

    browser->GetHost()->SendMouseClickEvent(mouse_event, button_type, mouse_up,
                                            click_count);

    // Update mouse capture state
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && !mouse_captured_) {
      mouse_captured_ = true;
      last_mouse_x_ = event.button.x;
      last_mouse_y_ = event.button.y;
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && mouse_captured_) {
      mouse_captured_ = false;
    }
    break;
  }

  case SDL_EVENT_MOUSE_MOTION: {
    mouse_event.x = static_cast<int>(event.motion.x / dpi_scale_);
    mouse_event.y = static_cast<int>(event.motion.y / dpi_scale_);

    browser->GetHost()->SendMouseMoveEvent(mouse_event, false);

    last_mouse_x_ = event.motion.x;
    last_mouse_y_ = event.motion.y;
    break;
  }
  }
}

void SDL3Window::SendKeyEvent(const SDL_Event &event) {
  if (!client_) {
    return;
  }

  auto browser = client_->GetFirstBrowser();
  if (!browser) {
    return;
  }

  CefKeyEvent key_event;
  key_event.windows_key_code = event.key.key;
  key_event.native_key_code = event.key.scancode;
  key_event.modifiers = 0;

  // Set modifiers
  if (event.key.mod & SDL_KMOD_CTRL)
    key_event.modifiers |= EVENTFLAG_CONTROL_DOWN;
  if (event.key.mod & SDL_KMOD_SHIFT)
    key_event.modifiers |= EVENTFLAG_SHIFT_DOWN;
  if (event.key.mod & SDL_KMOD_ALT)
    key_event.modifiers |= EVENTFLAG_ALT_DOWN;

  key_event.type = (event.type == SDL_EVENT_KEY_DOWN) ? KEYEVENT_KEYDOWN
                                                       : KEYEVENT_KEYUP;

  browser->GetHost()->SendKeyEvent(key_event);
}

void SDL3Window::SendScrollEvent(const SDL_Event &event) {
  if (!client_) {
    return;
  }

  auto browser = client_->GetFirstBrowser();
  if (!browser) {
    return;
  }

  CefMouseEvent mouse_event;
  float mouse_x, mouse_y;
  SDL_GetMouseState(&mouse_x, &mouse_y);

  // Apply DPI scaling to mouse coordinates for CEF
  mouse_event.x = static_cast<int>(mouse_x / dpi_scale_);
  mouse_event.y = static_cast<int>(mouse_y / dpi_scale_);
  mouse_event.modifiers = 0;

  int delta_x = static_cast<int>(event.wheel.x * 40); // Scale wheel delta
  int delta_y = static_cast<int>(event.wheel.y * 40);

  browser->GetHost()->SendMouseWheelEvent(mouse_event, delta_x, delta_y);
}

void SDL3Window::UpdateTexture(const void *buffer, int width, int height) {
  if (!buffer || width <= 0 || height <= 0) {
    return;
  }

  // Create or recreate texture if dimensions changed
  if (!texture_ || width != width_ || height != height_) {
    if (texture_) {
      SDL_DestroyTexture(texture_);
    }

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32,
                                 SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture_) {
      Logger::LogMessage("Failed to create texture: " + std::string(SDL_GetError()));
      return;
    }

    width_ = width;
    height_ = height;
  }

  // Update texture with CEF buffer data
  void *pixels;
  int pitch;
  if (SDL_LockTexture(texture_, nullptr, &pixels, &pitch)) {
    // Copy row by row to handle potential pitch differences
    const uint8_t *src = static_cast<const uint8_t *>(buffer);
    uint8_t *dst = static_cast<uint8_t *>(pixels);
    const int rowBytes = width * 4; // 4 bytes per pixel (BGRA)

    for (int y = 0; y < height; y++) {
      memcpy(dst + y * pitch, src + y * rowBytes, rowBytes);
    }

    SDL_UnlockTexture(texture_);
  }
}

void SDL3Window::Resize(int width, int height) {
  width_ = width;
  height_ = height;

  if (client_) {
    auto browser = client_->GetFirstBrowser();
    if (browser) {
      // Apply DPI scaling for CEF browser resize
      int scaled_width = static_cast<int>(width / dpi_scale_);
      int scaled_height = static_cast<int>(height / dpi_scale_);
      browser->GetHost()->WasResized();
    }
  }

  Logger::LogMessage("Window resized to " + std::to_string(width) + "x" +
                     std::to_string(height));
}

void SDL3Window::Render() {
  if (!renderer_) {
    return;
  }

  // Clear the screen
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
  SDL_RenderClear(renderer_);

  // Render main texture
  if (texture_) {
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
  }

  // Render editor overlay if enabled
  if (editor_enabled_ && editor_texture_) {
    SDL_FRect editor_dst = {
        static_cast<float>(editor_rect_.x),
        static_cast<float>(editor_rect_.y),
        static_cast<float>(editor_rect_.width),
        static_cast<float>(editor_rect_.height)
    };
    SDL_RenderTexture(renderer_, editor_texture_, nullptr, &editor_dst);
  }

  SDL_RenderPresent(renderer_);
}

bool SDL3Window::HandleWindowDragging(const SDL_Event &event) {
  if (!client_)
    return false;

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

        Logger::LogMessage(
            "Window dragging started at (" + std::to_string(event.button.x) +
            ", " + std::to_string(event.button.y) + ") - in draggable region");
        return true; // Consume the event
      } else {
        // Point is not in a draggable region, don't start dragging
        // Let the event pass through to CEF for normal interaction
        Logger::LogMessage("Mouse click at (" + std::to_string(event.button.x) +
                           ", " + std::to_string(event.button.y) +
                           ") - not in draggable region, passing to CEF");
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

// Editor sublayer management methods
void SDL3Window::EnableEditor(bool enable) {
  editor_enabled_ = enable;
  Logger::LogMessage("Editor sublayer " +
                     std::string(enable ? "enabled" : "disabled"));
}

void SDL3Window::SetEditorPosition(int x, int y, int width, int height) {
  editor_rect_.x = x;
  editor_rect_.y = y;
  editor_rect_.width = width;
  editor_rect_.height = height;
  Logger::LogMessage("Editor position set to (" + std::to_string(x) + ", " +
                     std::to_string(y) + ") with size " +
                     std::to_string(width) + "x" + std::to_string(height));
}

void SDL3Window::SetEditorBrowser(CefRefPtr<CefBrowser> browser) {
  editor_browser_ = browser;
  Logger::LogMessage("Editor browser reference set");
}

void SDL3Window::ResizeMenuOverlay(int height) {
  Logger::LogMessage("ResizeMenuOverlay called with height: " + std::to_string(height));
  
  if (!menu_overlay_visible_) {
    Logger::LogMessage("ResizeMenuOverlay: Menu overlay is not visible, ignoring resize request");
    return;
  }
  
  // Get the correct hardcoded height for the current menu section
  int correctHeight = height;
  if (!current_menu_section_.empty()) {
    if (current_menu_section_ == "File") {
      correctHeight = 346;
    } else if (current_menu_section_ == "Edit") {
      correctHeight = 610;
    } else if (current_menu_section_ == "View") {
      correctHeight = 274;
    } else if (current_menu_section_ == "Navigate") {
      correctHeight = 250;
    } else if (current_menu_section_ == "Selection") {
      correctHeight = 226;
    } else if (current_menu_section_ == "Tools") {
      correctHeight = 298;
    } else if (current_menu_section_ == "Window") {
      correctHeight = 226;
    } else if (current_menu_section_ == "Help") {
      correctHeight = 130;
    } else {
      correctHeight = 300; // Default height for unknown sections
    }
    
    Logger::LogMessage("ResizeMenuOverlay: Using hardcoded height " + std::to_string(correctHeight) + 
                       " for section '" + current_menu_section_ + "' (requested: " + std::to_string(height) + ")");
  }
  
  if (correctHeight <= 0) {
    Logger::LogMessage("ResizeMenuOverlay: Invalid height " + std::to_string(correctHeight) + ", ignoring resize request");
    return;
  }
  
  // Update menu overlay dimensions
  // The overlay position (x, y) remains the same, only height changes
  Logger::LogMessage("ResizeMenuOverlay: Resizing menu overlay to height " + std::to_string(correctHeight) + 
                     " at position (" + std::to_string(menu_overlay_x_) + ", " + std::to_string(menu_overlay_y_) + ")");
  
  // Note: The actual window resizing would be handled by the CEF browser
  // This method serves as a notification that the content height has changed
  // The browser window itself will be resized by CEF based on the content
}

void SDL3Window::UpdateEditorTexture(const void *buffer, int width,
                                     int height) {
  Logger::LogMessage(
      "UpdateEditorTexture called: " + std::to_string(width) + "x" +
      std::to_string(height) +
      ", editor_enabled_=" + (editor_enabled_ ? "true" : "false") +
      ", editor_browser_=" + (editor_browser_ ? "valid" : "null"));

  if (!buffer) {
    Logger::LogMessage("UpdateEditorTexture: Buffer is null!");
    return;
  }

  if (width <= 0 || height <= 0) {
    Logger::LogMessage("UpdateEditorTexture: Invalid dimensions - width: " +
                       std::to_string(width) +
                       ", height: " + std::to_string(height));
    return;
  }

  if (!editor_enabled_ || !editor_browser_) {
    Logger::LogMessage("UpdateEditorTexture: Early return - editor not enabled "
                       "or browser null");
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
  if (!editor_texture_ || current_texture_width != width ||
      current_texture_height != height) {

    Logger::LogMessage("UpdateEditorTexture: Creating new texture (old: " +
                       std::to_string(current_texture_width) + "x" +
                       std::to_string(current_texture_height) +
                       ", new: " + std::to_string(width) + "x" +
                       std::to_string(height) + ")");

    if (editor_texture_) {
      SDL_DestroyTexture(editor_texture_);
      Logger::LogMessage("UpdateEditorTexture: Destroyed old texture");
    }

    // Try RGBA32 format first, then fallback to BGRA32
    editor_texture_ =
        SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32,
                          SDL_TEXTUREACCESS_STREAMING, width, height);

    if (!editor_texture_) {
      Logger::LogMessage("UpdateEditorTexture: RGBA32 failed, trying BGRA32");
      editor_texture_ =
          SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32,
                            SDL_TEXTUREACCESS_STREAMING, width, height);
    }

    if (!editor_texture_) {
      const char *error = SDL_GetError();
      Logger::LogMessage("UpdateEditorTexture: Failed to create editor texture "
                         "with both formats: " +
                         std::string(error ? error : "Unknown error"));
      return;
    }

    // Enable alpha blending for the editor texture
    int blend_result =
        SDL_SetTextureBlendMode(editor_texture_, SDL_BLENDMODE_BLEND);
    if (blend_result != 0) {
      const char *error = SDL_GetError();
      Logger::LogMessage(
          "UpdateEditorTexture: Failed to set blend mode (result=" +
          std::to_string(blend_result) +
          "): " + std::string(error ? error : "Unknown error"));
      // Continue anyway - blend mode failure is not critical
    } else {
      Logger::LogMessage("UpdateEditorTexture: Successfully set blend mode");
    }

    // Update stored dimensions
    current_texture_width = width;
    current_texture_height = height;

    Logger::LogMessage(
        "UpdateEditorTexture: Successfully created editor texture: " +
        std::to_string(width) + "x" + std::to_string(height));
  }

  // Update texture with CEF buffer data
  void *pixels;
  int pitch;

  // Clear any previous SDL errors
  SDL_ClearError();

  bool lock_success =
      SDL_LockTexture(editor_texture_, nullptr, &pixels, &pitch);
  if (lock_success) {
    Logger::LogMessage(
        "UpdateEditorTexture: Successfully locked texture - pitch=" +
        std::to_string(pitch) +
        ", expected_row_bytes=" + std::to_string(width * 4));

    if (!pixels) {
      Logger::LogMessage(
          "UpdateEditorTexture: Locked texture but pixels pointer is null");
      SDL_UnlockTexture(editor_texture_);
      return;
    }

    // Copy row by row to handle potential pitch differences
    const uint8_t *src = static_cast<const uint8_t *>(buffer);
    uint8_t *dst = static_cast<uint8_t *>(pixels);
    const int rowBytes = width * 4; // 4 bytes per pixel (BGRA)

    if (pitch < rowBytes) {
      Logger::LogMessage("UpdateEditorTexture: Invalid pitch " +
                         std::to_string(pitch) + " < " +
                         std::to_string(rowBytes));
      SDL_UnlockTexture(editor_texture_);
      return;
    }

    for (int y = 0; y < height; y++) {
      memcpy(dst + y * pitch, src + y * rowBytes, rowBytes);
    }

    SDL_UnlockTexture(editor_texture_);
    Logger::LogMessage(
        "UpdateEditorTexture: Successfully updated texture data");
  } else {
    const char *error = SDL_GetError();
    Logger::LogMessage("UpdateEditorTexture: Failed to lock editor texture: " +
                       std::string(error ? error : "Unknown error"));

    // If texture is already locked, this might be a threading issue
    // Try to unlock it first and then retry once
    if (error &&
        std::string(error).find("already locked") != std::string::npos) {
      Logger::LogMessage("UpdateEditorTexture: Texture already locked, "
                         "attempting to unlock and retry");
      SDL_UnlockTexture(editor_texture_);
      SDL_ClearError();

      // Retry lock once
      lock_success = SDL_LockTexture(editor_texture_, nullptr, &pixels, &pitch);
      if (lock_success) {
        Logger::LogMessage(
            "UpdateEditorTexture: Retry lock successful - pitch=" +
            std::to_string(pitch));

        if (pixels) {
          const uint8_t *src = static_cast<const uint8_t *>(buffer);
          uint8_t *dst = static_cast<uint8_t *>(pixels);
          const int rowBytes = width * 4;

          if (pitch >= rowBytes) {
            for (int y = 0; y < height; y++) {
              memcpy(dst + y * pitch, src + y * rowBytes, rowBytes);
            }
            Logger::LogMessage("UpdateEditorTexture: Successfully updated "
                               "texture data on retry");
          } else {
            Logger::LogMessage("UpdateEditorTexture: Invalid pitch on retry " +
                               std::to_string(pitch) + " < " +
                               std::to_string(rowBytes));
          }
        }
        SDL_UnlockTexture(editor_texture_);
      } else {
        const char *retry_error = SDL_GetError();
        Logger::LogMessage(
            "UpdateEditorTexture: Retry lock also failed: " +
            std::string(retry_error ? retry_error : "Unknown error"));
      }
    }
  }
}

// Menu overlay position and state setter methods
void SDL3Window::SetMenuOverlayPosition(int x, int y) {
  menu_overlay_x_ = x;
  menu_overlay_y_ = y;
}

void SDL3Window::SetMenuOverlayVisible(bool visible) {
  menu_overlay_visible_ = visible;
}

void SDL3Window::SetCurrentMenuSection(const std::string& section) {
  current_menu_section_ = section;
}

// Getters
SDL_Window* SDL3Window::GetSDLWindow() const { return window_; }
SDL_Renderer* SDL3Window::GetRenderer() const { return renderer_; }
bool SDL3Window::ShouldClose() const { return should_close_; }
bool SDL3Window::IsMinimized() const { return minimized_; }
bool SDL3Window::IsMaximized() const { return maximized_; }
int SDL3Window::GetWidth() const { return width_; }
int SDL3Window::GetHeight() const { return height_; }
float SDL3Window::GetDPIScale() const { return dpi_scale_; }
int SDL3Window::GetMenuOverlayX() const { return menu_overlay_x_; }
int SDL3Window::GetMenuOverlayY() const { return menu_overlay_y_; }

void SDL3Window::SetClient(SimpleClient* client) { client_ = client; }