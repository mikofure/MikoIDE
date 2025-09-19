#ifndef APP_CONFIG_HPP
#define APP_CONFIG_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include "webapp.hpp"

// Application configuration constants
#define APP_NAME "CEF Quickstart"
#define APP_VERSION "1.0.0"
#define APP_AUTHOR "Ariz Kamizuki"

// Window configuration
#define DEFAULT_WINDOW_WIDTH 1200
#define DEFAULT_WINDOW_HEIGHT 800
#define MIN_WINDOW_WIDTH 800
#define MIN_WINDOW_HEIGHT 600

// Debug configuration
#ifdef _DEBUG
    #define DEBUG_MODE 1
    #define LOG_LEVEL 0  // Verbose logging
#else
    #define DEBUG_MODE 0
    #define LOG_LEVEL 2  // Error logging only
#endif

// CEF configuration
#define CEF_MULTI_THREADED_MESSAGE_LOOP 0
#define CEF_ENABLE_SANDBOX 0

// Resource paths
#define RESOURCES_DIR "Resources"
#define LOCALES_DIR "locales"

// AppConfig class for runtime configuration
class AppConfig {
private:
    static std::string UrlEncode(const std::string& value) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;
        
        for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
            std::string::value_type c = (*i);
            
            // Keep alphanumeric and other accepted characters intact
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
                continue;
            }
            
            // Any other characters are percent-encoded
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << int((unsigned char) c);
            escaped << std::nouppercase;
        }
        
        return escaped.str();
    }
    
public:
    static bool IsDebugMode() {
#ifdef _DEBUG
        return true;
#else
        return false;
#endif
    }
    
    static std::string GetStartupUrl() {
        if (IsDebugMode()) {
            return "http://localhost:5173";
        } else {
            // Use custom scheme for embedded resources
            return "miko://app/index.html";
        }
    }
    
    // Resource configuration
    static std::string GetAppOrigin() {
        return "miko://app/";
    }
    
    static std::string GetResourcePath() {
        return "index.html";
    }
};

#endif // APP_CONFIG_HPP