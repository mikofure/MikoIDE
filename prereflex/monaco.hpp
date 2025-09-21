#pragma once

#include "include/cef_browser.h"
#include "include/cef_client.h"
#include <string>

// Forward declarations
class SDL3Window;

// Monaco Editor Manager class for handling Monaco editor functionality
class MonacoEditorManager {
public:
  explicit MonacoEditorManager(SDL3Window* window);
  ~MonacoEditorManager() = default;

  // Editor management
  void OpenEditor(int x, int y, int width, int height);
  void CloseEditor();
  
  // Editor state
  bool IsEditorEnabled() const;
  void SetEditorBrowser(CefRefPtr<CefBrowser> browser);
  
  // Editor positioning and sizing
  void SetEditorPosition(int x, int y, int width, int height);
  void UpdateEditorTexture(const void* buffer, int width, int height);
  
  // Editor browser management
  CefRefPtr<CefBrowser> GetEditorBrowser() const;

private:
  SDL3Window* window_;
  CefRefPtr<CefBrowser> editor_browser_;
  bool editor_enabled_;
  
  struct EditorRect {
    int x, y, width, height;
  } editor_rect_;
};