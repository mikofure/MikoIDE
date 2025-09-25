#include "mikoclient.hpp"
#include "../internal/simpleipc.hpp"
#include "client.hpp"
#include "offscreenrender.hpp"
#include "windowed.hpp"

// Windows API includes
#include <shellapi.h>
#include <windows.h>

bool HyperionClient::OnCursorChange(CefRefPtr<CefBrowser> browser,
                                    CefCursorHandle cursor,
                                    cef_cursor_type_t type,
                                    const CefCursorInfo &custom_cursor_info) {
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

    SDL_Cursor *sdl_cursor = SDL_CreateSystemCursor(cursor_id);
    if (sdl_cursor) {
      SDL_SetCursor(sdl_cursor);
    }
  }
  return true;
}

// HyperionClient implementation
HyperionClient::HyperionClient(SDL3Window *window)
    : window_(window), menu_overlay_active_(false),
      menu_overlay_browser_(nullptr) {
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
bool HyperionClient::OnQuery(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t query_id,
    const CefString &request, bool persistent,
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
        std::string type =
            request_str.substr(type_start, type_end - type_start);

        if (type == "open_menu_overlay") {
          Logger::LogMessage("DEBUG: Starting open_menu_overlay handler");
          Logger::LogMessage("DEBUG: Request string: " + request_str);

          try {
            Logger::LogMessage("DEBUG: Extracting JSON fields");
            // Extract section, x, y from JSON
            size_t section_pos = request_str.find("\"section\":");
            size_t x_pos = request_str.find("\"x\":");
            size_t y_pos = request_str.find("\"y\":");
            size_t width_pos = request_str.find("\"width\":");
            size_t height_pos = request_str.find("\"height\":");

            int width = 0;
            int height = 0;
            Logger::LogMessage("DEBUG: Found positions - section: " +
                               std::to_string(section_pos) +
                               ", x: " + std::to_string(x_pos) +
                               ", y: " + std::to_string(y_pos));

            if (width_pos != std::string::npos) {
              size_t w_start = request_str.find(":", width_pos) + 1;
              size_t w_end = request_str.find_first_of(",}", w_start);
              width = std::stoi(request_str.substr(w_start, w_end - w_start));
            }
            if (height_pos != std::string::npos) {
              size_t h_start = request_str.find(":", height_pos) + 1;
              size_t h_end = request_str.find_first_of(",}", h_start);
              height = std::stoi(request_str.substr(h_start, h_end - h_start));
            }
            if (section_pos != std::string::npos &&
                x_pos != std::string::npos && y_pos != std::string::npos) {
              Logger::LogMessage("DEBUG: Extracting section");
              // Extract section
              size_t section_start =
                  request_str.find("\"", section_pos + 10) + 1;
              size_t section_end = request_str.find("\"", section_start);
              std::string section = request_str.substr(
                  section_start, section_end - section_start);
              Logger::LogMessage("DEBUG: Extracted section: " + section);

              Logger::LogMessage("DEBUG: Extracting x coordinate");
              // Extract x coordinate (handle numeric values without quotes)
              size_t x_colon = request_str.find(":", x_pos);
              size_t x_start = x_colon + 1;
              // Skip any whitespace
              while (x_start < request_str.length() &&
                     (request_str[x_start] == ' ' ||
                      request_str[x_start] == '\t')) {
                x_start++;
              }
              size_t x_end = request_str.find(",", x_start);
              if (x_end == std::string::npos)
                x_end = request_str.find("}", x_start);
              std::string x_str = request_str.substr(x_start, x_end - x_start);
              // Trim whitespace
              x_str.erase(0, x_str.find_first_not_of(" \t"));
              x_str.erase(x_str.find_last_not_of(" \t") + 1);
              Logger::LogMessage("DEBUG: x_str before conversion: '" + x_str +
                                 "'");
              int x = static_cast<int>(std::stod(x_str));
              Logger::LogMessage("DEBUG: Extracted x: " + std::to_string(x));

              Logger::LogMessage("DEBUG: Extracting y coordinate");
              // Extract y coordinate (handle numeric values without quotes)
              size_t y_colon = request_str.find(":", y_pos);
              size_t y_start = y_colon + 1;
              // Skip any whitespace
              while (y_start < request_str.length() &&
                     (request_str[y_start] == ' ' ||
                      request_str[y_start] == '\t')) {
                y_start++;
              }
              size_t y_end = request_str.find("}", y_start);
              if (y_end == std::string::npos)
                y_end = request_str.find(",", y_start);
              std::string y_str = request_str.substr(y_start, y_end - y_start);
              // Trim whitespace
              y_str.erase(0, y_str.find_first_not_of(" \t"));
              y_str.erase(y_str.find_last_not_of(" \t") + 1);
              Logger::LogMessage("DEBUG: y_str before conversion: '" + y_str +
                                 "'");
              int y = static_cast<int>(std::stod(y_str));
              Logger::LogMessage("DEBUG: Extracted y: " + std::to_string(y));

              Logger::LogMessage(
                  "Processing menu overlay request for section: " + section +
                  " at (" + std::to_string(x) + ", " + std::to_string(y) + ")");

              Logger::LogMessage(
                  "DEBUG: About to call OpenMenuOverlay or CloseMenuOverlay");
              // Handle menu overlay opening
              if (section == "close") {
                Logger::LogMessage("DEBUG: Calling CloseMenuOverlay");
                CloseMenuOverlay();
              } else {
                Logger::LogMessage("DEBUG: Calling OpenMenuOverlay");
                OpenMenuOverlay(section, x, y, width, height);
              }
              Logger::LogMessage(
                  "DEBUG: Successfully completed menu overlay operation");
            } else {
              Logger::LogMessage("DEBUG: Missing required JSON fields");
            }

            callback->Success("success");
            return true;
          } catch (const std::exception &ex) {
            Logger::LogMessage("Exception in open_menu_overlay handler: " +
                               std::string(ex.what()));
            callback->Failure(500, "Internal error in menu overlay");
            return true;
          } catch (...) {
            Logger::LogMessage("Unknown exception in open_menu_overlay handler "
                               "(possibly 0xe06d7363)");
            callback->Failure(500, "Unknown error in menu overlay");
            return true;
          }
        } else if (type == "menu_item_click") {
          try {
            // Extract section and action from JSON
            size_t section_pos = request_str.find("\"section\":");
            size_t action_pos = request_str.find("\"action\":");

            if (section_pos != std::string::npos &&
                action_pos != std::string::npos) {
              // Extract section
              size_t section_start =
                  request_str.find("\"", section_pos + 10) + 1;
              size_t section_end = request_str.find("\"", section_start);
              std::string section = request_str.substr(
                  section_start, section_end - section_start);

              // Extract action
              size_t action_start = request_str.find("\"", action_pos + 9) + 1;
              size_t action_end = request_str.find("\"", action_start);
              std::string action =
                  request_str.substr(action_start, action_end - action_start);

              // Log menu item click for debugging
              Logger::LogMessage("Menu item clicked - Section: " + section +
                                 ", Action: " + action);

              // Close menu overlay after click
              CloseMenuOverlay();

              callback->Success("success");
              return true;
            } else {
              Logger::LogMessage("Invalid menu item click parameters");
              callback->Failure(400, "Invalid parameters");
              return true;
            }
          } catch (const std::exception &ex) {
            Logger::LogMessage("Exception in menu_item_click handler: " +
                               std::string(ex.what()));
            callback->Failure(500, "Internal error in menu item click");
            return true;
          } catch (...) {
            Logger::LogMessage("Unknown exception in menu_item_click handler "
                               "(possibly 0xe06d7363)");
            callback->Failure(500, "Unknown error in menu item click");
            return true;
          }
        } else if (type == "open_editor") {
          try {
            // Extract position and dimensions from JSON
            size_t x_pos = request_str.find("\"x\":");
            size_t y_pos = request_str.find("\"y\":");
            size_t width_pos = request_str.find("\"width\":");
            size_t height_pos = request_str.find("\"height\":");

            if (x_pos != std::string::npos && y_pos != std::string::npos &&
                width_pos != std::string::npos &&
                height_pos != std::string::npos) {

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
              int width = std::stoi(
                  request_str.substr(width_start, width_end - width_start));

              // Extract height
              size_t height_start = request_str.find(":", height_pos) + 1;
              size_t height_end = request_str.find_first_of(",}", height_start);
              int height = std::stoi(
                  request_str.substr(height_start, height_end - height_start));

              Logger::LogMessage(
                  "Opening editor at position: " + std::to_string(x) + "," +
                  std::to_string(y) + " with size: " + std::to_string(width) +
                  "x" + std::to_string(height));

              // Open editor using the same technique as menu overlay
              OpenEditor(x, y, width, height);

              callback->Success("success");
              return true;
            } else {
              Logger::LogMessage(
                  "DEBUG: Missing required JSON fields for editor");
              callback->Failure(400, "Missing required parameters");
              return true;
            }
          } catch (const std::exception &ex) {
            Logger::LogMessage("Exception in open_editor handler: " +
                               std::string(ex.what()));
            callback->Failure(500, "Internal error in editor");
            return true;
          } catch (...) {
            Logger::LogMessage("Unknown exception in open_editor handler "
                               "(possibly 0xe06d7363)");
            callback->Failure(500, "Unknown error in editor");
            return true;
          }
        } else if (type == "close_editor") {
          try {
            Logger::LogMessage("Closing editor");
            CloseEditor();
            callback->Success("success");
            return true;
          } catch (const std::exception &ex) {
            Logger::LogMessage("Exception in close_editor handler: " +
                               std::string(ex.what()));
            callback->Failure(500, "Internal error closing editor");
            return true;
          } catch (...) {
            Logger::LogMessage("Unknown exception in close_editor handler "
                               "(possibly 0xe06d7363)");
            callback->Failure(500, "Unknown error closing editor");
            return true;
          }
        } else if (type == "editor_position_update") {
          try {
            // Extract new position and dimensions from JSON
            size_t x_pos = request_str.find("\"x\":");
            size_t y_pos = request_str.find("\"y\":");
            size_t width_pos = request_str.find("\"width\":");
            size_t height_pos = request_str.find("\"height\":");

            if (x_pos != std::string::npos && y_pos != std::string::npos &&
                width_pos != std::string::npos &&
                height_pos != std::string::npos) {

              // Extract coordinates and dimensions
              size_t x_start = request_str.find(":", x_pos) + 1;
              size_t x_end = request_str.find_first_of(",}", x_start);
              int x = std::stoi(request_str.substr(x_start, x_end - x_start));

              size_t y_start = request_str.find(":", y_pos) + 1;
              size_t y_end = request_str.find_first_of(",}", y_start);
              int y = std::stoi(request_str.substr(y_start, y_end - y_start));

              size_t width_start = request_str.find(":", width_pos) + 1;
              size_t width_end = request_str.find_first_of(",}", width_start);
              int width = std::stoi(
                  request_str.substr(width_start, width_end - width_start));

              size_t height_start = request_str.find(":", height_pos) + 1;
              size_t height_end = request_str.find_first_of(",}", height_start);
              int height = std::stoi(
                  request_str.substr(height_start, height_end - height_start));

              Logger::LogMessage(
                  "Updating editor position to: " + std::to_string(x) + "," +
                  std::to_string(y) + " with size: " + std::to_string(width) +
                  "x" + std::to_string(height));

              // Update editor position using window method
              if (window_) {
                window_->SetEditorPosition(x, y, width, height);
              }

              callback->Success("success");
              return true;
            } else {
              Logger::LogMessage("DEBUG: Missing required JSON fields for "
                                 "editor position update");
              callback->Failure(400, "Missing required parameters");
              return true;
            }
          } catch (const std::exception &ex) {
            Logger::LogMessage("Exception in editor_position_update handler: " +
                               std::string(ex.what()));
            callback->Failure(500, "Internal error updating editor position");
            return true;
          } catch (...) {
            Logger::LogMessage("Unknown exception in editor_position_update "
                               "handler (possibly 0xe06d7363)");
            callback->Failure(500, "Unknown error updating editor position");
            return true;
          }
        } else if (type == "auto_height_result") {
          try {
            // Extract height from JSON
            size_t height_pos = request_str.find("\"height\":");

            if (height_pos != std::string::npos) {
              size_t height_start = request_str.find(":", height_pos) + 1;
              size_t height_end = request_str.find_first_of(",}", height_start);
              int height = std::stoi(
                  request_str.substr(height_start, height_end - height_start));

              Logger::LogMessage("Received auto height result: " +
                                 std::to_string(height));

              // Resize menu overlay to the calculated height
              if (window_ && menu_overlay_active_) {
                window_->ResizeMenuOverlay(height);
              }

              callback->Success("success");
              return true;
            } else {
              Logger::LogMessage(
                  "DEBUG: Missing height field in auto_height_result");
              callback->Failure(400, "Missing height parameter");
              return true;
            }
          } catch (const std::exception &ex) {
            Logger::LogMessage("Exception in auto_height_result handler: " +
                               std::string(ex.what()));
            callback->Failure(500, "Internal error processing auto height");
            return true;
          } catch (...) {
            Logger::LogMessage(
                "Unknown exception in auto_height_result handler");
            callback->Failure(500, "Unknown error processing auto height");
            return true;
          }
        }
      }
    } catch (const std::exception &) {
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
      std::string response =
          SimpleIPC::IPCHandler::GetInstance().HandleCall(method, message);
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
  } else if (request_str == "maximize_window") {
    if (window_) {
      window_->Maximize();
      callback->Success("");
      return true;
    }
  } else if (request_str == "restore_window") {
    if (window_) {
      window_->Restore();
      callback->Success("");
      return true;
    }
  } else if (request_str == "close_window") {
    if (window_) {
      window_->Close();
      callback->Success("");
      return true;
    }
  } else if (request_str == "toggle_borderless") {
    if (window_) {
      window_->SetBorderless(!window_->ShouldClose()); // Toggle borderless mode
      callback->Success("");
      return true;
    }
  }

  return false;
}

