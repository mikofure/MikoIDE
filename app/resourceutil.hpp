#pragma once
#include <string>
#include <vector>
#include "include/cef_stream.h"

namespace ResourceUtil {
    // Resource ID definitions
    enum ResourceId {
        IDR_HTML_INDEX = 100,
        IDR_CSS_MAIN = 101,
        IDR_JS_MAIN = 102
    };
    
    // Get resource ID from path
    int GetResourceId(const std::string& path);
    
    // Load binary resource by ID
    std::vector<uint8_t> LoadBinaryResource(int resource_id);
    
    // Get MIME type from file extension
    std::string GetMimeType(const std::string& path);
    
    // Create CEF stream reader from resource data
    CefRefPtr<CefStreamReader> CreateResourceReader(const std::vector<uint8_t>& data);
}