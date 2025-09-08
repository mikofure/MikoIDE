#include "resourceutil.hpp"
#include "webapp.hpp"
#include <map>
#include <algorithm>
#include "include/cef_stream.h"

namespace ResourceUtil {
    
    // Resource path to ID mapping
    static const std::map<std::string, int> resourceMap = {
        {"/index.html", IDR_HTML_INDEX},
        {"/main.css", IDR_CSS_MAIN},
        {"/main.js", IDR_JS_MAIN}
    };
    
    int GetResourceId(const std::string& path) {
        auto it = resourceMap.find(path);
        if (it != resourceMap.end()) {
            return it->second;
        }
        return -1; // Resource not found
    }
    
    std::vector<uint8_t> LoadBinaryResource(int resource_id) {
        // Load HTML content from webapp.hpp
        if (resource_id == IDR_HTML_INDEX) {
            const char* html_content = GetWebAppHTML();
            unsigned int html_size = GetWebAppHTMLSize();
            
            std::vector<uint8_t> data(html_content, html_content + html_size);
            return data;
        }
        
        // Return empty vector for unknown resources
        return std::vector<uint8_t>();
    }
    
    std::string GetMimeType(const std::string& path) {
        std::string lower_path = path;
        std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), ::tolower);
        
        // Helper function to check if string ends with suffix
        auto ends_with = [](const std::string& str, const std::string& suffix) {
            return str.size() >= suffix.size() && 
                   str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        };
        
        if (ends_with(lower_path, ".html") || ends_with(lower_path, ".htm")) {
            return "text/html";
        } else if (ends_with(lower_path, ".css")) {
            return "text/css";
        } else if (ends_with(lower_path, ".js")) {
            return "application/javascript";
        } else if (ends_with(lower_path, ".json")) {
            return "application/json";
        } else if (ends_with(lower_path, ".png")) {
            return "image/png";
        } else if (ends_with(lower_path, ".jpg") || ends_with(lower_path, ".jpeg")) {
            return "image/jpeg";
        } else if (ends_with(lower_path, ".svg")) {
            return "image/svg+xml";
        }
        
        return "application/octet-stream";
    }
    
    CefRefPtr<CefStreamReader> CreateResourceReader(const std::vector<uint8_t>& data) {
        if (data.empty()) {
            return nullptr;
        }
        
        return CefStreamReader::CreateForData(
            const_cast<void*>(static_cast<const void*>(data.data())), 
            data.size()
        );
    }
}