void HyperionClient::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                   const CefString &title) {
  CEF_REQUIRE_UI_THREAD();

  if (window_ && window_->GetSDLWindow()) {
    SDL_SetWindowTitle(window_->GetSDLWindow(), title.ToString().c_str());
  }
}

void HyperionClient::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefRefPtr<CefContextMenuParams> params,
                                         CefRefPtr<CefMenuModel> model) {
  CEF_REQUIRE_UI_THREAD();

  // Clear the context menu to disable right-click menu
  model->Clear();
}

bool HyperionClient::OnContextMenuCommand(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params, int command_id,
    EventFlags event_flags) {
  CEF_REQUIRE_UI_THREAD();
  return false;
}

bool HyperionClient::OnDragEnter(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefDragData> dragData,
                                 CefDragHandler::DragOperationsMask mask) {
  CEF_REQUIRE_UI_THREAD();
  return false;
}

void HyperionClient::OnDraggableRegionsChanged(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    const std::vector<CefDraggableRegion> &regions) {
  CEF_REQUIRE_UI_THREAD();

  // Store the draggable regions for window movement
  draggable_regions_ = regions;

  // Enhanced logging for debugging
  Logger::LogMessage("Draggable regions updated. Total regions: " +
                     std::to_string(regions.size()));

  for (size_t i = 0; i < regions.size(); ++i) {
    const auto &region = regions[i];
    std::string region_type = region.draggable ? "draggable" : "non-draggable";
    Logger::LogMessage("Region " + std::to_string(i) + ": " + region_type +
                       " at (" + std::to_string(region.bounds.x) + ", " +
                       std::to_string(region.bounds.y) + ") size " +
                       std::to_string(region.bounds.width) + "x" +
                       std::to_string(region.bounds.height));
  }
}

