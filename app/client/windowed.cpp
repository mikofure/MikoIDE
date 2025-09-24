#include <dwmapi.h>
#include <shellapi.h>
#include <SDL3/SDL.h>
#include "windowed.hpp"
#include "mikoclient.hpp"

// DirectX 11 Renderer for performance optimization
#include "../renderer/dx11_renderer.hpp"
#include "../utils/config.hpp"

// Link dwmapi library
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
      editor_browser_(nullptr), editor_texture_(nullptr),
      editor_has_focus_(false), main_browser_has_focus_(true) {}

SDL3Window::~SDL3Window() { Shutdown(); }

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

  window_ = SDL_CreateWindow("Hyperion", width_, height_, window_flags);

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
  texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32,
                               SDL_TEXTUREACCESS_STREAMING, width_, height_);
  if (!texture_) {
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
    return false;
  }
  // IMPORTANT: Avoid blending (OSR may deliver alpha=0 everywhere)
  SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_NONE);

  // Get Windows HWND for dwmapi
  hwnd_ =
      (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window_),
                                   SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

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

  // Enable text input for the window
  SDL_StartTextInput(window_);

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
    SDL_StopTextInput(window_);
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

void SDL3Window::Close() { should_close_ = true; }

HWND SDL3Window::GetHWND() const { return hwnd_; }

void SDL3Window::ApplyRoundedCorners() {
  if (!hwnd_)
    return;

  // Apply rounded corners using DWM API (Windows 11 style)
  DWM_WINDOW_CORNER_PREFERENCE cornerPreference = DWMWCP_ROUND;
  DwmSetWindowAttribute(hwnd_, DWMWA_WINDOW_CORNER_PREFERENCE,
                        &cornerPreference, sizeof(cornerPreference));

  // Enable immersive dark mode if available
  BOOL darkMode = TRUE;
  DwmSetWindowAttribute(hwnd_, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode,
                        sizeof(darkMode));

  Logger::LogMessage("Applied rounded corners and dark mode to window");
}

void SDL3Window::SetBorderless(bool borderless) {
  borderless_ = borderless;
  UpdateWindowStyle();
}

void SDL3Window::UpdateWindowStyle() {
  if (!hwnd_)
    return;

  LONG_PTR style = GetWindowLongPtr(hwnd_, GWL_STYLE);
  if (borderless_) {
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
               WS_SYSMENU);
    style |= WS_POPUP;
  } else {
    style |= (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
              WS_SYSMENU);
    style &= ~WS_POPUP;
  }
  SetWindowLongPtr(hwnd_, GWL_STYLE, style);

  // Update extended style for proper taskbar appearance
  LONG_PTR exStyle = GetWindowLongPtr(hwnd_, GWL_EXSTYLE);
  exStyle |= WS_EX_APPWINDOW;
  SetWindowLongPtr(hwnd_, GWL_EXSTYLE, exStyle);

  // Force window to redraw
  SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
               SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                   SWP_NOOWNERZORDER);
}

void SDL3Window::InitializeDwmApi() {
  if (!hwnd_)
    return;

  // Check DWM composition status (always enabled on Windows 10/11)
  BOOL composition_enabled = FALSE;
  DwmIsCompositionEnabled(&composition_enabled);

  // Extend frame into client area for better visual effects
  MARGINS margins = {0, 0, 0, 1};
  DwmExtendFrameIntoClientArea(hwnd_, &margins);
}

