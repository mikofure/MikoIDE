#include "chromiumclient.hpp"
#include "windowed.hpp"
#include "../internal/simpleipc.hpp"
#include "../resources/resourceutil.hpp"
#include "../utils/config.hpp"
#include "../utils/logger.hpp"
#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"
#include <SDL3/SDL.h>
#include <shellapi.h>

// CloseBrowserTask implementation
CloseBrowserTask::CloseBrowserTask(CefRefPtr<SimpleClient> client,
                                   bool force_close)
    : client_(client), force_close_(force_close) {}

void CloseBrowserTask::Execute() {
  client_->DoCloseAllBrowsers(force_close_);
}

// SimpleClient implementation
SimpleClient::SimpleClient(SDL3Window *window)
    : window_(window), menu_overlay_active_(false),
      menu_overlay_browser_(nullptr) {
  // Create the browser-side router for query handling
  CefMessageRouterConfig config;
  message_router_ = CefMessageRouterBrowserSide::Create(config);

  // Create the resource provider for handling custom schemes
  resource_provider_ = CefResourceManager::Create();

  // Add provider for miko:// scheme
  resource_provider_->AddDirectoryProvider("miko", GetResourceDir(),
                                           100, std::string());
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
  return nullptr; // Not used for windowed rendering
}

bool SimpleClient::OnQuery(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t query_id,
    const CefString &request, bool persistent,
    CefRefPtr<CefMessageRouterBrowserSide::Callback> callback) {

  const std::string &request_str = request;
  Logger::LogMessage("Received query: " + request_str);

  // Parse JSON request
  rapidjson::Document doc;
  doc.Parse(request_str.c_str());

  if (doc.HasParseError()) {
    Logger::LogMessage("Failed to parse JSON request");
    callback->Failure(-1, "Invalid JSON");
    return true;
  }

  if (!doc.HasMember("type") || !doc["type"].IsString()) {
    Logger::LogMessage("Request missing 'type' field");
    callback->Failure(-1, "Missing type field");
    return true;
  }

  std::string type = doc["type"].GetString();
  Logger::LogMessage("Request type: " + type);

  if (type == "open_menu_overlay") {
    if (doc.HasMember("section") && doc["section"].IsString()) {
      std::string section = doc["section"].GetString();
      Logger::LogMessage("Opening menu overlay for section: " + section);
      
      // Get position from document if available
      int x = 0, y = 32, width = 300, height = 300;
      if (doc.HasMember("x") && doc["x"].IsInt()) x = doc["x"].GetInt();
      if (doc.HasMember("y") && doc["y"].IsInt()) y = doc["y"].GetInt();
      if (doc.HasMember("width") && doc["width"].IsInt()) width = doc["width"].GetInt();
      if (doc.HasMember("height") && doc["height"].IsInt()) height = doc["height"].GetInt();
      
      OpenMenuOverlay(section, x, y, width, height);
      callback->Success("Menu overlay opened");
    } else {
      callback->Failure(-1, "Missing section parameter");
    }
    return true;
  }

  if (type == "menu_item_click") {
    if (doc.HasMember("item") && doc["item"].IsString()) {
      std::string item = doc["item"].GetString();
      Logger::LogMessage("Menu item clicked: " + item);
      
      // Handle specific menu items
      if (item == "New File") {
        // Create new file
        Logger::LogMessage("Creating new file");
      } else if (item == "Open File") {
        // Open file dialog
        Logger::LogMessage("Opening file dialog");
      } else if (item == "Save") {
        // Save current file
        Logger::LogMessage("Saving current file");
      } else if (item == "Exit") {
        // Exit application
        Logger::LogMessage("Exiting application");
        CloseAllBrowsers(false);
      }
      
      // Close menu overlay after item click
      CloseMenuOverlay();
      callback->Success("Menu item handled");
    } else {
      callback->Failure(-1, "Missing item parameter");
    }
    return true;
  }

  if (type == "open_editor") {
    Logger::LogMessage("Opening editor");
    
    // Get position and size from request
    int x = 0, y = 0, width = 800, height = 600;
    if (doc.HasMember("x") && doc["x"].IsInt()) x = doc["x"].GetInt();
    if (doc.HasMember("y") && doc["y"].IsInt()) y = doc["y"].GetInt();
    if (doc.HasMember("width") && doc["width"].IsInt()) width = doc["width"].GetInt();
    if (doc.HasMember("height") && doc["height"].IsInt()) height = doc["height"].GetInt();
    
    OpenEditor(x, y, width, height);
    callback->Success("Editor opened");
    return true;
  }

  if (type == "close_editor") {
    Logger::LogMessage("Closing editor");
    CloseEditor();
    callback->Success("Editor closed");
    return true;
  }

  if (type == "close_menu_overlay") {
    Logger::LogMessage("Closing menu overlay");
    CloseMenuOverlay();
    callback->Success("Menu overlay closed");
    return true;
  }

  if (type == "auto_height_result") {
    if (doc.HasMember("height") && doc["height"].IsInt()) {
      int height = doc["height"].GetInt();
      Logger::LogMessage("Auto-height result received: " + std::to_string(height));
      
      // Resize the menu overlay to the content height
      if (window_) {
        window_->ResizeMenuOverlay(height);
      }
      
      callback->Success("Height updated");
    } else {
      callback->Failure(-1, "Missing height parameter");
    }
    return true;
  }

  // Handle other request types...
  Logger::LogMessage("Unhandled request type: " + type);
  callback->Failure(-1, "Unknown request type");
  return true;
}

