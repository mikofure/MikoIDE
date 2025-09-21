#include "monaco.hpp"
#include "windowed.hpp"
#include "offscreen.hpp"
#include "../utils/logger.hpp"
#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"
#include <SDL3/SDL.h>

MonacoEditorManager::MonacoEditorManager(SDL3Window* window)
    : window_(window), editor_browser_(nullptr), editor_enabled_(false) {
  editor_rect_ = {0, 0, 0, 0};
}

void MonacoEditorManager::OpenEditor(int x, int y, int width, int height) {
  CEF_REQUIRE_UI_THREAD();

  try {
    Logger::LogMessage("OpenEditor called - x: " + std::to_string(x) + ", y: " +
                       std::to_string(y) + ", width: " + std::to_string(width) +
                       ", height: " + std::to_string(height));

    if (window_) {
      Logger::LogMessage("Creating OSR editor browser");

      // Enable editor in the window
      window_->EnableEditor(true);
      window_->SetEditorPosition(x, y, width, height);

      // Create OSR browser for the editor
      CefWindowInfo window_info;
      window_info.SetAsWindowless(nullptr); // Use OSR mode

      Logger::LogMessage("Editor OSR bounds - x: " + std::to_string(x) +
                         ", y: " + std::to_string(y) +
                         ", w: " + std::to_string(width) +
                         ", h: " + std::to_string(height));

      CefBrowserSettings browser_settings;
      browser_settings.javascript_close_windows = STATE_ENABLED;
      browser_settings.background_color = CefColorSetARGB(
          0, 0, 0, 0); // Fully transparent background for consistency

      // Create editor URL using monaco resource
      std::string editor_url = "miko://monaco/index.html";
      Logger::LogMessage("Editor URL: " + editor_url);

      // Create OSR render handler for the editor
      CefRefPtr<OSRRenderHandler> render_handler =
          new OSRRenderHandler(window_);

      bool browser_created = CefBrowserHost::CreateBrowser(
          window_info, this, editor_url, browser_settings, nullptr, nullptr);
      Logger::LogMessage("CreateBrowser result: " +
                         std::string(browser_created ? "true" : "false"));

      Logger::LogMessage("Monaco editor opened with OSR at position (" +
                         std::to_string(x) + ", " + std::to_string(y) + ") " +
                         "with size (" + std::to_string(width) + "x" +
                         std::to_string(height) + ") and URL: " + editor_url);
    } else {
      Logger::LogMessage("Failed to open editor: window is null");
    }
  } catch (const std::exception &ex) {
    Logger::LogMessage("Exception in OpenEditor: " + std::string(ex.what()));
  } catch (...) {
    Logger::LogMessage("Unknown exception in OpenEditor");
  }
}

void MonacoEditorManager::CloseEditor() {
  CEF_REQUIRE_UI_THREAD();

  try {
    if (editor_browser_ && editor_browser_->GetHost()) {
      editor_browser_->GetHost()->CloseBrowser(true);
      if (window_) {
        window_->EnableEditor(false);
      }
      editor_browser_ = nullptr;
      Logger::LogMessage("Monaco editor closed");
    }
  } catch (const std::exception &ex) {
    Logger::LogMessage("Exception in CloseEditor: " + std::string(ex.what()));
  } catch (...) {
    Logger::LogMessage("Unknown exception in CloseEditor");
  }
}

bool MonacoEditorManager::IsEditorEnabled() const {
  return editor_enabled_;
}

void MonacoEditorManager::SetEditorBrowser(CefRefPtr<CefBrowser> browser) {
  editor_browser_ = browser;
  Logger::LogMessage("Editor browser reference set");
}

void MonacoEditorManager::SetEditorPosition(int x, int y, int width, int height) {
  editor_rect_.x = x;
  editor_rect_.y = y;
  editor_rect_.width = width;
  editor_rect_.height = height;
  editor_enabled_ = true;
  Logger::LogMessage("Editor position set to (" + std::to_string(x) + ", " +
                     std::to_string(y) + ") with size " +
                     std::to_string(width) + "x" + std::to_string(height));
}

void MonacoEditorManager::UpdateEditorTexture(const void *buffer, int width, int height) {
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

  if (!window_) {
    Logger::LogMessage("UpdateEditorTexture: Window is null!");
    return;
  }

  // Delegate to the window's texture update method
  window_->UpdateEditorTexture(buffer, width, height);
}

CefRefPtr<CefBrowser> MonacoEditorManager::GetEditorBrowser() const {
  return editor_browser_;
}