void SDL3Window::UpdateDPIScale() {
  if (!hwnd_) {
    dpi_scale_ = 1.0f;
    return;
  }

  // Get DPI for the window's monitor
  UINT dpi = GetDpiForWindow(hwnd_);
  dpi_scale_ =
      static_cast<float>(dpi) / 96.0f; // 96 DPI is the standard baseline

  Logger::LogMessage(
      "HiDPI: Detected DPI scale factor: " + std::to_string(dpi_scale_) +
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

bool SDL3Window::HandleEvent(const SDL_Event &event) {
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

      // Account for UI elements: title bar (32px) + navbar (96px) + status bar
      // (24px)
      int margin_x = 0;
      int margin_y = 32 + 96; // Title bar + navbar at top
      int margin_bottom = 24; // Status bar at bottom

      SetEditorPosition(margin_x, margin_y, new_width - margin_x,
                        new_height - margin_y - margin_bottom);

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
      // Account for UI elements: title bar (32px) + navbar (96px) + status bar
      // (24px)
      int margin_x = 0;
      int margin_y = 32 + 91; // Title bar + navbar at top
      int margin_bottom = 23; // Status bar at bottom

      SetEditorPosition(margin_x, margin_y, width_ - margin_x,
                        height_ - margin_y - margin_bottom);

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

    Logger::LogMessage("Window resized to " + std::to_string(width_) + "x" +
                       std::to_string(height_));

    // Check for DPI changes on resize (common trigger for DPI changes)
    UpdateDPIScale();

    // Recreate texture with new size
    if (texture_) {
      SDL_DestroyTexture(texture_);
      texture_ =
          SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32,
                            SDL_TEXTUREACCESS_STREAMING, width_, height_);
      if (!texture_) {
        Logger::LogMessage("Resize: Failed to recreate texture - " +
                           std::string(SDL_GetError()));
      } else {
        SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_NONE);
        Logger::LogMessage("Resize: Texture recreated successfully");
      }
    }

    // Update editor size to match new window dimensions
    if (editor_enabled_ && editor_browser_) {
      // Calculate new editor dimensions based on resized window
      // Account for UI elements: title bar (32px) + navbar (96px) + status bar
      // (24px)
      int margin_x = 0;
      int margin_y = 32 + 91; // Title bar + navbar at top
      int margin_bottom = 23; // Status bar at bottom

      SetEditorPosition(margin_x, margin_y, width_ - margin_x,
                        height_ - margin_y - margin_bottom);

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
        Logger::LogMessage(
            "Display change: Notified CEF browser of scale change");
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
    } catch (const std::exception &ex) {
      Logger::LogMessage("HandleEvent: Exception in SendMouseEvent - " +
                         std::string(ex.what()));
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

  case SDL_EVENT_TEXT_INPUT:
    SendTextInputEvent(event);
    return true;

  default:
    return false;
  }
}

bool SDL3Window::IsPointInEditor(int x, int y) const {
  return editor_enabled_ && x >= editor_rect_.x &&
         x < editor_rect_.x + editor_rect_.width && y >= editor_rect_.y &&
         y < editor_rect_.y + editor_rect_.height;
}