void SimpleClient::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                 const CefString &title) {
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

bool SimpleClient::IsPointInDragRegion(int x, int y) const {
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

bool SimpleClient::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefRequest> request,
                                  bool user_gesture, bool is_redirect) {
  CEF_REQUIRE_UI_THREAD();

  message_router_->OnBeforeBrowse(browser, frame);
  return false;
}

bool SimpleClient::OnOpenURLFromTab(
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

bool SimpleClient::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefProcessId source_process, CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  return message_router_->OnProcessMessageReceived(browser, frame,
                                                   source_process, message);
}

CefRefPtr<CefResourceHandler>
SimpleClient::GetResourceHandler(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefRequest> request) {
  CEF_REQUIRE_IO_THREAD();

  return resource_provider_->Create(browser, frame, "miko", request);
}

bool SimpleClient::OnBeforePopup(
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

void SimpleClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
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

        // Position the overlay window and ensure it stays on top
        int overlay_x = 0, overlay_y = 32;
        if (window_) {
          overlay_x = window_->GetMenuOverlayX();
          overlay_y = window_->GetMenuOverlayY();
          Logger::LogMessage("Retrieved overlay position from SDL3Window: (" + 
                            std::to_string(overlay_x) + ", " + 
                            std::to_string(overlay_y) + ")");
        }
        SetWindowPos(overlay_hwnd, HWND_TOPMOST, overlay_x, overlay_y, 0, 0,
                     SWP_NOSIZE | SWP_NOACTIVATE);
        Logger::LogMessage("Set menu overlay window position to (" + 
                          std::to_string(overlay_x) + ", " + 
                          std::to_string(overlay_y) + ") and stay on top");
      } else {
        Logger::LogMessage(
            "Failed to get overlay window handle for transparency");
      }
    }
  }
}

bool SimpleClient::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  return false;
}

void SimpleClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
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