bool HyperionClient::IsPointInDragRegion(int x, int y) const {
  // First, check if point is in any non-draggable region
  // Non-draggable regions take priority over draggable ones
  for (const auto &region : draggable_regions_) {
    if (!region.draggable && x >= region.bounds.x &&
        x < region.bounds.x + region.bounds.width && y >= region.bounds.y &&
        y < region.bounds.y + region.bounds.height) {
      // Point is in a non-draggable region (app-region: no-drag)
      return false;
    }
  }

  // Then check if point is in any draggable region
  for (const auto &region : draggable_regions_) {
    if (region.draggable && x >= region.bounds.x &&
        x < region.bounds.x + region.bounds.width && y >= region.bounds.y &&
        y < region.bounds.y + region.bounds.height) {
      // Point is in a draggable region (app-region: drag)
      return true;
    }
  }

  // Point is not in any defined region, default to non-draggable
  return false;
}

bool HyperionClient::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    bool user_gesture, bool is_redirect) {
  CEF_REQUIRE_UI_THREAD();

  message_router_->OnBeforeBrowse(browser, frame);
  return false;
}

bool HyperionClient::OnOpenURLFromTab(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    const CefString &target_url,
    CefRequestHandler::WindowOpenDisposition target_disposition,
    bool user_gesture) {
  CEF_REQUIRE_UI_THREAD();

  // Handle external URL opening
  if (target_disposition == 3 || // WOD_NEW_FOREGROUND_TAB
      target_disposition == 4 || // WOD_NEW_BACKGROUND_TAB
      target_disposition == 5 || // WOD_NEW_POPUP
      target_disposition == 6) { // WOD_NEW_WINDOW

    // Open in system default browser
    ShellExecuteA(nullptr, "open", target_url.ToString().c_str(), nullptr,
                  nullptr, SW_SHOWNORMAL);
    return true;
  }

  return false;
}