void SDL3Window::SendMouseEvent(const SDL_Event &event) {
  if (!client_) {
    Logger::LogMessage("SendMouseEvent: Client is null");
    return;
  }

  // Determine which browser to send the event to
  CefRefPtr<CefBrowser> target_browser;
  bool is_editor_event = IsPointInEditor(event.button.x, event.button.y);

  if (is_editor_event && editor_browser_) {
    target_browser = editor_browser_;
    Logger::LogMessage("SendMouseEvent: Routing to editor browser");
  } else {
    target_browser = client_->GetFirstBrowser();
    Logger::LogMessage("SendMouseEvent: Routing to main browser");
  }

  if (!target_browser) {
    Logger::LogMessage("SendMouseEvent: Target browser is null");
    return;
  }

  auto host = target_browser->GetHost();
  if (!host) {
    Logger::LogMessage("SendMouseEvent: Browser host is null");
    return;
  }

  CefMouseEvent mouse_event;
  mouse_event.x = event.button.x;
  mouse_event.y = event.button.y;

  // Get current modifier state from SDL and convert to CEF modifiers
  SDL_Keymod sdl_modifiers = SDL_GetModState();
  mouse_event.modifiers = 0;

  if (sdl_modifiers & SDL_KMOD_SHIFT) {
    mouse_event.modifiers |= EVENTFLAG_SHIFT_DOWN;
  }
  if (sdl_modifiers & SDL_KMOD_CTRL) {
    mouse_event.modifiers |= EVENTFLAG_CONTROL_DOWN;
  }
  if (sdl_modifiers & SDL_KMOD_ALT) {
    mouse_event.modifiers |= EVENTFLAG_ALT_DOWN;
  }
  if (sdl_modifiers & SDL_KMOD_GUI) {
    mouse_event.modifiers |= EVENTFLAG_COMMAND_DOWN;
  }

  // If this is an editor event, adjust coordinates relative to editor area
  if (is_editor_event) {
    mouse_event.x -= editor_rect_.x;
    mouse_event.y -= editor_rect_.y;

    // Validate coordinates are within editor bounds
    if (mouse_event.x < 0 || mouse_event.y < 0 ||
        mouse_event.x > editor_rect_.width ||
        mouse_event.y > editor_rect_.height) {
      Logger::LogMessage(
          "SendMouseEvent: Editor mouse coordinates out of bounds (" +
          std::to_string(mouse_event.x) + ", " + std::to_string(mouse_event.y) +
          ") editor size: " + std::to_string(editor_rect_.width) + "x" +
          std::to_string(editor_rect_.height));
      return;
    }
  } else {
    // Validate coordinates are within main window bounds
    if (mouse_event.x < 0 || mouse_event.y < 0 || mouse_event.x > width_ ||
        mouse_event.y > height_) {
      Logger::LogMessage(
          "SendMouseEvent: Main window mouse coordinates out of bounds (" +
          std::to_string(mouse_event.x) + ", " + std::to_string(mouse_event.y) +
          ")");
      return;
    }
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

      // Update focus tracking on mouse down events
      if (!mouse_up) {
        if (is_editor_event) {
          editor_has_focus_ = true;
          main_browser_has_focus_ = false;
          Logger::LogMessage("SendMouseEvent: Focus switched to editor");
        } else {
          editor_has_focus_ = false;
          main_browser_has_focus_ = true;
          Logger::LogMessage("SendMouseEvent: Focus switched to main browser");
        }
      }

      Logger::LogMessage("SendMouseEvent: Sending click event at (" +
                         std::to_string(mouse_event.x) + ", " +
                         std::to_string(mouse_event.y) +
                         ") button=" + std::to_string(button_type) +
                         " up=" + std::to_string(mouse_up) + " to " +
                         (is_editor_event ? "editor" : "main") + " browser");

      host->SendMouseClickEvent(mouse_event, button_type, mouse_up,
                                click_count);
      break;
    }
    }
  } catch (const std::exception &ex) {
    Logger::LogMessage("SendMouseEvent: Exception caught - " +
                       std::string(ex.what()));
  } catch (...) {
    Logger::LogMessage("SendMouseEvent: Unknown exception caught");
  }
}