void SimpleClient::OnLoadError(CefRefPtr<CefBrowser> browser,
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

void SimpleClient::OnLoadStart(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               TransitionType transition_type) {
  CEF_REQUIRE_UI_THREAD();

  if (frame->IsMain()) {
    // Main frame started loading
    Logger::LogMessage("Main frame started loading");
  }
}

void SimpleClient::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            int httpStatusCode) {
  CEF_REQUIRE_UI_THREAD();

  if (frame->IsMain() && menu_overlay_active_ && menu_overlay_browser_ && 
      browser->IsSame(menu_overlay_browser_)) {
    // Menu overlay finished loading - get content height for auto-sizing
    Logger::LogMessage("Menu overlay loaded, getting content height for auto-sizing");
    
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

bool SimpleClient::OnBeforeDownload(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item,
    const CefString &suggested_name,
    CefRefPtr<CefBeforeDownloadCallback> callback) {
  CEF_REQUIRE_UI_THREAD();

  // Set the download path
  std::string download_path = GetDownloadPath(suggested_name.ToString());
  callback->Continue(download_path, false);

  return true;
}

void SimpleClient::OnDownloadUpdated(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item,
    CefRefPtr<CefDownloadItemCallback> callback) {
  CEF_REQUIRE_UI_THREAD();

  if (download_item->IsComplete()) {
    Logger::LogMessage("Download completed: " +
                       download_item->GetFullPath().ToString());
  }
}

void SimpleClient::SpawnNewWindow() {
  // This would create a new window instance
  // Implementation depends on application architecture
}

// OSR specific methods
void SimpleClient::SendMouseClickEvent(int x, int y,
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

void SimpleClient::SendMouseMoveEvent(int x, int y, bool mouse_leave) {
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

void SimpleClient::SendMouseWheelEvent(int x, int y, int delta_x, int delta_y) {
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

void SimpleClient::OpenEditor(int x, int y, int width, int height) {
  Logger::LogMessage("OpenEditor called with position (" + std::to_string(x) + 
                     ", " + std::to_string(y) + ") and size " + 
                     std::to_string(width) + "x" + std::to_string(height));

  if (!window_) {
    Logger::LogMessage("OpenEditor: No window available");
    return;
  }

  // Enable the editor sublayer in the window
  window_->EnableEditor(true);
  window_->SetEditorPosition(x, y, width, height);

  // Create a new CEF browser for the editor using OSR
  CefWindowInfo window_info;
  window_info.SetAsWindowless(nullptr); // OSR mode

  CefBrowserSettings browser_settings;
  browser_settings.windowless_frame_rate = 60; // 60 FPS for smooth rendering

  // Create the browser for Monaco Editor
  std::string editor_url = "miko://editor/monaco.html";
  Logger::LogMessage("Creating editor browser with URL: " + editor_url);

  CefBrowserHost::CreateBrowser(window_info, this, editor_url, browser_settings,
                                nullptr, nullptr);
}

void SimpleClient::CloseEditor() {
  Logger::LogMessage("CloseEditor called");

  if (!window_) {
    Logger::LogMessage("CloseEditor: No window available");
    return;
  }

  // Disable the editor sublayer
  window_->EnableEditor(false);

  // Close the editor browser if it exists
  if (browser_list_.size() >= 2) {
    // The second browser should be the editor browser
    auto editor_browser = browser_list_[1];
    if (editor_browser) {
      Logger::LogMessage("Closing editor browser");
      editor_browser->GetHost()->CloseBrowser(false);
    }
  }

  Logger::LogMessage("Editor closed");
}

void SimpleClient::OpenMenuOverlay(const std::string &section, int x, int y, int width, int height) {
  Logger::LogMessage("OpenMenuOverlay called for section: " + section + 
                     " at position (" + std::to_string(x) + ", " + 
                     std::to_string(y) + ") with size " + 
                     std::to_string(width) + "x" + std::to_string(height));

  if (menu_overlay_active_) {
    Logger::LogMessage("Menu overlay already active, closing existing overlay first");
    CloseMenuOverlay();
  }

  if (!window_) {
    Logger::LogMessage("OpenMenuOverlay: No window available");
    return;
  }

  // Set the current menu section and position in the window
  window_->SetCurrentMenuSection(section);
  window_->SetMenuOverlayPosition(x, y);
  window_->SetMenuOverlayVisible(true);

  // Create a new CEF browser for the menu overlay
  CefWindowInfo window_info;
  
  // Get the main window handle for parenting
  HWND parent_hwnd = window_->GetHWND();
  if (!parent_hwnd) {
    Logger::LogMessage("OpenMenuOverlay: Failed to get parent window handle");
    return;
  }

  // Configure as a child window with transparency support
  window_info.SetAsChild(parent_hwnd, CefRect(x, y, width, height));
  window_info.ex_style |= WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST;

  CefBrowserSettings browser_settings;
  browser_settings.background_color = 0x00000000; // Transparent background

  // Build the overlay URL with parameters
  std::string overlay_url = BuildOverlayURL(section, x, y, width, height);
  Logger::LogMessage("Creating menu overlay browser with URL: " + overlay_url);

  // Create the browser
  auto browser = CefBrowserHost::CreateBrowserSync(window_info, this, overlay_url, 
                                                   browser_settings, nullptr, nullptr);
  
  if (browser) {
    menu_overlay_browser_ = browser;
    menu_overlay_active_ = true;
    Logger::LogMessage("Menu overlay browser created successfully");
  } else {
    Logger::LogMessage("Failed to create menu overlay browser");
    window_->SetMenuOverlayVisible(false);
  }
}

void SimpleClient::CloseMenuOverlay() {
  Logger::LogMessage("CloseMenuOverlay called");

  if (!menu_overlay_active_ || !menu_overlay_browser_) {
    Logger::LogMessage("CloseMenuOverlay: No active menu overlay to close");
    return;
  }

  // Close the overlay browser
  Logger::LogMessage("Closing menu overlay browser");
  menu_overlay_browser_->GetHost()->CloseBrowser(false);
  
  // Reset tracking variables (will be set to null in OnBeforeClose)
  menu_overlay_active_ = false;
  
  // Update window state
  if (window_) {
    window_->SetMenuOverlayVisible(false);
    window_->SetCurrentMenuSection("");
  }

  Logger::LogMessage("Menu overlay close initiated");
}

std::string SimpleClient::GetMenuOverlayHTML(const std::string &section) {
  // Return HTML content for the specified menu section
  return;
}

std::string SimpleClient::BuildOverlayURL(const std::string &section, int x,
                                          int y, int width, int height) {
  return "miko://menuoverlay/" + section + "?x=" + std::to_string(x) + 
         "&y=" + std::to_string(y) + "&width=" + std::to_string(width) + 
         "&height=" + std::to_string(height);
}