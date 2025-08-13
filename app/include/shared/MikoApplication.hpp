#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "include/wrapper/cef_byte_read_handler.h"

namespace miko {

// Binary resource structure for embedded resources
struct BinaryResource {
    const unsigned char* data;
    size_t size;
    const char* mime_type;
};

// Resource manager for binary embedded resources
class BinaryResourceManager {
public:
    static BinaryResourceManager& getInstance();
    
    // Register a binary resource
    void registerResource(const std::string& path, const BinaryResource& resource);
    
    // Get resource data as string
    bool getResourceString(const std::string& path, std::string& outData);
    
    // Get resource as CEF stream reader
    CefRefPtr<CefStreamReader> getResourceReader(const std::string& path);
    
    // Check if resource exists
    bool hasResource(const std::string& path) const;
    
    // Get resource MIME type
    std::string getResourceMimeType(const std::string& path) const;
    
private:
    BinaryResourceManager() = default;
    std::unordered_map<std::string, BinaryResource> resources_;
};

// Macro to declare binary resource data
#define DECLARE_BINARY_RESOURCE(name) \
    extern const unsigned char name##_data[]; \
    extern const size_t name##_size; \
    extern const char name##_mime_type[];

// Macro to define binary resource data
#define DEFINE_BINARY_RESOURCE(name, data_array, mime) \
    const unsigned char name##_data[] = data_array; \
    const size_t name##_size = sizeof(data_array); \
    const char name##_mime_type[] = mime;

// Macro to register binary resource
#define REGISTER_BINARY_RESOURCE(path, name) \
    BinaryResourceManager::getInstance().registerResource(path, {name##_data, name##_size, name##_mime_type});

// Declare existing binary resources from MikoApplication.cpp
extern unsigned char mikoide_index_html[];
extern unsigned int mikoide_index_html_len;

// Declare common resources
DECLARE_BINARY_RESOURCE(index_html)

// Initialize all binary resources
void initializeBinaryResources();

} // namespace miko