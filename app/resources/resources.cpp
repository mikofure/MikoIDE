#include "resources.hpp"
#include "resourceutil.hpp"
#include "include/cef_response.h"
#include "include/cef_stream.h"
#include "include/wrapper/cef_stream_resource_handler.h"
#include "../utils/logger.hpp"
#include <string>

BinaryResourceProvider::BinaryResourceProvider() {
}

CefRefPtr<CefResourceHandler> BinaryResourceProvider::Create(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& scheme_name,
    CefRefPtr<CefRequest> request) {
    
    CEF_REQUIRE_IO_THREAD();
    
    std::string url = request->GetURL();
    Logger::LogMessage("BinaryResourceProvider: Handling URL: " + url);
    
    // Handle miko:// scheme requests
    if (url.find("miko://") != 0) {
        Logger::LogMessage("BinaryResourceProvider: URL does not start with miko://");
        return nullptr;
    }
    
    // Extract the path from the URL (remove "miko://")
    std::string path = url.substr(7); // Remove "miko://"
    
    // Remove query parameters if present
    size_t query_pos = path.find('?');
    if (query_pos != std::string::npos) {
        path = path.substr(0, query_pos);
    }
    
    // Handle different miko:// subdomains
    if (path.find("app") == 0) {
        // Remove "app" prefix for legacy support
        path = path.substr(3);
        if (path.empty() || path == "/") {
            path = "/index.html";
        }
    } else if (path.find("monaco") == 0) {
        // Handle monaco subdomain - ensure proper path format
        if (path.find("monaco/") == 0) {
            path = "/" + path; // Convert "monaco/index.html" to "/monaco/index.html"
        } else if (path == "monaco") {
            path = "/monaco/index.html"; // Default to index.html for bare monaco
        }
    } else if (path.find("menuoverlay") == 0) {
        // Handle menuoverlay subdomain - ensure proper path format
        if (path.find("menuoverlay/") == 0) {
            path = "/" + path; // Convert "menuoverlay/index.html" to "/menuoverlay/index.html"
        } else if (path == "menuoverlay") {
            path = "/menuoverlay/index.html"; // Default to index.html for bare menuoverlay
        }
    } else {
        // Default handling for other paths
        if (path.empty() || path == "/") {
            path = "/index.html";
        } else if (path[0] != '/') {
            path = "/" + path;
        }
    }
    Logger::LogMessage("BinaryResourceProvider: Extracted path: " + path);
    
    // Get resource ID from path
    int resource_id = ResourceUtil::GetResourceId(path);
    Logger::LogMessage("BinaryResourceProvider: Resource ID for path '" + path + "': " + std::to_string(resource_id));
    if (resource_id == -1) {
        Logger::LogMessage("BinaryResourceProvider: ERROR - Resource not found for path: " + path);
        Logger::LogMessage("BinaryResourceProvider: Available paths: /index.html, /editor.html, /menuoverlay/index.html, /main.css, /main.js");
        return nullptr; // Resource not found
    }
    
    // Load resource data
    std::vector<uint8_t> resource_data = ResourceUtil::LoadBinaryResource(resource_id);
    if (resource_data.empty()) {
        return nullptr;
    }
    
    // Create stream reader
    CefRefPtr<CefStreamReader> stream = ResourceUtil::CreateResourceReader(resource_data);
    if (!stream) {
        return nullptr;
    }
    
    // Get MIME type
    std::string mime_type = ResourceUtil::GetMimeType(path);
    
    // Create and return the resource handler
    return new CefStreamResourceHandler(mime_type, stream);
}