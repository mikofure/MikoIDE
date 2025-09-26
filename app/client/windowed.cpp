#include "windowed.hpp"
#include "mikoclient.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include "include/cef_render_handler.h"
#include <algorithm>

// Cross-platform renderer system
#include "../renderer/renderer_factory.hpp"
#include "../utils/config.hpp"

namespace {

SDL_Point GetEventPosition(const SDL_Event &event) {
  SDL_Point point{0, 0};
  switch (event.type) {
  case SDL_EVENT_MOUSE_MOTION:
    point.x = static_cast<int>(event.motion.x);
    point.y = static_cast<int>(event.motion.y);
    break;
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
  case SDL_EVENT_MOUSE_BUTTON_UP:
    point.x = static_cast<int>(event.button.x);
    point.y = static_cast<int>(event.button.y);
    break;
  case SDL_EVENT_MOUSE_WHEEL:
    point.x = 0;
    point.y = 0;
    break;
  default:
    break;
  }
  return point;
}

} // namespace

SDL3Window::SDL3Window()
    : window_(nullptr), renderer_(nullptr), texture_(nullptr), platform_window_(nullptr),
      client_(nullptr), width_(DEFAULT_WINDOW_WIDTH),
      height_(DEFAULT_WINDOW_HEIGHT), minimized_(false), maximized_(false),
      should_close_(false), borderless_(true), mouse_captured_(false),
      last_mouse_x_(0), last_mouse_y_(0), is_dragging_(false), drag_start_x_(0),
      drag_start_y_(0), window_start_x_(0), window_start_y_(0),
      cross_platform_renderer_(nullptr), dpi_scale_(1.0f),
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

  // Initialize platform-specific window features
  platform_window_ = PlatformFactory::CreatePlatformWindow();
  if (platform_window_ && platform_window_->Initialize(window_)) {
    platform_window_->SetRoundedCorners(true);
    platform_window_->SetDarkMode(true);
    platform_window_->SetBorderless(borderless_);
    platform_window_->ExtendFrameIntoClientArea();
  }

  // Initialize cross-platform renderer for performance boost
  cross_platform_renderer_ = RendererFactory::CreateRenderer(
      RendererFactory::RendererPreference::Performance
  );
  
  if (cross_platform_renderer_) {
    auto native_handle = platform_window_ ? platform_window_->GetNativeHandle() : nullptr;
    if (native_handle && cross_platform_renderer_->Initialize(native_handle, width_, height_)) {
      Logger::LogMessage("Cross-platform renderer initialized successfully: " + 
                         cross_platform_renderer_->GetRendererName());
    } else {
      Logger::LogMessage("Failed to initialize cross-platform renderer");
      cross_platform_renderer_.reset();
    }
  }

  // Initialize DPI scaling
  UpdateDPIScale();

  // Enable text input for the window
  SDL_StartTextInput(window_);

  if (!SDL_SetWindowHitTest(window_, HitTestCallback, this)) {
    const char *error = SDL_GetError();
    Logger::Warning("SDL_SetWindowHitTest failed: " +
                    std::string(error ? error : "unknown"));
  }

  return true;
}

