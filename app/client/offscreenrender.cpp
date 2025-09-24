// OSRRenderHandler implementation
OSRRenderHandler::OSRRenderHandler(SDL3Window *window) : window_(window) {}

void OSRRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser,
                                   CefRect &rect) {
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
                           std::to_string(rect.width) + "x" +
                           std::to_string(rect.height));
        return;
      } else {
        Logger::LogMessage(
            "GetViewRect: Editor rect has invalid dimensions, using default");
      }
    }

    // For main browser or if editor rect is invalid, use full window size
    rect.x = 0;
    rect.y = 0;
    rect.width = DEFAULT_WINDOW_WIDTH;   // Default width
    rect.height = DEFAULT_WINDOW_HEIGHT; // Default height

    // Get actual window size and apply DPI scaling
    if (window_->GetSDLWindow()) {
      int w, h;
      SDL_GetWindowSize(window_->GetSDLWindow(), &w, &h);

      // Apply DPI scaling to get logical size for CEF
      float dpi_scale = window_->GetDPIScale();
      rect.width = static_cast<int>(w / dpi_scale);
      rect.height = static_cast<int>(h / dpi_scale);

      Logger::LogMessage("GetViewRect: Main browser - Physical size " +
                         std::to_string(w) + "x" + std::to_string(h) +
                         ", DPI scale " + std::to_string(dpi_scale) +
                         ", Logical size " + std::to_string(rect.width) + "x" +
                         std::to_string(rect.height));
    } else {
      Logger::LogMessage("GetViewRect: Using default size " +
                         std::to_string(rect.width) + "x" +
                         std::to_string(rect.height));
    }
  } else {
    Logger::LogMessage("GetViewRect: Window is null, using default " +
                       std::to_string(DEFAULT_WINDOW_WIDTH) + "x" +
                       std::to_string(DEFAULT_WINDOW_HEIGHT));
    rect.x = 0;
    rect.y = 0;
    rect.width = DEFAULT_WINDOW_WIDTH;
    rect.height = DEFAULT_WINDOW_HEIGHT;
  }
}

void OSRRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                               PaintElementType type,
                               const RectList &dirtyRects, const void *buffer,
                               int width, int height) {
  if (type != PET_VIEW || !window_ || !buffer) {
    Logger::LogMessage(
        "OnPaint: Invalid parameters - type=" + std::to_string(type) +
        ", window=" + (window_ ? "valid" : "null") +
        ", buffer=" + (buffer ? "valid" : "null"));
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

bool OSRRenderHandler::StartDragging(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data,
    CefRenderHandler::DragOperationsMask allowed_ops, int x, int y) {
  // Handle drag and drop operations
  return false; // Let CEF handle it
}

void OSRRenderHandler::UpdateDragCursor(
    CefRefPtr<CefBrowser> browser, CefRenderHandler::DragOperation operation) {
  // Update cursor during drag operations
}
