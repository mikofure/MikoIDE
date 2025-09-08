#include "binaryresourceprovider.hpp"
#include "resourceutil.hpp"
#include "include/cef_response.h"
#include "include/cef_stream.h"
#include "include/wrapper/cef_stream_resource_handler.h"
#include "logger.hpp"
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
    
    // Only handle miko://app requests
    if (url.find("miko://app") != 0) {
        Logger::LogMessage("BinaryResourceProvider: URL does not start with miko://app");
        return nullptr;
    }
    
    // Extract the path from the URL (remove "miko://app")
    std::string path = url.substr(10); // Remove "miko://app"
    if (path.empty() || path == "/") {
        path = "/index.html";
    }
    Logger::LogMessage("BinaryResourceProvider: Extracted path: " + path);
    
    // Get resource ID from path
    int resource_id = ResourceUtil::GetResourceId(path);
    Logger::LogMessage("BinaryResourceProvider: Resource ID: " + std::to_string(resource_id));
    if (resource_id == -1) {
        Logger::LogMessage("BinaryResourceProvider: Resource not found for path: " + path);
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