void SDL3Window::SendKeyEvent(const SDL_Event &event) {
  if (!client_)
    return;

  // Determine which browser to send the event to based on focus state
  CefRefPtr<CefBrowser> target_browser;

  if (editor_has_focus_ && editor_browser_) {
    target_browser = editor_browser_;
    Logger::LogMessage("SendKeyEvent: Routing to editor browser (has focus)");
  } else if (main_browser_has_focus_) {
    target_browser = client_->GetFirstBrowser();
    Logger::LogMessage("SendKeyEvent: Routing to main browser (has focus)");
  } else {
    // Fallback to main browser if no focus is set
    target_browser = client_->GetFirstBrowser();
    Logger::LogMessage("SendKeyEvent: Routing to main browser (fallback)");
  }

  if (!target_browser)
    return;

  // Check for modifier keys
  SDL_Keymod modifiers = SDL_GetModState();
  bool ctrl_pressed = (modifiers & SDL_KMOD_CTRL) != 0;
  bool shift_pressed = (modifiers & SDL_KMOD_SHIFT) != 0;
  bool alt_pressed = (modifiers & SDL_KMOD_ALT) != 0;

  // Handle copy-paste shortcuts
  if (ctrl_pressed && event.type == SDL_EVENT_KEY_DOWN) {
    if (event.key.key == SDLK_C) {
      // Ctrl+C - Copy
      Logger::LogMessage("SendKeyEvent: Handling Ctrl+C copy command");
      target_browser->GetMainFrame()->Copy();
      return;
    } else if (event.key.key == SDLK_V) {
      // Ctrl+V - Paste
      Logger::LogMessage("SendKeyEvent: Handling Ctrl+V paste command");
      target_browser->GetMainFrame()->Paste();
      return;
    } else if (event.key.key == SDLK_X) {
      // Ctrl+X - Cut
      Logger::LogMessage("SendKeyEvent: Handling Ctrl+X cut command");
      target_browser->GetMainFrame()->Cut();
      return;
    } else if (event.key.key == SDLK_A) {
      // Ctrl+A - Select All
      Logger::LogMessage("SendKeyEvent: Handling Ctrl+A select all command");

      // Send the key event to CEF instead of using SelectAll() method
      CefKeyEvent select_all_event;
      select_all_event.type = KEYEVENT_KEYDOWN;
      select_all_event.windows_key_code = 'A';
      select_all_event.native_key_code = event.key.scancode;
      select_all_event.modifiers = EVENTFLAG_CONTROL_DOWN;

      target_browser->GetHost()->SendKeyEvent(select_all_event);
      return;
    } else if (event.key.key == SDLK_Z) {
      // Ctrl+Z - Undo
      Logger::LogMessage("SendKeyEvent: Handling Ctrl+Z undo command");
      target_browser->GetMainFrame()->Undo();
      return;
    } else if (event.key.key == SDLK_Y) {
      // Ctrl+Y - Redo
      Logger::LogMessage("SendKeyEvent: Handling Ctrl+Y redo command");
      target_browser->GetMainFrame()->Redo();
      return;
    }
  }

  CefKeyEvent key_event;
  key_event.windows_key_code = MapSDLKeyToWindowsVK(event.key.key);
  key_event.native_key_code = event.key.scancode;

  // Set modifiers properly
  key_event.modifiers = 0;
  if (ctrl_pressed)
    key_event.modifiers |= EVENTFLAG_CONTROL_DOWN;
  if (shift_pressed)
    key_event.modifiers |= EVENTFLAG_SHIFT_DOWN;
  if (alt_pressed)
    key_event.modifiers |= EVENTFLAG_ALT_DOWN;

  if (event.type == SDL_EVENT_KEY_DOWN) {
    key_event.type = KEYEVENT_KEYDOWN;
  } else {
    key_event.type = KEYEVENT_KEYUP;
  }

  target_browser->GetHost()->SendKeyEvent(key_event);

  // Special handling for Enter key to generate carriage return character
  if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_RETURN) {
    CefKeyEvent char_event;
    char_event.type = KEYEVENT_CHAR;
    char_event.character = '\r';
    char_event.unmodified_character = '\r';
    char_event.windows_key_code = '\r';
    char_event.native_key_code = 0;
    char_event.modifiers = 0;

    target_browser->GetHost()->SendKeyEvent(char_event);
    Logger::LogMessage(
        "SendKeyEvent: Generated carriage return character for Enter key");
  }
}

