#include "overlay.hpp"
#include "windowed.hpp"
#include "offscreen.hpp"
#include "../resources/resourceutil.hpp"
#include "../utils/logger.hpp"
#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"
#include <windows.h>

OverlayManager::OverlayManager(SDL3Window* window)
    : window_(window), menu_overlay_active_(false), menu_overlay_browser_(nullptr) {
}

void OverlayManager::OpenMenuOverlay(const std::string &section, int x, int y, int width, int height) {
  CEF_REQUIRE_UI_THREAD();

  try {
    // Check if menu overlay is already active
    if (menu_overlay_active_) {
      Logger::LogMessage("Menu overlay already active, ignoring request");
      return;
    }

    Logger::LogMessage("OpenMenuOverlay called - section: " + section +
                       ", x: " + std::to_string(x) +
                       ", y: " + std::to_string(y) +
                       ", width: " + std::to_string(width) +
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
        int overlayWidth = (width > 0) ? width : 300;   // Default width if not specified
        int overlayHeight = 0; // Default to auto height
        
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
        
        Logger::LogMessage("Using hardcoded height " + std::to_string(overlayHeight) + " for section: " + section);

        // Fixed positioning - always use x coordinate or cursor, y is always 32
        int overlayX, overlayY;
        if (x != 0) {
          // Use provided x coordinate
          overlayX = x;
          Logger::LogMessage("Using provided x coordinate: " + std::to_string(x));
        } else {
          // Use cursor x position if no specific x coordinate provided
          overlayX = cursor_pos.x + 10; // Offset slightly from cursor
          Logger::LogMessage("Using cursor x position: " + std::to_string(cursor_pos.x));
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
            "Overlay window bounds - x: " + std::to_string(overlayX) +
            ", y: " + std::to_string(overlayY) +
            ", w: " + std::to_string(overlayWidth) +
            ", h: " + std::to_string(overlayHeight));

        CefBrowserSettings browser_settings;
        browser_settings.javascript_close_windows = STATE_ENABLED;
        // Set transparent background using BGRA format (Alpha = 0 for
        // transparency)
        browser_settings.background_color =
            CefColorSetARGB(0, 0, 0, 0); // Fully transparent BGRA

        // Update SDL3Window overlay position tracking BEFORE creating browser
        if (window_) {
          window_->SetMenuOverlayPosition(overlayX, overlayY);
          window_->SetMenuOverlayVisible(true);
          window_->SetCurrentMenuSection(section);
        }

        // Create Steam-like overlay routing URL
        std::string overlay_url = BuildOverlayURL(section, overlayX, overlayY,
                                                  overlayWidth, overlayHeight);
        Logger::LogMessage("Overlay URL created, length: " +
                           std::to_string(overlay_url.length()));
        Logger::LogMessage("URL preview (first 200 chars): " +
                           overlay_url.substr(0, 200));

        bool browser_created = CefBrowserHost::CreateBrowser(
            window_info, nullptr, overlay_url, browser_settings, nullptr, nullptr);
        Logger::LogMessage("CreateBrowser result: " +
                           std::string(browser_created ? "true" : "false"));

        // Mark overlay as active if browser creation was successful
        if (browser_created) {
          menu_overlay_active_ = true;
          
          Logger::LogMessage("Menu overlay marked as active and position updated");
        }

        Logger::LogMessage("Menu overlay opened for section: " + section +
                           " at positioned (" + std::to_string(overlayX) +
                           ", " + std::to_string(overlayY) + ") with size (" +
                           std::to_string(overlayWidth) + "x" +
                           std::to_string(overlayHeight) +
                           ") and URL: " + overlay_url);
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

void OverlayManager::CloseMenuOverlay() {
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
    ResetMenuOverlayTracking();
    
    // Reset SDL3Window overlay position tracking
    if (window_) {
      window_->SetMenuOverlayVisible(false);
      window_->SetMenuOverlayPosition(0, 32);
      window_->SetCurrentMenuSection("");
    }

  } catch (const std::exception &ex) {
    Logger::LogMessage("Exception in CloseMenuOverlay: " +
                       std::string(ex.what()));
    // Reset tracking variables even on exception
    ResetMenuOverlayTracking();
    
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
    ResetMenuOverlayTracking();
    
    // Reset SDL3Window overlay position tracking even on exception
    if (window_) {
      window_->SetMenuOverlayVisible(false);
      window_->SetMenuOverlayPosition(0, 0);
      window_->SetCurrentMenuSection("");
    }
  }
}

std::string OverlayManager::GetMenuOverlayHTML(const std::string &section) {
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

std::string OverlayManager::BuildOverlayURL(const std::string &section, int x,
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

void OverlayManager::ResetMenuOverlayTracking() {
  menu_overlay_active_ = false;
  menu_overlay_browser_ = nullptr;
}