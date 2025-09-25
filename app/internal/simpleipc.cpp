#include "simpleipc.hpp"
#include "../client/client.hpp"
#include "include/cef_parser.h"
#include "include/cef_values.h"
#include "include/cef_version.h"
#include <chrono>
#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

// External reference to SDL window
extern std::unique_ptr<SDL3Window> g_sdl_window;

namespace SimpleIPC {

IPCHandler::IPCHandler() {
  // Register default handlers
  RegisterHandler("ping", HandlePing);
  RegisterHandler("getSystemInfo", HandleGetSystemInfo);
  RegisterHandler("echo", HandleEcho);
  RegisterHandler("resizeWindow", HandleResizeWindow);
}

std::string IPCHandler::HandleCall(const std::string &method,
                                   const std::string &message) {
  auto it = handlers_.find(method);
  if (it != handlers_.end()) {
    try {
      return it->second(message);
    } catch (const std::exception &e) {
      return "Error: " + std::string(e.what());
    }
  } else {
    return "Error: Unknown method: " + method;
  }
}

void IPCHandler::RegisterHandler(const std::string &method,
                                 MessageHandler handler) {
  handlers_[method] = handler;
}

IPCHandler &IPCHandler::GetInstance() {
  static IPCHandler instance;
  return instance;
}

void InitializeIPC(CefRefPtr<CefFrame> frame) {
  if (!frame.get())
    return;

  // Inject JavaScript code to create the nativeAPI object
  std::string js_code = R"(
            window.nativeAPI = {
                call: function(method, message) {
                    // This will be handled by cefQuery in the browser process
                    return new Promise(function(resolve, reject) {
                        if (window.cefQuery) {
                            window.cefQuery({
                                request: 'ipc_call:' + method + ':' + (message || ''),
                                onSuccess: function(response) {
                                    resolve(response);
                                },
                                onFailure: function(error_code, error_message) {
                                    reject(new Error(error_message));
                                }
                            });
                        } else {
                            reject(new Error('CEF Query not available'));
                        }
                    });
                }
            };
        )";

  frame->ExecuteJavaScript(js_code, frame->GetURL(), 0);
}

std::string HandlePing(const std::string &message) {
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << "Pong! Server time: " << std::ctime(&time_t);
  std::string result = ss.str();

  // Remove trailing newline
  if (!result.empty() && result.back() == '\n') {
    result.pop_back();
  }

  return result;
}

std::string HandleGetSystemInfo(const std::string &message) {
  std::stringstream ss;
  ss << "{";
  ss << "\"platform\": \"Windows\",";
  ss << "\"cef_version\": \"" << CEF_VERSION << "\",";
  ss << "\"timestamp\": \""
     << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count()
     << "\"";
  ss << "}";

  return ss.str();
}

std::string HandleEcho(const std::string &message) {
  return "Echo: " + message;
}

std::string HandleResizeWindow(const std::string &message) {
  try {
    // Parse JSON using CEF's native JSON parser
    CefRefPtr<CefValue> json_value =
        CefParseJSON(message, JSON_PARSER_ALLOW_TRAILING_COMMAS);

    if (!json_value || json_value->GetType() != VTYPE_DICTIONARY) {
      return "Error: Invalid JSON format";
    }

    CefRefPtr<CefDictionaryValue> json_dict = json_value->GetDictionary();

    if (!json_dict->HasKey("width") || !json_dict->HasKey("height")) {
      return "Error: Missing width or height parameters";
    }

    int width = json_dict->GetInt("width");
    int height = json_dict->GetInt("height");

    // Validate dimensions
    if (width < 400 || height < 300 || width > 3840 || height > 2160) {
      return "Error: Invalid window dimensions";
    }

    // Access the global SDL window and resize it
    if (g_sdl_window) {
      // Use SDL to resize the window
      if (auto sdl_window = g_sdl_window->GetSDLWindow()) {
        SDL_SetWindowSize(sdl_window, width, height);

        // The CEF browser resize notification will be handled
        // automatically by the SDL window event handler
        return "Window resized successfully";
      } else {
        return "Error: SDL window not available";
      }
    } else {
      return "Error: Global SDL window not initialized";
    }

  } catch (const std::exception &e) {
    return "Error: " + std::string(e.what());
  }
}
} // namespace SimpleIPC