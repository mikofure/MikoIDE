// CEF OSR Render Handler
#pragma once

#include "include/cef_render_handler.h"
#include <SDL3/SDL.h>
#include <mutex>
#include <vector>

// Forward declaration
class SDL3Window;

#include "../utils/logger.hpp"
#include "../utils/config.hpp"
#include "../renderer/dx11_renderer.hpp"

class OSRRenderHandler : public CefRenderHandler {
public:
  OSRRenderHandler(SDL3Window *window);

  // CefRenderHandler methods
  void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;
  void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
               const RectList &dirtyRects, const void *buffer, int width,
               int height) override;

  bool StartDragging(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefDragData> drag_data,
                     CefRenderHandler::DragOperationsMask allowed_ops, int x,
                     int y) override;
  void UpdateDragCursor(CefRefPtr<CefBrowser> browser,
                        CefRenderHandler::DragOperation operation) override;

private:
  SDL3Window *window_;
  IMPLEMENT_REFCOUNTING(OSRRenderHandler);
};
