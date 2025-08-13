#pragma once

#include <string>
#include <memory>
#include <optional>
#include <functional>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

namespace shared
{
namespace util
{

/**
 * Optimized JSON/YAML utility class with error handling and performance optimizations
 */
class JsonYamlUtil
{
public:
    using json = nlohmann::json;
    using JsonCallback = std::function<void(const json&)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    // JSON Operations
    static std::optional<json> parseJson(const std::string& jsonStr) noexcept;
    static std::optional<json> parseJsonFromFile(const std::string& filePath) noexcept;
    static bool writeJsonToFile(const json& data, const std::string& filePath) noexcept;
    static std::string serializeJson(const json& data, bool pretty = false) noexcept;
    
    // YAML Operations
    static std::optional<YAML::Node> parseYaml(const std::string& yamlStr) noexcept;
    static std::optional<YAML::Node> parseYamlFromFile(const std::string& filePath) noexcept;
    static bool writeYamlToFile(const YAML::Node& data, const std::string& filePath) noexcept;
    static std::string serializeYaml(const YAML::Node& data) noexcept;
    
    // Conversion utilities
    static std::optional<json> yamlToJson(const YAML::Node& yamlNode) noexcept;
    static std::optional<YAML::Node> jsonToYaml(const json& jsonData) noexcept;
    
    // Validation utilities
    static bool isValidJson(const std::string& jsonStr) noexcept;
    static bool isValidYaml(const std::string& yamlStr) noexcept;
    
    // Response builders
    static json createSuccessResponse(const std::string& message = "") noexcept;
    static json createErrorResponse(const std::string& error) noexcept;
    static json createDataResponse(const json& data, const std::string& message = "") noexcept;
    
    // Safe data extraction methods
    static std::optional<std::string> safeGetString(const json& data, const std::string& key) noexcept;
    static std::optional<int> safeGetInt(const json& data, const std::string& key) noexcept;
    static std::optional<bool> safeGetBool(const json& data, const std::string& key) noexcept;
    static std::optional<json> safeGetObject(const json& data, const std::string& key) noexcept;
    
private:
    static std::string getLastError();
    static thread_local std::string lastError;
};

} // namespace util
} // namespace shared