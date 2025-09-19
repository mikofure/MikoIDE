#include "simpleipc.hpp"
#include "include/cef_version.h"
#include <sstream>
#include <chrono>
#include <ctime>

namespace SimpleIPC {
    
    IPCHandler::IPCHandler() {
        // Register default handlers
        RegisterHandler("ping", HandlePing);
        RegisterHandler("getSystemInfo", HandleGetSystemInfo);
        RegisterHandler("echo", HandleEcho);
    }
    
    std::string IPCHandler::HandleCall(const std::string& method, const std::string& message) {
        auto it = handlers_.find(method);
        if (it != handlers_.end()) {
            try {
                return it->second(message);
            } catch (const std::exception& e) {
                return "Error: " + std::string(e.what());
            }
        } else {
            return "Error: Unknown method: " + method;
        }
    }
    
    void IPCHandler::RegisterHandler(const std::string& method, MessageHandler handler) {
        handlers_[method] = handler;
    }
    
    IPCHandler& IPCHandler::GetInstance() {
        static IPCHandler instance;
        return instance;
    }
    
    void InitializeIPC(CefRefPtr<CefFrame> frame) {
        if (!frame.get()) return;
        
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
    
    std::string HandlePing(const std::string& message) {
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
    
    std::string HandleGetSystemInfo(const std::string& message) {
        std::stringstream ss;
        ss << "{";
        ss << "\"platform\": \"Windows\",";
        ss << "\"cef_version\": \"" << CEF_VERSION << "\",";
        ss << "\"timestamp\": \"" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << "\"";
        ss << "}";
        
        return ss.str();
    }
    
    std::string HandleEcho(const std::string& message) {
        return "Echo: " + message;
    }
}