bool HyperionClient::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefProcessId source_process, CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  return message_router_->OnProcessMessageReceived(browser, frame,
                                                   source_process, message);
}

bool HyperionClient::OnBeforePopup(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int popup_id,
    const CefString &target_url, const CefString &target_frame_name,
    CefLifeSpanHandler::WindowOpenDisposition target_disposition,
    bool user_gesture, const CefPopupFeatures &popupFeatures,
    CefWindowInfo &windowInfo, CefRefPtr<CefClient> &client,
    CefBrowserSettings &settings, CefRefPtr<CefDictionaryValue> &extra_info,
    bool *no_javascript_access) {
  CEF_REQUIRE_UI_THREAD();

  // Block popups and open in system browser instead
  ShellExecuteA(nullptr, "open", target_url.ToString().c_str(), nullptr,
                nullptr, SW_SHOWNORMAL);
  return true;
}

void HyperionClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  browser_list_.push_back(browser);

  if (browser_list_.size() == 1) {
    // First CEF browser created
    Logger::LogMessage("First CEF browser created");
  } else if (browser_list_.size() == 2) {
    // Second browser is the editor browser (created by OpenEditor)
    if (window_) {
      window_->SetEditorBrowser(browser);
      Logger::LogMessage(
          "Editor browser (2nd browser) created and stored for OSR rendering");
    }
  } else {
    // Third+ browser - check if this is a menu overlay browser and apply
    // transparency
    std::string url = browser->GetMainFrame()->GetURL().ToString();
    Logger::LogMessage("OnAfterCreated: Checking URL for browser #" +
                       std::to_string(browser_list_.size()) + ": " + url);
    if (url.find("miko://menuoverlay/") == 0) {
      // Get the native window handle for the overlay
      HWND overlay_hwnd = browser->GetHost()->GetWindowHandle();
      if (overlay_hwnd) {
        // Apply full transparency settings to the overlay window
        // Use RGB(0,0,0) as transparent color key and set alpha to 0 for full
        // transparency
        SetLayeredWindowAttributes(overlay_hwnd, RGB(0, 0, 0), 0,
                                   LWA_COLORKEY | LWA_ALPHA);
        Logger::LogMessage(
            "Applied full transparency to menu overlay window (alpha=0)");

        // Ensure the window stays on top
        SetWindowPos(overlay_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        Logger::LogMessage("Set menu overlay window to stay on top");
      } else {
        Logger::LogMessage(
            "Failed to get overlay window handle for transparency");
      }
    }
  }
}

