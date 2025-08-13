#include "shared/MikoApplication.hpp"
#include "include/wrapper/cef_byte_read_handler.h"

namespace miko {

// Implementation of BinaryResourceManager
BinaryResourceManager& BinaryResourceManager::getInstance() {
    static BinaryResourceManager instance;
    return instance;
}

void BinaryResourceManager::registerResource(const std::string& path, const BinaryResource& resource) {
    resources_[path] = resource;
}

bool BinaryResourceManager::getResourceString(const std::string& path, std::string& outData) {
    auto it = resources_.find(path);
    if (it != resources_.end()) {
        outData = std::string(reinterpret_cast<const char*>(it->second.data), it->second.size);
        return true;
    }
    return false;
}

CefRefPtr<CefStreamReader> BinaryResourceManager::getResourceReader(const std::string& path) {
    auto it = resources_.find(path);
    if (it != resources_.end()) {
        return CefStreamReader::CreateForHandler(
            new CefByteReadHandler(
                const_cast<unsigned char*>(it->second.data),
                it->second.size,
                nullptr
            )
        );
    }
    return nullptr;
}

bool BinaryResourceManager::hasResource(const std::string& path) const {
    return resources_.find(path) != resources_.end();
}

std::string BinaryResourceManager::getResourceMimeType(const std::string& path) const {
    auto it = resources_.find(path);
    if (it != resources_.end()) {
        return std::string(it->second.mime_type);
    }
    return "text/html"; // default
}

// Binary resource data is defined in MikoApplication.cpp
// const unsigned char index_html_data[] - defined externally
// const size_t index_html_size - defined externally  
const char index_html_mime_type[] = "text/html";

// Initialize all binary resources
void initializeBinaryResources() {
    auto& manager = BinaryResourceManager::getInstance();
    
    // Register resources with their paths using existing binary data
    manager.registerResource("index.html", {mikoide_index_html, mikoide_index_html_len, "text/html"});
}

} // namespace miko