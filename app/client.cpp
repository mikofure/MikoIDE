#include "client.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "internal/simpleipc.hpp"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_app.h"
#include "include/views/cef_window.h"
#include <SDL3/SDL.h>

// Global variables
extern SDL_Window* g_sdl_window;
extern CefRefPtr<CefWindow> g_cef_window;

// CloseBrowserTask implementation
CloseBrowserTask::CloseBrowserTask(CefRefPtr<SimpleClient> client, bool force_close)
    : client_(client), force_close_(force_close) {
}

void CloseBrowserTask::Execute() {
    client_->DoCloseAllBrowsers(force_close_);
}

// SimpleClient implementation
SimpleClient::SimpleClient() {
    // Create message router for JavaScript-to-C++ communication
    CefMessageRouterConfig config;
    message_router_ = CefMessageRouterBrowserSide::Create(config);
    message_router_->AddHandler(this, false);
    
    // Create the binary resource provider for miko:// protocol
    resource_provider_ = new BinaryResourceProvider();
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

bool SimpleClient::OnQuery(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          int64_t query_id,
                          const CefString& request,
                          bool persistent,
                          CefRefPtr<CefMessageRouterBrowserSide::Callback> callback) {
    CEF_REQUIRE_UI_THREAD();
    
    std::string request_str = request.ToString();
    
    if (request_str == "minimize_window") {
        if (g_cef_window) {
            g_cef_window->Minimize();
            callback->Success("");
            return true;
        }
    }
    else if (request_str == "maximize_window") {
        if (g_cef_window) {
            g_cef_window->Maximize();
            callback->Success("");
            return true;
        }
    }
    else if (request_str == "restore_window") {
        if (g_cef_window) {
            g_cef_window->Restore();
            callback->Success("");
            return true;
        }
    }
    else if (request_str == "close_window") {
        if (g_cef_window) {
            g_cef_window->Close();
            callback->Success("");
            return true;
        }
    }
    else if (request_str == "get_window_state") {
        if (g_cef_window) {
            if (g_cef_window->IsMaximized()) {
                callback->Success("maximized");
            } else {
                callback->Success("normal");
            }
            return true;
        }
    }
    else if (request_str == "spawn_new_window") {
        // Create a new browser window
        SpawnNewWindow();
        callback->Success("");
        return true;
    }
    else if (request_str == "create_new_file") {
        // Handle new file creation via cefQuery
        browser->GetMainFrame()->ExecuteJavaScript(
            "if (window.createNewFileFromCEF) { window.createNewFileFromCEF(); }",
            browser->GetMainFrame()->GetURL(), 0);
        callback->Success("");
        return true;
    }
    else if (request_str.substr(0, 9) == "ipc_call:") {
        // Handle IPC calls: format is "ipc_call:method:message"
        std::string remaining = request_str.substr(9);
        size_t colon_pos = remaining.find(':');
        
        std::string method;
        std::string message;
        
        if (colon_pos != std::string::npos) {
            method = remaining.substr(0, colon_pos);
            message = remaining.substr(colon_pos + 1);
        } else {
            method = remaining;
            message = "";
        }
        
        // Handle the IPC call using the singleton handler
        std::string result = SimpleIPC::IPCHandler::GetInstance().HandleCall(method, message);
        callback->Success(result);
        return true;
    }
    
    return false; // Request not handled
}

void SimpleClient::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                const CefString& title) {
    CEF_REQUIRE_UI_THREAD();
    
    std::string windowTitle = "SwipeIDE - " + title.ToString();
    if (AppConfig::IsDebugMode()) {
        windowTitle += " [DEBUG]";
    } else {
        windowTitle += " [RELEASE]";
    }
    
    // Window title is now managed by CEF views
    // SDL window is hidden and used only for compatibility
}

void SimpleClient::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefContextMenuParams> params,
                                     CefRefPtr<CefMenuModel> model) {
    CEF_REQUIRE_UI_THREAD();
    
    // Clear all default context menu items
    model->Clear();
    
    // Optionally add custom menu items here
    // For now, we disable all context menu functionality
}

bool SimpleClient::OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefContextMenuParams> params,
                                      int command_id,
                                      EventFlags event_flags) {
    CEF_REQUIRE_UI_THREAD();
    
    // Handle custom context menu commands here if any
    // Return true to indicate the command was handled
    return false;
}

bool SimpleClient::OnDragEnter(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefDragData> dragData,
                             CefDragHandler::DragOperationsMask mask) {
    CEF_REQUIRE_UI_THREAD();
    
    // Allow drag operations for app-region dragging
    // This enables CSS -webkit-app-region: drag functionality
    return false; // Allow default drag behavior
}