void SDL3Window::SendScrollEvent(const SDL_Event &event) {
  if (!client_)
    return;

  // Determine which browser to send the event to based on mouse position
  CefRefPtr<CefBrowser> target_browser;
  bool is_editor_event = IsPointInEditor(last_mouse_x_, last_mouse_y_);

  if (is_editor_event && editor_browser_) {
    target_browser = editor_browser_;
    Logger::LogMessage("SendScrollEvent: Routing to editor browser");
  } else {
    target_browser = client_->GetFirstBrowser();
    Logger::LogMessage("SendScrollEvent: Routing to main browser");
  }

  if (!target_browser)
    return;

  CefMouseEvent mouse_event;
  mouse_event.x = last_mouse_x_;
  mouse_event.y = last_mouse_y_;
  mouse_event.modifiers = 0;

  // If this is an editor event, adjust coordinates relative to editor area
  if (is_editor_event) {
    mouse_event.x -= editor_rect_.x;
    mouse_event.y -= editor_rect_.y;
  }

  int delta_x = 0;
  int delta_y = static_cast<int>(event.wheel.y * 120); // Standard wheel delta

  target_browser->GetHost()->SendMouseWheelEvent(mouse_event, delta_x, delta_y);
}

void SDL3Window::SendTextInputEvent(const SDL_Event &event) {
  if (!client_)
    return;

  // Determine which browser to send the event to based on focus state
  CefRefPtr<CefBrowser> target_browser;

  if (editor_has_focus_ && editor_browser_) {
    target_browser = editor_browser_;
    Logger::LogMessage(
        "SendTextInputEvent: Routing to editor browser (has focus)");
  } else if (main_browser_has_focus_) {
    target_browser = client_->GetFirstBrowser();
    Logger::LogMessage(
        "SendTextInputEvent: Routing to main browser (has focus)");
  } else {
    // Fallback to main browser if no focus is set
    target_browser = client_->GetFirstBrowser();
    Logger::LogMessage(
        "SendTextInputEvent: Routing to main browser (fallback)");
  }

  if (!target_browser)
    return;

  // Convert SDL text input to CEF key event
  const char *text = event.text.text;
  if (!text || strlen(text) == 0) {
    Logger::LogMessage("SendTextInputEvent: Empty text input");
    return;
  }

  // For each character in the text input
  for (int i = 0; text[i] != '\0'; i++) {
    CefKeyEvent key_event;
    key_event.type = KEYEVENT_CHAR;
    key_event.character = text[i];
    key_event.unmodified_character = text[i];
    key_event.windows_key_code = text[i];
    key_event.native_key_code = 0;
    key_event.modifiers = 0;

    target_browser->GetHost()->SendKeyEvent(key_event);

    Logger::LogMessage("SendTextInputEvent: Sent character '" +
                       std::string(1, text[i]) + "' to " +
                       (editor_has_focus_ ? "editor" : "main") + " browser");
  }
}

void SDL3Window::UpdateTexture(const void *buffer, int width, int height) {
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
    // Check if texture dimensions match the incoming data (use float types for
    // SDL3)
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

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32,
                                 SDL_TEXTUREACCESS_STREAMING, width, height);

    if (!texture_) {
      return;
    }
    // IMPORTANT: Use NONE blend mode for main texture to avoid interference
    // with editor overlay
    SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_NONE);
  }

  // 🔑 Copy CEF buffer → texture
  void *pixels = nullptr;
  int pitch = 0;
  if (SDL_LockTexture(texture_, nullptr, &pixels, &pitch)) {
    const uint8_t *src = static_cast<const uint8_t *>(buffer);
    const int rowBytes = width * 4; // BGRA32
    for (int y = 0; y < height; y++) {
      std::memcpy(static_cast<uint8_t *>(pixels) + y * pitch,
                  src + y * rowBytes, rowBytes);
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

    SDL_FRect editorDestRect = {(float)editor_rect_.x, (float)editor_rect_.y,
                                (float)editor_rect_.width,
                                (float)editor_rect_.height};

    SDL_RenderTexture(renderer_, editor_texture_, nullptr, &editorDestRect);
  }

  // 3. Present final frame
  if (renderer_) {
    SDL_RenderPresent(renderer_);
  }
}

