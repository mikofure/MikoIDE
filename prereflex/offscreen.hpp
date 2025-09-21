#pragma once

#include "include/cef_render_handler.h"

// Forward declaration
class SDL3Window;

// Off-screen render handler for CEF browser
class OSRRenderHandler : public CefRenderHandler {
public:
    OSRRenderHandler(SDL3Window* window);
    
    // CefRenderHandler methods
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    void OnPaint(CefRefPtr<CefBrowser> browser,
                 PaintElementType type,
                 const RectList& dirtyRects,
                 const void* buffer,
                 int width,
                 int height) override;

    bool StartDragging(CefRefPtr<CefBrowser> browser,
                      CefRefPtr<CefDragData> drag_data,
                      CefRenderHandler::DragOperationsMask allowed_ops,
                      int x, int y) override;
    void UpdateDragCursor(CefRefPtr<CefBrowser> browser,
                         CefRenderHandler::DragOperation operation) override;

private:
    SDL3Window* window_;
    IMPLEMENT_REFCOUNTING(OSRRenderHandler);
};