void SimpleClient::OnDraggableRegionsChanged(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const std::vector<CefDraggableRegion>& regions) {
    CEF_REQUIRE_UI_THREAD();
    
    // Handle draggable regions for CSS -webkit-app-region: drag
    // This method is called when the web page defines draggable regions
    if (g_cef_window) {
        // Set draggable regions on the CEF window
        // This enables CSS-based window dragging functionality
        g_cef_window->SetDraggableRegions(regions);
        
        Logger::LogMessage("Updated draggable regions: " + std::to_string(regions.size()) + " regions");
    }
}

bool SimpleClient::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                bool user_gesture,
                                bool is_redirect) {
    CEF_REQUIRE_UI_THREAD();
    
    // Notify message router
    if (message_router_) {
        message_router_->OnBeforeBrowse(browser, frame);
    }
    
    // Allow navigation within the same origin/domain
    // Block external navigation that might open new windows
    return false; // Allow navigation
}

bool SimpleClient::OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  const CefString& target_url,
                                  CefRequestHandler::WindowOpenDisposition target_disposition,
                                  bool user_gesture) {
    CEF_REQUIRE_UI_THREAD();
    
    // Check if this is a Ctrl+Shift+N request (new window with user gesture)
    if (user_gesture && target_disposition == CEF_WOD_NEW_WINDOW) {
        // Allow Ctrl+Shift+N to spawn new windows
        SpawnNewWindow();
        return true; // Handle the request ourselves
    }
    
    // Block all other attempts to open new tabs/windows
    // This prevents Ctrl+Click, middle-click, and target="_blank" from opening new windows
    if (target_disposition == CEF_WOD_NEW_FOREGROUND_TAB ||
        target_disposition == CEF_WOD_NEW_BACKGROUND_TAB ||
        target_disposition == CEF_WOD_NEW_POPUP ||
        target_disposition == CEF_WOD_NEW_WINDOW) {
        
        // Instead of opening a new window/tab, navigate in the current frame
        browser->GetMainFrame()->LoadURL(target_url);
        return true; // Block the default behavior
    }
    
    return false; // Allow other dispositions
}

bool SimpleClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) {
    CEF_REQUIRE_UI_THREAD();
    
    // Forward to message router
    if (message_router_) {
        return message_router_->OnProcessMessageReceived(browser, frame, source_process, message);
    }
    
    return false;
}

CefRefPtr<CefResourceHandler> SimpleClient::GetResourceHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request) {
    CEF_REQUIRE_IO_THREAD();
    
    // miko:// protocol requests are now handled by the registered scheme handler factory
    return nullptr;
}

bool SimpleClient::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  int popup_id,
                                  const CefString& target_url,
                                  const CefString& target_frame_name,
                                  CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                                  bool user_gesture,
                                  const CefPopupFeatures& popupFeatures,
                                  CefWindowInfo& windowInfo,
                                  CefRefPtr<CefClient>& client,
                                  CefBrowserSettings& settings,
                                  CefRefPtr<CefDictionaryValue>& extra_info,
                                  bool* no_javascript_access) {
    CEF_REQUIRE_UI_THREAD();

    // Log popup attempt
    Logger::LogMessage("Popup blocked: " + target_url.ToString());

    // Block all popups to prevent unwanted Chrome UI elements
  // Only allow controlled new windows via SpawnNewWindow for legitimate user gestures
  if (user_gesture && target_disposition == CEF_WOD_NEW_WINDOW) {
    // For legitimate user gestures like Ctrl+Shift+N, use our controlled SpawnNewWindow
    SpawnNewWindow();
  }

    // Always return true to block the popup
    return true;
}

void SimpleClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();
    browser_list_.push_back(browser);
    
    // Register message router with the browser
    if (message_router_) {
        // No specific browser registration needed for message router
    }
    
    std::string mode = AppConfig::IsDebugMode() ? "DEBUG" : "RELEASE";
    std::string url = AppConfig::GetStartupUrl();
    Logger::LogMessage("CEF Browser started in " + mode + " mode");
    Logger::LogMessage("Loading URL: " + url);
}

  bool SimpleClient::DoClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();
    return false;
}

void SimpleClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();
    
    // Clean up message router when all browsers are closed
    if (browser_list_.empty() && message_router_) {
        message_router_->RemoveHandler(this);
        message_router_ = nullptr;
    }
    
    BrowserList::iterator bit = browser_list_.begin();
    for (; bit != browser_list_.end(); ++bit) {
        if ((*bit)->IsSame(browser)) {
            browser_list_.erase(bit);
            break;
        }
    }

    if (browser_list_.empty()) {
        extern bool g_running;
        g_running = false;
        CefQuitMessageLoop();
    }
}

