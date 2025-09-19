#pragma once

#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include <string>
#include <functional>
#include <map>

namespace SimpleIPC {
    // Message handler callback type
    using MessageHandler = std::function<std::string(const std::string&)>;
    
    // IPC Handler class for ExecuteJavaScript-based communication
    class IPCHandler {
    public:
        IPCHandler();
        
        // Handle IPC call
        std::string HandleCall(const std::string& method, const std::string& message);
        
        // Register a message handler
        void RegisterHandler(const std::string& method, MessageHandler handler);
        
        // Get singleton instance
        static IPCHandler& GetInstance();
        
    private:
        std::map<std::string, MessageHandler> handlers_;
    };
    
    // Initialize IPC system with ExecuteJavaScript
    void InitializeIPC(CefRefPtr<CefFrame> frame);
    
    // Test methods
    std::string HandlePing(const std::string& message);
    std::string HandleGetSystemInfo(const std::string& message);
    std::string HandleEcho(const std::string& message);
}