void SDL3Window::Shutdown() {
  // Cleanup cross-platform renderer first
  if (cross_platform_renderer_) {
    cross_platform_renderer_->Shutdown();
    cross_platform_renderer_.reset();
  }

  // Cleanup platform window
  if (platform_window_) {
    platform_window_->Shutdown();
    platform_window_.reset();
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

PlatformWindowHandle SDL3Window::GetNativeHandle() const { 
  return platform_window_ ? platform_window_->GetNativeHandle() : nullptr; 
}

void SDL3Window::ApplyRoundedCorners() {
  if (platform_window_) {
    platform_window_->SetRoundedCorners(true);
    platform_window_->SetDarkMode(true);
    Logger::LogMessage("Applied rounded corners and dark mode to window");
  }
}

void SDL3Window::SetBorderless(bool borderless) {
  borderless_ = borderless;
  if (platform_window_) {
    platform_window_->SetBorderless(borderless);
  }
}

void SDL3Window::UpdateDPIScale() {
  if (platform_window_) {
    platform_window_->UpdateDPIScale();
    dpi_scale_ = platform_window_->GetDPIScale();
  } else {
    dpi_scale_ = 1.0f;
  }

  Logger::LogMessage(
      "HiDPI: Detected DPI scale factor: " + std::to_string(dpi_scale_));

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

  case SDL_EVENT_TEXT_EDITING:
    SendTextEditingEvent(event);
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

CefRefPtr<CefBrowser> SDL3Window::GetFocusedBrowser() {
  if (editor_has_focus_ && editor_browser_) {
    return editor_browser_;
  }

  if (main_browser_has_focus_ && client_) {
    if (auto browser = client_->GetFirstBrowser()) {
      return browser;
    }
  }

  if (editor_browser_) {
    return editor_browser_;
  }

  if (client_) {
    return client_->GetFirstBrowser();
  }

  return nullptr;
}

void SDL3Window::SendMouseEvent(const SDL_Event &event) {
  if (!client_) {
    Logger::LogMessage("SendMouseEvent: Client is null");
    return;
  }

  SDL_Point window_point = GetEventPosition(event);
  bool is_editor_event = IsPointInEditor(window_point.x, window_point.y);

  CefRefPtr<CefBrowser> target_browser;
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

  CefMouseEvent mouse_event{};
  mouse_event.x = window_point.x;
  mouse_event.y = window_point.y;
  mouse_event.modifiers = 0;

  SDL_Keymod sdl_modifiers = SDL_GetModState();
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

  Uint32 mouse_buttons = SDL_GetMouseState(nullptr, nullptr);
  if (mouse_buttons & SDL_BUTTON_LMASK) {
    mouse_event.modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
  }
  if (mouse_buttons & SDL_BUTTON_MMASK) {
    mouse_event.modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
  }
  if (mouse_buttons & SDL_BUTTON_RMASK) {
    mouse_event.modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
  }

  if (is_editor_event) {
    mouse_event.x -= editor_rect_.x;
    mouse_event.y -= editor_rect_.y;

    if (mouse_event.x < 0 || mouse_event.y < 0 ||
        mouse_event.x > editor_rect_.width ||
        mouse_event.y > editor_rect_.height) {
      Logger::LogMessage("SendMouseEvent: Editor mouse coordinates out of bounds");
      return;
    }
  } else {
    if (mouse_event.x < 0 || mouse_event.y < 0 || mouse_event.x > width_ ||
        mouse_event.y > height_) {
      Logger::LogMessage("SendMouseEvent: Main window mouse coordinates out of bounds");
      return;
    }
  }

  last_mouse_x_ = window_point.x;
  last_mouse_y_ = window_point.y;

  try {
    switch (event.type) {
    case SDL_EVENT_MOUSE_MOTION: {
      host->SendMouseMoveEvent(mouse_event, false);
      break;
    }
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

      if (!mouse_up) {
        if (is_editor_event) {
          editor_has_focus_ = true;
          main_browser_has_focus_ = false;
        } else {
          editor_has_focus_ = false;
          main_browser_has_focus_ = true;
        }

        host->SetFocus(true);

        if (is_editor_event) {
          if (auto main_browser = client_ ? client_->GetFirstBrowser() : nullptr) {
            if (main_browser && main_browser != target_browser) {
              main_browser->GetHost()->SetFocus(false);
            }
          }
        } else if (editor_browser_ && editor_browser_ != target_browser) {
          editor_browser_->GetHost()->SetFocus(false);
        }
      }

      Logger::LogMessage("SendMouseEvent: Sending click event at (" +
                         std::to_string(mouse_event.x) + ", " +
                         std::to_string(mouse_event.y) + ")");

      host->SendMouseClickEvent(mouse_event, button_type, mouse_up, click_count);
      break;
    }
    default:
      break;
    }
  } catch (const std::exception &ex) {
    Logger::LogMessage("SendMouseEvent: Exception caught - " + std::string(ex.what()));
  } catch (...) {
    Logger::LogMessage("SendMouseEvent: Unknown exception caught");
  }
}

void SDL3Window::SendKeyEvent(const SDL_Event &event) {
  if (!client_) {
    return;
  }

  CefRefPtr<CefBrowser> target_browser = GetFocusedBrowser();
  if (!target_browser) {
    return;
  }

  auto host = target_browser->GetHost();
  if (!host) {
    return;
  }

  host->SetFocus(true);

  SDL_Keymod modifiers = SDL_GetModState();
  bool ctrl_pressed = (modifiers & SDL_KMOD_CTRL) != 0;
  bool shift_pressed = (modifiers & SDL_KMOD_SHIFT) != 0;
  bool alt_pressed = (modifiers & SDL_KMOD_ALT) != 0;

  if (ctrl_pressed && event.type == SDL_EVENT_KEY_DOWN) {
    switch (event.key.key) {
    case SDLK_C:
      Logger::LogMessage("SendKeyEvent: Handling Ctrl+C copy command");
      target_browser->GetMainFrame()->Copy();
      break;
    case SDLK_V:
      Logger::LogMessage("SendKeyEvent: Handling Ctrl+V paste command");
      target_browser->GetMainFrame()->Paste();
      break;
    case SDLK_X:
      Logger::LogMessage("SendKeyEvent: Handling Ctrl+X cut command");
      target_browser->GetMainFrame()->Cut();
      break;
    case SDLK_A: {
      Logger::LogMessage("SendKeyEvent: Handling Ctrl+A select all command");
      target_browser->GetMainFrame()->SelectAll();
      break;
    }
    case SDLK_Z:
      Logger::LogMessage("SendKeyEvent: Handling Ctrl+Z undo command");
      target_browser->GetMainFrame()->Undo();
      break;
    case SDLK_Y:
      Logger::LogMessage("SendKeyEvent: Handling Ctrl+Y redo command");
      target_browser->GetMainFrame()->Redo();
      break;
    default:
      break;
    }
  }

  CefKeyEvent key_event{};
  key_event.windows_key_code = MapSDLKeyToWindowsVK(event.key.key);
  key_event.native_key_code = event.key.scancode;
  key_event.modifiers = 0;

  if (ctrl_pressed) {
    key_event.modifiers |= EVENTFLAG_CONTROL_DOWN;
  }
  if (shift_pressed) {
    key_event.modifiers |= EVENTFLAG_SHIFT_DOWN;
  }
  if (alt_pressed) {
    key_event.modifiers |= EVENTFLAG_ALT_DOWN;
  }

  key_event.type = (event.type == SDL_EVENT_KEY_DOWN) ? KEYEVENT_KEYDOWN
                                                      : KEYEVENT_KEYUP;

  host->SendKeyEvent(key_event);

  if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_RETURN) {
    CefKeyEvent char_event{};
    char_event.type = KEYEVENT_CHAR;
    char_event.character = '\r';
    char_event.unmodified_character = '\r';
    char_event.windows_key_code = '\r';
    char_event.native_key_code = 0;
    char_event.modifiers = 0;

    host->SendKeyEvent(char_event);
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

  CefMouseEvent mouse_event{};
  mouse_event.x = last_mouse_x_;
  mouse_event.y = last_mouse_y_;
  mouse_event.modifiers = 0;

  SDL_Keymod sdl_modifiers = SDL_GetModState();
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

  Uint32 mouse_buttons = SDL_GetMouseState(nullptr, nullptr);
  if (mouse_buttons & SDL_BUTTON_LMASK) {
    mouse_event.modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
  }
  if (mouse_buttons & SDL_BUTTON_MMASK) {
    mouse_event.modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
  }
  if (mouse_buttons & SDL_BUTTON_RMASK) {
    mouse_event.modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
  }

  if (is_editor_event) {
    mouse_event.x -= editor_rect_.x;
    mouse_event.y -= editor_rect_.y;
  }

  int delta_x = static_cast<int>(event.wheel.x * 120);
  int delta_y = static_cast<int>(event.wheel.y * 120);

  target_browser->GetHost()->SendMouseWheelEvent(mouse_event, delta_x, delta_y);
}

void SDL3Window::SendTextInputEvent(const SDL_Event &event) {
  if (!client_) {
    return;
  }

  CefRefPtr<CefBrowser> target_browser = GetFocusedBrowser();
  if (!target_browser) {
    return;
  }

  auto host = target_browser->GetHost();
  if (!host) {
    return;
  }

  const char *text = event.text.text;
  if (!text || *text == '\0') {
    Logger::LogMessage("SendTextInputEvent: Empty text input");
    return;
  }

  std::string utf8_text(text);
  CefString cef_text(utf8_text);

  const int no_replace = std::numeric_limits<int>::max();
  host->ImeCommitText(cef_text, CefRange(no_replace, no_replace), 0);
  host->ImeFinishComposingText(false);
}

void SDL3Window::SendTextEditingEvent(const SDL_Event &event) {
  if (!client_) {
    return;
  }

  CefRefPtr<CefBrowser> target_browser = GetFocusedBrowser();
  if (!target_browser) {
    return;
  }

  auto host = target_browser->GetHost();
  if (!host) {
    return;
  }

  std::string composition_text = event.edit.text;
  CefString cef_text(composition_text);
  const int text_length = static_cast<int>(cef_text.length());

  std::vector<CefCompositionUnderline> underlines;
  if (text_length > 0) {
    CefCompositionUnderline underline;
    underline.range = CefRange(0, text_length);
    underline.color = 0xFF000000;
    underline.background_color = 0x00000000;
    underline.thick = false;
    underlines.push_back(underline);
  }

  int composition_start = std::max(0, event.edit.start);
  composition_start = std::min(composition_start, text_length);

  int composition_length = std::max(0, event.edit.length);
  composition_length = std::min(composition_length, text_length - composition_start);

  int caret_position = composition_start + composition_length;
  caret_position = std::clamp(caret_position, composition_start,
                              composition_start + composition_length);

  CefRange replacement_range(composition_start,
                              composition_start + composition_length);
  CefRange selection_range(caret_position, caret_position);

  host->ImeSetComposition(cef_text, underlines, replacement_range,
                          selection_range);
}

bool SDL3Window::UpdateTexture(const void* buffer, int width, int height) {
    if (!buffer || width <= 0 || height <= 0) {
        Logger::Error("UpdateTexture: Invalid parameters");
        return false;
    }

    // Priority: Use cross-platform renderer for optimal performance
    if (cross_platform_renderer_ && cross_platform_renderer_->IsInitialized()) {
        bool success = cross_platform_renderer_->UpdateTexture(buffer, width, height);
        if (success) {
            return true;
        }
        Logger::Warning("UpdateTexture: Cross-platform renderer failed, falling back to SDL3");
    }

    // Optimized SDL3 fallback with texture caching
    if (!renderer_) {
        Logger::Error("UpdateTexture: SDL renderer is null");
        return false;
    }

    // Check if we need to recreate texture (avoid frequent recreation)
    bool needNewTexture = false;
    if (!texture_) {
        needNewTexture = true;
    } else {
        int currentWidth, currentHeight;
        float texWidth, texHeight;
        if (SDL_GetTextureSize(texture_, &texWidth, &texHeight)) {
            currentWidth = (int)texWidth;
            currentHeight = (int)texHeight;
            if (currentWidth != width || currentHeight != height) {
                needNewTexture = true;
            }
        } else {
            needNewTexture = true;
        }
    }

    if (needNewTexture) {
        if (texture_) {
            SDL_DestroyTexture(texture_);
        }
        
        // Create optimized texture for 120 FPS performance
        texture_ = SDL_CreateTexture(renderer_, 
                                   SDL_PIXELFORMAT_BGRA32,
                                   SDL_TEXTUREACCESS_STREAMING,  // Optimized for frequent updates
                                   width, height);
        if (!texture_) {
            Logger::Error("UpdateTexture: Failed to create SDL texture: " + std::string(SDL_GetError()));
            return false;
        }

        // Optimize texture for performance
        SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_NONE);  // Disable blending for main texture
        SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_LINEAR);  // Better scaling quality
        
        Logger::LogMessage("UpdateTexture: Created new texture " + 
                          std::to_string(width) + "x" + std::to_string(height));
    }

    // Fast texture update with optimized locking
    void* pixels;
    int pitch;
    
    // Use non-blocking lock for 120 FPS performance
    if (!SDL_LockTexture(texture_, nullptr, &pixels, &pitch)) {
        Logger::Error("UpdateTexture: Failed to lock texture: " + std::string(SDL_GetError()));
        return false;
    }

    // Optimized memory copy
    const uint8_t* srcData = static_cast<const uint8_t*>(buffer);
    uint8_t* dstData = static_cast<uint8_t*>(pixels);
    const int srcPitch = width * 4;  // BGRA format
    
    if (pitch == srcPitch) {
        // Direct memory copy when pitches match (fastest path)
        std::memcpy(dstData, srcData, height * srcPitch);
    } else {
        // Row-by-row copy when pitches differ
        for (int y = 0; y < height; ++y) {
            std::memcpy(dstData + y * pitch, srcData + y * srcPitch, srcPitch);
        }
    }

    SDL_UnlockTexture(texture_);
    return true;
}