void SimpleClient::OnLoadError(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              ErrorCode errorCode,
                              const CefString& errorText,
                              const CefString& failedUrl) {
    CEF_REQUIRE_UI_THREAD();

    if (errorCode == ERR_ABORTED)
        return;

    if (AppConfig::IsDebugMode() && failedUrl.ToString().find("localhost:3000") != std::string::npos) {
        std::string errorHtml = R"(
            <html>
            <head><title>Development Server Not Running</title></head>
            <body style="font-family: Arial, sans-serif; padding: 40px; background: #f5f5f5;">
                <div style="background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1);">
                    <h1 style="color: #e74c3c; margin-top: 0;">🚫 Development Server Not Running</h1>
                    <p>The React development server is not running on <strong>localhost:3000</strong>.</p>
                    <h3>To start the development server:</h3>
                    <ol>
                        <li>Open a terminal in the <code>renderer</code> directory</li>
                        <li>Run: <code style="background: #f8f9fa; padding: 2px 6px; border-radius: 3px;">bun run dev</code></li>
                        <li>Wait for the server to start</li>
                        <li>Refresh this page</li>
                    </ol>
                    <p style="margin-top: 30px; padding: 15px; background: #e8f4f8; border-left: 4px solid #3498db; border-radius: 4px;">
                        <strong>💡 Tip:</strong> The development server provides hot reloading and debugging features.
                    </p>
                </div>
            </body>
            </html>
        )";
        frame->LoadURL("data:text/html," + errorHtml);
        return;
    }

    std::string errorHtml = "<html><body bgcolor=\"white\">" +
                           std::string("<h2>Failed to load URL ") +
                           std::string(failedUrl) +
                           std::string(" with error ") +
                           std::string(errorText) +
                           std::string(" (") +
                           std::to_string(errorCode) +
                           std::string(").</h2></body></html>");
    frame->LoadURL("data:text/html," + errorHtml);
}

void SimpleClient::OnLoadStart(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              TransitionType transition_type) {
    CEF_REQUIRE_UI_THREAD();
    
    if (frame->IsMain()) {
        std::string mode = AppConfig::IsDebugMode() ? "DEBUG" : "RELEASE";
        Logger::LogMessage("Loading page in " + mode + " mode...");
        
        // Initialize IPC system
        SimpleIPC::InitializeIPC(frame);
    }
}

void SimpleClient::CloseAllBrowsers(bool force_close) {
    if (!CefCurrentlyOn(TID_UI)) {
        CefPostTask(TID_UI, new CloseBrowserTask(this, force_close));
        return;
    }

    if (browser_list_.empty())
        return;

    BrowserList::const_iterator it = browser_list_.begin();
    for (; it != browser_list_.end(); ++it)
        (*it)->GetHost()->CloseBrowser(force_close);
}

void SimpleClient::DoCloseAllBrowsers(bool force_close) {
    if (browser_list_.empty())
        return;

    BrowserList::const_iterator it = browser_list_.begin();
    for (; it != browser_list_.end(); ++it)
        (*it)->GetHost()->CloseBrowser(force_close);
}

CefRefPtr<CefBrowser> SimpleClient::GetFirstBrowser() {
    if (!browser_list_.empty()) {
        return browser_list_.front();
    }
    return nullptr;
}

bool SimpleClient::HasBrowsers() {
    return !browser_list_.empty();
}