bool HyperionClient::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  return false;
}

void HyperionClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
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

void HyperionClient::OnLoadError(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                                 const CefString &errorText,
                                 const CefString &failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files
  if (errorCode == ERR_ABORTED)
    return;

  // Display a load error message using a data: URI
  std::ostringstream loadError;
  loadError << "<html><body bgcolor=\"white\">"
               "<h2>Failed to load URL "
            << std::string(failedUrl) << " with error "
            << std::string(errorText) << " (" << errorCode
            << ").</h2></body></html>";

  frame->LoadURL(GetDataURI(loadError.str(), "text/html"));
}

void HyperionClient::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 TransitionType transition_type) {
  CEF_REQUIRE_UI_THREAD();

  if (frame->IsMain()) {
    // Main frame started loading
    Logger::LogMessage("Main frame started loading");
  }
}

void HyperionClient::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame, int httpStatusCode) {
  CEF_REQUIRE_UI_THREAD();

  if (frame->IsMain() && menu_overlay_active_ && menu_overlay_browser_ &&
      browser->IsSame(menu_overlay_browser_)) {
    // Menu overlay finished loading - get content height for auto-sizing
    Logger::LogMessage(
        "Menu overlay loaded, getting content height for auto-sizing");

    // Execute JavaScript to get the document height
    CefRefPtr<CefV8Context> context = frame->GetV8Context();
    if (context) {
      std::string js_code =
          "var height = Math.max("
          "  document.body.scrollHeight,"
          "  document.body.offsetHeight,"
          "  document.documentElement.clientHeight,"
          "  document.documentElement.scrollHeight,"
          "  document.documentElement.offsetHeight"
          ");"
          "window.cefQuery({"
          "  request: JSON.stringify({"
          "    type: 'auto_height_result',"
          "    height: height"
          "  }),"
          "  onSuccess: function(response) {},"
          "  onFailure: function(error_code, error_message) {}"
          "});";

      frame->ExecuteJavaScript(js_code, "", 0);
    }
  }
}