bool SDL3Window::UpdateEditorTexture(const void* buffer, int width, int height) {
    if (!buffer || width <= 0 || height <= 0) {
        Logger::Error("UpdateEditorTexture: Invalid parameters");
        return false;
    }

    if (!renderer_) {
        Logger::Error("UpdateEditorTexture: SDL renderer is null");
        return false;
    }

    // Optimized editor texture management
    bool needNewTexture = false;
    if (!editor_texture_) {
        needNewTexture = true;
    } else {
        int currentWidth, currentHeight;
        float texWidth, texHeight;
        if (SDL_GetTextureSize(editor_texture_, &texWidth, &texHeight)) {
            currentWidth = (int)texWidth;
            currentHeight = (int)texHeight;
            if (currentWidth != width || currentHeight != height) {
                needNewTexture = true;
            }
        } else {
            needNewTexture = true;
        }
    }

    if (needNewTexture) {
        if (editor_texture_) {
            SDL_DestroyTexture(editor_texture_);
        }
        
        // Create optimized editor texture
        editor_texture_ = SDL_CreateTexture(renderer_, 
                                          SDL_PIXELFORMAT_BGRA32,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          width, height);
        if (!editor_texture_) {
            Logger::Error("UpdateEditorTexture: Failed to create texture: " + std::string(SDL_GetError()));
            return false;
        }

        // Enable alpha blending for editor overlay
        SDL_SetTextureBlendMode(editor_texture_, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(editor_texture_, SDL_SCALEMODE_LINEAR);
        
        Logger::LogMessage("UpdateEditorTexture: Created new editor texture " + 
                          std::to_string(width) + "x" + std::to_string(height));
    }

    // Fast texture update
    void* pixels;
    int pitch;
    
    if (!SDL_LockTexture(editor_texture_, nullptr, &pixels, &pitch)) {
        Logger::Error("UpdateEditorTexture: Failed to lock texture: " + std::string(SDL_GetError()));
        return false;
    }

    // Optimized memory copy for editor texture
    const uint8_t* srcData = static_cast<const uint8_t*>(buffer);
    uint8_t* dstData = static_cast<uint8_t*>(pixels);
    const int srcPitch = width * 4;
    
    if (pitch == srcPitch) {
        std::memcpy(dstData, srcData, height * srcPitch);
    } else {
        for (int y = 0; y < height; ++y) {
            std::memcpy(dstData + y * pitch, srcData + y * srcPitch, srcPitch);
        }
    }

    SDL_UnlockTexture(editor_texture_);
    return true;
}

bool SDL3Window::Render() {
    // Priority: Use cross-platform renderer for optimal 120 FPS performance
    if (cross_platform_renderer_ && cross_platform_renderer_->IsInitialized()) {
        if (cross_platform_renderer_->BeginFrame()) {
            cross_platform_renderer_->Render();
            cross_platform_renderer_->EndFrame();
            bool success = cross_platform_renderer_->Present();
            if (success) {
                return true;
            }
        }
        Logger::Warning("Render: Cross-platform renderer failed, falling back to SDL3");
    }

    // Optimized SDL3 fallback rendering
    if (!renderer_) {
        Logger::Error("Render: SDL renderer is null");
        return false;
    }

    // Fast clear with optimized color
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);  // Black background
    SDL_RenderClear(renderer_);

    // Render main texture with optimized settings
    if (texture_) {
        // Use hardware acceleration when available
        SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
    }

    // Render editor overlay if enabled
    if (editor_enabled_ && editor_texture_) {
        SDL_FRect editor_rect = {
            static_cast<float>(editor_rect_.x),
            static_cast<float>(editor_rect_.y),
            static_cast<float>(editor_rect_.width),
            static_cast<float>(editor_rect_.height)
        };
        
        // Ensure proper alpha blending for overlay
        SDL_RenderTexture(renderer_, editor_texture_, nullptr, &editor_rect);
    }

    // Present with optimized timing for 120 FPS
    SDL_RenderPresent(renderer_);
    
    return true;
}