bool SDL3Window::HandleWindowDragging(const SDL_Event &event) {
  if (!client_)
    return false;

  switch (event.type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    if (event.button.button == SDL_BUTTON_LEFT) {
      bool in_draggable_region = false;

      // Check if the click is in a draggable region
      // First check main browser draggable regions
      if (client_->IsPointInDragRegion(event.button.x, event.button.y)) {
        in_draggable_region = true;
      }
      // If editor is active and mouse is in editor area, check if it's in title
      // bar region
      else if (editor_enabled_ &&
               IsPointInEditor(event.button.x, event.button.y)) {
        // For editor, assume title bar area (top 32px) is draggable
        // This matches the margin_y calculation used elsewhere
        int title_bar_height = 32;
        if (event.button.y >= editor_rect_.y &&
            event.button.y < editor_rect_.y + title_bar_height) {
          in_draggable_region = true;
          Logger::LogMessage(
              "Mouse click in editor title bar area - allowing drag");
        }
      }

      if (in_draggable_region) {
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
  Logger::LogMessage("ResizeMenuOverlay called with height: " +
                     std::to_string(height));

  if (!menu_overlay_visible_) {
    Logger::LogMessage("ResizeMenuOverlay: Menu overlay is not visible, "
                       "ignoring resize request");
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

    Logger::LogMessage("ResizeMenuOverlay: Using hardcoded height " +
                       std::to_string(correctHeight) + " for section '" +
                       current_menu_section_ +
                       "' (requested: " + std::to_string(height) + ")");
  }

  if (correctHeight <= 0) {
    Logger::LogMessage("ResizeMenuOverlay: Invalid height " +
                       std::to_string(correctHeight) +
                       ", ignoring resize request");
    return;
  }

  // Update menu overlay dimensions
  // The overlay position (x, y) remains the same, only height changes
  Logger::LogMessage("ResizeMenuOverlay: Resizing menu overlay to height " +
                     std::to_string(correctHeight) + " at position (" +
                     std::to_string(menu_overlay_x_) + ", " +
                     std::to_string(menu_overlay_y_) + ")");

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

void SDL3Window::SetCurrentMenuSection(const std::string &section) {
  current_menu_section_ = section;
}
// Key mapping function to convert SDL key codes to Windows virtual key codes
int SDL3Window::MapSDLKeyToWindowsVK(SDL_Keycode sdl_key) const {
  switch (sdl_key) {
  // Letters
  case SDLK_A:
    return 'A';
  case SDLK_B:
    return 'B';
  case SDLK_C:
    return 'C';
  case SDLK_D:
    return 'D';
  case SDLK_E:
    return 'E';
  case SDLK_F:
    return 'F';
  case SDLK_G:
    return 'G';
  case SDLK_H:
    return 'H';
  case SDLK_I:
    return 'I';
  case SDLK_J:
    return 'J';
  case SDLK_K:
    return 'K';
  case SDLK_L:
    return 'L';
  case SDLK_M:
    return 'M';
  case SDLK_N:
    return 'N';
  case SDLK_O:
    return 'O';
  case SDLK_P:
    return 'P';
  case SDLK_Q:
    return 'Q';
  case SDLK_R:
    return 'R';
  case SDLK_S:
    return 'S';
  case SDLK_T:
    return 'T';
  case SDLK_U:
    return 'U';
  case SDLK_V:
    return 'V';
  case SDLK_W:
    return 'W';
  case SDLK_X:
    return 'X';
  case SDLK_Y:
    return 'Y';
  case SDLK_Z:
    return 'Z';

  // Numbers
  case SDLK_0:
    return '0';
  case SDLK_1:
    return '1';
  case SDLK_2:
    return '2';
  case SDLK_3:
    return '3';
  case SDLK_4:
    return '4';
  case SDLK_5:
    return '5';
  case SDLK_6:
    return '6';
  case SDLK_7:
    return '7';
  case SDLK_8:
    return '8';
  case SDLK_9:
    return '9';

  // Special keys
  case SDLK_RETURN:
    return 0x0D; // VK_RETURN
  case SDLK_ESCAPE:
    return 0x1B; // VK_ESCAPE
  case SDLK_BACKSPACE:
    return 0x08; // VK_BACK
  case SDLK_TAB:
    return 0x09; // VK_TAB
  case SDLK_SPACE:
    return 0x20; // VK_SPACE
  case SDLK_DELETE:
    return 0x2E; // VK_DELETE
  case SDLK_HOME:
    return 0x24; // VK_HOME
  case SDLK_END:
    return 0x23; // VK_END
  case SDLK_PAGEUP:
    return 0x21; // VK_PRIOR
  case SDLK_PAGEDOWN:
    return 0x22; // VK_NEXT
  case SDLK_LEFT:
    return 0x25; // VK_LEFT
  case SDLK_UP:
    return 0x26; // VK_UP
  case SDLK_RIGHT:
    return 0x27; // VK_RIGHT
  case SDLK_DOWN:
    return 0x28; // VK_DOWN
  case SDLK_INSERT:
    return 0x2D; // VK_INSERT

  // Function keys
  case SDLK_F1:
    return 0x70; // VK_F1
  case SDLK_F2:
    return 0x71; // VK_F2
  case SDLK_F3:
    return 0x72; // VK_F3
  case SDLK_F4:
    return 0x73; // VK_F4
  case SDLK_F5:
    return 0x74; // VK_F5
  case SDLK_F6:
    return 0x75; // VK_F6
  case SDLK_F7:
    return 0x76; // VK_F7
  case SDLK_F8:
    return 0x77; // VK_F8
  case SDLK_F9:
    return 0x78; // VK_F9
  case SDLK_F10:
    return 0x79; // VK_F10
  case SDLK_F11:
    return 0x7A; // VK_F11
  case SDLK_F12:
    return 0x7B; // VK_F12

  // Modifier keys
  case SDLK_LSHIFT:
    return 0xA0; // VK_LSHIFT
  case SDLK_RSHIFT:
    return 0xA1; // VK_RSHIFT
  case SDLK_LCTRL:
    return 0xA2; // VK_LCONTROL
  case SDLK_RCTRL:
    return 0xA3; // VK_RCONTROL
  case SDLK_LALT:
    return 0xA4; // VK_LMENU
  case SDLK_RALT:
    return 0xA5; // VK_RMENU

  // Punctuation and symbols
  case SDLK_SEMICOLON:
    return 0xBA; // VK_OEM_1
  case SDLK_EQUALS:
    return 0xBB; // VK_OEM_PLUS
  case SDLK_COMMA:
    return 0xBC; // VK_OEM_COMMA
  case SDLK_MINUS:
    return 0xBD; // VK_OEM_MINUS
  case SDLK_PERIOD:
    return 0xBE; // VK_OEM_PERIOD
  case SDLK_SLASH:
    return 0xBF; // VK_OEM_2
  case SDLK_GRAVE:
    return 0xC0; // VK_OEM_3
  case SDLK_LEFTBRACKET:
    return 0xDB; // VK_OEM_4
  case SDLK_BACKSLASH:
    return 0xDC; // VK_OEM_5
  case SDLK_RIGHTBRACKET:
    return 0xDD; // VK_OEM_6
  case SDLK_APOSTROPHE:
    return 0xDE; // VK_OEM_7

  default:
    // For unmapped keys, return the SDL key code as-is
    return static_cast<int>(sdl_key);
  }
}

bool SDL3Window::IsDX11Available() const {
  return dx11_renderer_ && dx11_renderer_->IsInitialized();
}

void SDL3Window::SetClient(CefRefPtr<HyperionClient> client) {
  client_ = client;
}