void HyperionClient::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread
    CefPostTask(TID_UI, new CloseBrowserTask(this, force_close));
    return;
  }

  DoCloseAllBrowsers(force_close);
}

void HyperionClient::DoCloseAllBrowsers(bool force_close) {
  CEF_REQUIRE_UI_THREAD();

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it) {
    (*it)->GetHost()->CloseBrowser(force_close);
  }
}

CefRefPtr<CefBrowser> HyperionClient::GetFirstBrowser() {
  CEF_REQUIRE_UI_THREAD();

  if (!browser_list_.empty())
    return browser_list_.front();
  return nullptr;
}

bool HyperionClient::HasBrowsers() {
  CEF_REQUIRE_UI_THREAD();
  return !browser_list_.empty();
}

bool HyperionClient::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                                   const CefKeyEvent &event,
                                   CefEventHandle os_event,
                                   bool *is_keyboard_shortcut) {
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
    if ((event.modifiers & EVENTFLAG_CONTROL_DOWN &&
         event.windows_key_code == 'R') ||
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

bool HyperionClient::OnBeforeDownload(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item,
    const CefString &suggested_name,
    CefRefPtr<CefBeforeDownloadCallback> callback) {
  CEF_REQUIRE_UI_THREAD();

  // Set the download path
  std::string download_path = GetDownloadPath(suggested_name.ToString());
  callback->Continue(download_path, false);

  return true;
}

void HyperionClient::OnDownloadUpdated(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item,
    CefRefPtr<CefDownloadItemCallback> callback) {
  CEF_REQUIRE_UI_THREAD();

  if (download_item->IsComplete()) {
    Logger::LogMessage("Download completed: " +
                       download_item->GetFullPath().ToString());
  }
}

void HyperionClient::SpawnNewWindow() {
  // This would create a new window instance
  // Implementation depends on application architecture
}

// OSR specific methods
void HyperionClient::SendMouseClickEvent(int x, int y,
                                         CefBrowserHost::MouseButtonType button,
                                         bool mouse_up, int click_count) {
  auto browser = GetFirstBrowser();
  if (browser && window_) {
    CefMouseEvent mouse_event;
    // Apply DPI scaling to mouse coordinates for CEF
    float dpi_scale = window_->GetDPIScale();
    mouse_event.x = static_cast<int>(x / dpi_scale);
    mouse_event.y = static_cast<int>(y / dpi_scale);
    mouse_event.modifiers = 0;
    browser->GetHost()->SendMouseClickEvent(mouse_event, button, mouse_up,
                                            click_count);
  }
}

void HyperionClient::SendMouseMoveEvent(int x, int y, bool mouse_leave) {
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

void HyperionClient::SendMouseWheelEvent(int x, int y, int delta_x,
                                         int delta_y) {
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
// Menu overlay management methods
void HyperionClient::OpenEditor(int x, int y, int width, int height) {
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

void HyperionClient::CloseEditor() {
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
  } catch (const std::exception &ex) {
    Logger::LogMessage("Exception in CloseEditor: " + std::string(ex.what()));
  } catch (...) {
    Logger::LogMessage("Unknown exception in CloseEditor");
  }
}

void HyperionClient::OpenMenuOverlay(const std::string &section, int x, int y,
                                     int width, int height) {
  CEF_REQUIRE_UI_THREAD();

  try {
    // Check if menu overlay is already active
    if (menu_overlay_active_) {
      Logger::LogMessage("Menu overlay already active, ignoring request");
      return;
    }

    Logger::LogMessage("OpenMenuOverlay called - section: " + section +
                       ", x: " + std::to_string(x) + ", y: " +
                       std::to_string(y) + ", width: " + std::to_string(width) +
                       ", height: " + std::to_string(height));

    if (!browser_list_.empty()) {
      CefRefPtr<CefBrowser> browser = browser_list_.front();
      if (browser && browser->GetHost()) {
        Logger::LogMessage("Browser available, creating overlay window");

        // Get screen dimensions for positioning calculations
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);

        // Get current cursor position for smart positioning
        POINT cursor_pos;
        GetCursorPos(&cursor_pos);

        // Use provided width if given, otherwise use default width
        // Hardcode heights based on actual measurements for each section
        int overlayWidth =
            (width > 0) ? width : 300; // Default width if not specified
        int overlayHeight = 0;         // Default to auto height

        // Hardcode heights for each menu section based on measurements
        if (section == "File") {
          overlayHeight = 346;
        } else if (section == "Edit") {
          overlayHeight = 610;
        } else if (section == "View") {
          overlayHeight = 274;
        } else if (section == "Navigate") {
          overlayHeight = 250;
        } else if (section == "Selection") {
          overlayHeight = 226;
        } else if (section == "Tools") {
          overlayHeight = 298;
        } else if (section == "Window") {
          overlayHeight = 226;
        } else if (section == "Help") {
          overlayHeight = 130;
        } else {
          overlayHeight = 300; // Default height for unknown sections
        }

        Logger::LogMessage("Using hardcoded height " +
                           std::to_string(overlayHeight) +
                           " for section: " + section);

        // Fixed positioning - always use x coordinate or cursor, y is always 32
        int overlayX, overlayY;
        if (x != 0) {
          // Use provided x coordinate
          overlayX = x;
          Logger::LogMessage("Using provided x coordinate: " +
                             std::to_string(x));
        } else {
          // Use cursor x position if no specific x coordinate provided
          overlayX = cursor_pos.x + 10; // Offset slightly from cursor
          Logger::LogMessage("Using cursor x position: " +
                             std::to_string(cursor_pos.x));
        }

        // Y position is always fixed to 32 (below title bar)
        overlayY = 32;
        Logger::LogMessage("Using fixed y position: 32");

        // Ensure overlay fits on screen horizontally
        if (overlayX + overlayWidth > screenWidth) {
          overlayX = std::max(0, screenWidth - overlayWidth);
        }
        if (overlayX < 0) {
          overlayX = 0; // Align to left edge
        }

        // Create a new browser window for the menu overlay
        CefWindowInfo window_info;
        window_info.SetAsPopup(window_->GetHWND(), "MenuOverlay");

        // Set window style for transparency and layered window
        window_info.ex_style = WS_EX_LAYERED | WS_EX_TOPMOST |
                               WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT;
        window_info.style = WS_POPUP | WS_VISIBLE;

        // Position the overlay window with calculated bounds
        window_info.bounds.x = overlayX;
        window_info.bounds.y = overlayY;
        window_info.bounds.width = overlayWidth;
        window_info.bounds.height = overlayHeight;

        Logger::LogMessage(
            "Overlay window bounds - x: " + std::to_string(overlayX) + ", y: " +
            std::to_string(overlayY) + ", w: " + std::to_string(overlayWidth) +
            ", h: " + std::to_string(overlayHeight));

        CefBrowserSettings browser_settings;
        browser_settings.javascript_close_windows = STATE_ENABLED;
        // Set transparent background using BGRA format (Alpha = 0 for
        // transparency)
        browser_settings.background_color =
            CefColorSetARGB(0, 0, 0, 0); // Fully transparent BGRA

        // Create Steam-like overlay routing URL
        std::string overlay_url = BuildOverlayURL(section, overlayX, overlayY,
                                                  overlayWidth, overlayHeight);
        Logger::LogMessage("Overlay URL created, length: " +
                           std::to_string(overlay_url.length()));
        Logger::LogMessage("URL preview (first 200 chars): " +
                           overlay_url.substr(0, 200));

        bool browser_created = CefBrowserHost::CreateBrowser(
            window_info, this, overlay_url, browser_settings, nullptr, nullptr);
        Logger::LogMessage("CreateBrowser result: " +
                           std::string(browser_created ? "true" : "false"));

        // Mark overlay as active if browser creation was successful
        if (browser_created) {
          menu_overlay_active_ = true;

          // Update SDL3Window overlay position tracking
          if (window_) {
            window_->SetMenuOverlayPosition(overlayX, overlayY);
            window_->SetMenuOverlayVisible(true);
            window_->SetCurrentMenuSection(section);
          }

          Logger::LogMessage(
              "Menu overlay marked as active and position updated");
        }

        Logger::LogMessage(
            "Menu overlay opened for section: " + section + " at positioned (" +
            std::to_string(overlayX) + ", " + std::to_string(overlayY) +
            ") with size (" + std::to_string(overlayWidth) + "x" +
            std::to_string(overlayHeight) + ") and URL: " + overlay_url);
      } else {
        Logger::LogMessage(
            "Failed to open menu overlay: browser or host is null");
      }
    } else {
      Logger::LogMessage("Failed to open menu overlay: no browsers available");
    }
  } catch (const std::exception &ex) {
    Logger::LogMessage("Exception in OpenMenuOverlay: " +
                       std::string(ex.what()));
  } catch (...) {
    Logger::LogMessage(
        "Unknown exception in OpenMenuOverlay (possibly 0xe06d7363)");
  }
}

void HyperionClient::CloseMenuOverlay() {
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

    // Reset SDL3Window overlay position tracking
    if (window_) {
      window_->SetMenuOverlayVisible(false);
      window_->SetMenuOverlayPosition(0, 0);
      window_->SetCurrentMenuSection("");
    }

  } catch (const std::exception &ex) {
    Logger::LogMessage("Exception in CloseMenuOverlay: " +
                       std::string(ex.what()));
    // Reset tracking variables even on exception
    menu_overlay_active_ = false;
    menu_overlay_browser_ = nullptr;

    // Reset SDL3Window overlay position tracking even on exception
    if (window_) {
      window_->SetMenuOverlayVisible(false);
      window_->SetMenuOverlayPosition(0, 0);
      window_->SetCurrentMenuSection("");
    }
  } catch (...) {
    Logger::LogMessage(
        "Unknown exception in CloseMenuOverlay (possibly 0xe06d7363)");
    // Reset tracking variables even on exception
    menu_overlay_active_ = false;
    menu_overlay_browser_ = nullptr;

    // Reset SDL3Window overlay position tracking even on exception
    if (window_) {
      window_->SetMenuOverlayVisible(false);
      window_->SetMenuOverlayPosition(0, 0);
      window_->SetCurrentMenuSection("");
    }
  }
}

std::string HyperionClient::GetMenuOverlayHTML(const std::string &section) {
  Logger::LogMessage("GetMenuOverlayHTML called for section: " + section);

  // Use embedded HTML resource from resourceutil
  std::vector<uint8_t> html_data =
      ResourceUtil::LoadBinaryResource(ResourceUtil::IDR_HTML_MENUOVERLAY);
  Logger::LogMessage("LoadBinaryResource returned " +
                     std::to_string(html_data.size()) + " bytes");

  if (!html_data.empty()) {
    std::string html_content(html_data.begin(), html_data.end());
    Logger::LogMessage("HTML content converted to string, length: " +
                       std::to_string(html_content.length()));
    Logger::LogMessage("HTML preview (first 100 chars): " +
                       html_content.substr(0, 100));
    return html_content;
  }

  // Fallback if resource is not available
  Logger::LogMessage("WARNING: Using fallback HTML - resource not available");
  return "<html><body>Menu overlay not available</body></html>";
}

std::string HyperionClient::BuildOverlayURL(const std::string &section, int x,
                                            int y, int width, int height) {
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

  Logger::LogMessage(
      "Building miko:// URL with parameters - pid: " + std::to_string(pid) +
      ", browser_id: " + std::to_string(browser_id));

  // Use miko:// protocol to serve HTML directly from binary resources with
  // parameters
  std::string overlay_url =
      "miko://menuoverlay/index.html"
      "?createflags=" +
      std::to_string(create_flags) + "&pid=" + std::to_string(pid) +
      "&browser=" + std::to_string(browser_id) +
      "&screenavailwidth=" + std::to_string(screen_width) +
      "&screenavailheight=" + std::to_string(screen_height) +
      "&section=" + section + "&x=" + std::to_string(x) +
      "&y=" + std::to_string(y) + "&width=" + std::to_string(width) +
      "&height=" + std::to_string(height);

  Logger::LogMessage("Miko URL created: " + overlay_url);

  return overlay_url;
}

CefRefPtr<CefDisplayHandler> HyperionClient::GetDisplayHandler() {
  return this;
}

CefRefPtr<CefLifeSpanHandler> HyperionClient::GetLifeSpanHandler() {
  return this;
}

CefRefPtr<CefLoadHandler> HyperionClient::GetLoadHandler() { return this; }

CefRefPtr<CefContextMenuHandler> HyperionClient::GetContextMenuHandler() {
  return this;
}

CefRefPtr<CefDragHandler> HyperionClient::GetDragHandler() { return this; }

CefRefPtr<CefRequestHandler> HyperionClient::GetRequestHandler() {
  return this;
}

CefRefPtr<CefKeyboardHandler> HyperionClient::GetKeyboardHandler() {
  return this;
}

CefRefPtr<CefDownloadHandler> HyperionClient::GetDownloadHandler() {
  return this;
}

CefRefPtr<CefRenderHandler> HyperionClient::GetRenderHandler() {
  return render_handler_;
}

CefRefPtr<CefResourceHandler>
HyperionClient::GetResourceHandler(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefRequest> request) {
  CEF_REQUIRE_IO_THREAD();

  return resource_provider_->Create(browser, frame, "miko", request);
}