bool SDL3Window::HandleWindowDragging(const SDL_Event &event) {
  if (!client_) {
    return false;
  }

  SDL_Point window_point = GetEventPosition(event);

  switch (event.type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event.button.button != SDL_BUTTON_LEFT) {
      break;
    }

    if (IsPointInEditor(window_point.x, window_point.y)) {
      return false;
    }

    bool in_drag_region = client_->IsPointInDragRegion(window_point.x, window_point.y);
    if (!in_drag_region && client_ && !client_->HasDraggableRegions() &&
        window_point.y >= 0 && window_point.y < 32) {
      in_drag_region = true;
    }

    if (!in_drag_region) {
      return false;
    }

    is_dragging_ = true;
    drag_start_x_ = window_point.x;
    drag_start_y_ = window_point.y;

    SDL_GetWindowPosition(window_, &window_start_x_, &window_start_y_);

    if (SDL_CaptureMouse(true) == 0) {
      mouse_captured_ = true;
    }

    Logger::LogMessage("Window dragging started at (" +
                        std::to_string(window_point.x) + ", " +
                        std::to_string(window_point.y) + ")");
    return true;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event.button.button == SDL_BUTTON_LEFT && is_dragging_) {
      is_dragging_ = false;
      if (mouse_captured_) {
        SDL_CaptureMouse(false);
        mouse_captured_ = false;
      }
      Logger::LogMessage("Window dragging stopped");
      return true;
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    if (is_dragging_) {
      float global_x = 0.0f;
      float global_y = 0.0f;
      SDL_GetGlobalMouseState(&global_x, &global_y);

      int offset_x = static_cast<int>(global_x) - (window_start_x_ + drag_start_x_);
      int offset_y = static_cast<int>(global_y) - (window_start_y_ + drag_start_y_);

      SDL_SetWindowPosition(window_, window_start_x_ + offset_x,
                            window_start_y_ + offset_y);
      return true;
    }
    break;
  }
  default:
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
SDL_HitTestResult SDL3Window::HitTest(const SDL_Point &point) const {
  const int border = kResizeBorder;
  const bool left = point.x >= 0 && point.x < border;
  const bool right = point.x <= width_ && point.x > width_ - border;
  const bool top = point.y >= 0 && point.y < border;
  const bool bottom = point.y <= height_ && point.y > height_ - border;

  if (top && left) {
    return SDL_HITTEST_RESIZE_TOPLEFT;
  }
  if (top && right) {
    return SDL_HITTEST_RESIZE_TOPRIGHT;
  }
  if (bottom && left) {
    return SDL_HITTEST_RESIZE_BOTTOMLEFT;
  }
  if (bottom && right) {
    return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
  }
  if (top) {
    return SDL_HITTEST_RESIZE_TOP;
  }
  if (bottom) {
    return SDL_HITTEST_RESIZE_BOTTOM;
  }
  if (left) {
    return SDL_HITTEST_RESIZE_LEFT;
  }
  if (right) {
    return SDL_HITTEST_RESIZE_RIGHT;
  }

  if (IsPointInEditor(point.x, point.y)) {
    return SDL_HITTEST_NORMAL;
  }

  if (client_) {
    if (client_->IsPointInDragRegion(point.x, point.y)) {
      return SDL_HITTEST_DRAGGABLE;
    }

    if (!client_->HasDraggableRegions() && point.y >= 0 && point.y < 32) {
      return SDL_HITTEST_DRAGGABLE;
    }
  }

  return SDL_HITTEST_NORMAL;
}

SDL_HitTestResult SDLCALL SDL3Window::HitTestCallback(SDL_Window *window,
                                                     const SDL_Point *area,
                                                     void *data) {
  auto *self = static_cast<SDL3Window *>(data);
  if (!self || !area) {
    return SDL_HITTEST_NORMAL;
  }
  return self->HitTest(*area);
}

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

bool SDL3Window::IsHardwareAccelerated() const {
  return cross_platform_renderer_ && cross_platform_renderer_->IsInitialized();
}

void SDL3Window::SetClient(CefRefPtr<HyperionClient> client) {
  client_ = client;
}