bool SimpleClient::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                                 const CefKeyEvent& event,
                                 CefEventHandle os_event,
                                 bool* is_keyboard_shortcut) {
    CEF_REQUIRE_UI_THREAD();
    
    // Block dangerous Chrome shortcuts that could expose browser UI
    if (event.type == KEYEVENT_KEYDOWN || event.type == KEYEVENT_RAWKEYDOWN) {
        // Block F12 (Developer Tools)
        if (event.windows_key_code == VK_F12) {
            Logger::LogMessage("Blocked F12 developer tools shortcut");
            return true; // Block the key event
        }
        
        // Block Ctrl+Shift+I (Developer Tools)
        // if (event.windows_key_code == 'I' && 
        //     (event.modifiers & EVENTFLAG_CONTROL_DOWN) && 
        //     (event.modifiers & EVENTFLAG_SHIFT_DOWN)) {
        //     Logger::LogMessage("Blocked Ctrl+Shift+I developer tools shortcut");
        //     return true;
        // }
        
        // Block Ctrl+Shift+J (Console)
        if (event.windows_key_code == 'J' && 
            (event.modifiers & EVENTFLAG_CONTROL_DOWN) && 
            (event.modifiers & EVENTFLAG_SHIFT_DOWN)) {
            Logger::LogMessage("Blocked Ctrl+Shift+J console shortcut");
            return true;
        }
        
        // Block Ctrl+U (View Source)
        if (event.windows_key_code == 'U' && 
            (event.modifiers & EVENTFLAG_CONTROL_DOWN) && 
            !(event.modifiers & EVENTFLAG_SHIFT_DOWN)) {
            Logger::LogMessage("Blocked Ctrl+U view source shortcut");
            return true;
        }
        
        // Block Ctrl+Shift+C (Inspect Element)
        if (event.windows_key_code == 'C' && 
            (event.modifiers & EVENTFLAG_CONTROL_DOWN) && 
            (event.modifiers & EVENTFLAG_SHIFT_DOWN)) {
            Logger::LogMessage("Blocked Ctrl+Shift+C inspect element shortcut");
            return true;
        }
        
        // Block F5 and Ctrl+R (Refresh) - handled by frontend
        if (event.windows_key_code == VK_F5 || 
            (event.windows_key_code == 'R' && (event.modifiers & EVENTFLAG_CONTROL_DOWN))) {
            Logger::LogMessage("Blocked browser refresh shortcut - handled by frontend");
            return true;
        }
        
        // Block Ctrl+Shift+Delete (Clear Browsing Data)
        if (event.windows_key_code == VK_DELETE && 
            (event.modifiers & EVENTFLAG_CONTROL_DOWN) && 
            (event.modifiers & EVENTFLAG_SHIFT_DOWN)) {
            Logger::LogMessage("Blocked Ctrl+Shift+Delete clear data shortcut");
            return true;
        }
        
        // Block Ctrl+N (New Window) - handled by frontend
        if (event.windows_key_code == 'N' && 
            (event.modifiers & EVENTFLAG_CONTROL_DOWN) && 
            !(event.modifiers & EVENTFLAG_SHIFT_DOWN)) {
            Logger::LogMessage("Blocked Ctrl+N new window shortcut - handled by frontend");
            return true;
        }
        
        // Block Ctrl+T (New Tab) - handled by frontend
        if (event.windows_key_code == 'T' && 
            (event.modifiers & EVENTFLAG_CONTROL_DOWN) && 
            !(event.modifiers & EVENTFLAG_SHIFT_DOWN)) {
            Logger::LogMessage("Blocked Ctrl+T new tab shortcut - handled by frontend");
            return true;
        }
        
        // Block Ctrl+Shift+N (New Incognito Window) - handled by frontend
        if (event.windows_key_code == 'N' && 
            (event.modifiers & EVENTFLAG_CONTROL_DOWN) && 
            (event.modifiers & EVENTFLAG_SHIFT_DOWN)) {
            Logger::LogMessage("Blocked Ctrl+Shift+N incognito window shortcut - handled by frontend");
            return true;
        }
    }
    
    // Allow other key events to proceed to frontend
    return false;
}

bool SimpleClient::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefDownloadItem> download_item,
                                    const CefString& suggested_name,
                                    CefRefPtr<CefBeforeDownloadCallback> callback) {
  CEF_REQUIRE_UI_THREAD();

  // Create downloads directory if it doesn't exist
  std::string downloads_path = "downloads/" + suggested_name.ToString();
  
  // Log download start
  Logger::LogMessage("Download started: " + suggested_name.ToString());
  
  // Continue download without showing dialog (show_dialog = false)
  callback->Continue(downloads_path, false);
  
  return true;
}

void SimpleClient::OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefDownloadItem> download_item,
                                     CefRefPtr<CefDownloadItemCallback> callback) {
  CEF_REQUIRE_UI_THREAD();

  if (download_item->IsComplete()) {
    Logger::LogMessage("Download completed: " + download_item->GetFullPath().ToString());
  } else if (download_item->IsCanceled()) {
    Logger::LogMessage("Download canceled: " + download_item->GetFullPath().ToString());
  } else {
    // Log download progress
    int64_t received = download_item->GetReceivedBytes();
    int64_t total = download_item->GetTotalBytes();
    if (total > 0) {
      int progress = static_cast<int>((received * 100) / total);
      Logger::LogMessage("Download progress: " + std::to_string(progress) + "% - " + download_item->GetFullPath().ToString());
    }
  }
}

void SimpleClient::SpawnNewWindow() {
    CEF_REQUIRE_UI_THREAD();
    
    // Get the startup URL from config
    std::string url = AppConfig::GetStartupUrl();
    
    // Create window info for the new browser window
    CefWindowInfo window_info;
    window_info.SetAsPopup(nullptr, "SwipeIDE - New Window");
    window_info.bounds.x = 100;
    window_info.bounds.y = 100;
    window_info.bounds.width = 1200;
    window_info.bounds.height = 800;
    
    // Browser settings
    CefBrowserSettings browser_settings;
    // Note: web_security, file_access_from_file_urls, and universal_access_from_file_urls
    // are not available in this version of CEF
    
    // Create a new client instance for the new window
    CefRefPtr<SimpleClient> new_client = new SimpleClient();
    
    // Create the new browser window
    CefBrowserHost::CreateBrowser(window_info, new_client, url, browser_settings, nullptr, nullptr);
    
    Logger::LogMessage("Spawned new browser window");
}