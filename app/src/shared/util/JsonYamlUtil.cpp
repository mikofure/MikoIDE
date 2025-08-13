#include "shared/util/JsonYamlUtil.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace shared
{
namespace util
{

thread_local std::string JsonYamlUtil::lastError;

// JSON Operations
std::optional<JsonYamlUtil::json> JsonYamlUtil::parseJson(const std::string& jsonStr) noexcept
{
    try {
        if (jsonStr.empty()) {
            lastError = "Empty JSON string";
            return std::nullopt;
        }
        return json::parse(jsonStr);
    } catch (const json::parse_error& e) {
        lastError = "JSON parse error: " + std::string(e.what());
        return std::nullopt;
    } catch (const std::exception& e) {
        lastError = "JSON error: " + std::string(e.what());
        return std::nullopt;
    }
}

std::optional<JsonYamlUtil::json> JsonYamlUtil::parseJsonFromFile(const std::string& filePath) noexcept
{
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            lastError = "Cannot open file: " + filePath;
            return std::nullopt;
        }
        
        json data;
        file >> data;
        return data;
    } catch (const json::parse_error& e) {
        lastError = "JSON file parse error: " + std::string(e.what());
        return std::nullopt;
    } catch (const std::exception& e) {
        lastError = "JSON file error: " + std::string(e.what());
        return std::nullopt;
    }
}

bool JsonYamlUtil::writeJsonToFile(const json& data, const std::string& filePath) noexcept
{
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            lastError = "Cannot create file: " + filePath;
            return false;
        }
        
        file << data.dump(4); // Pretty print with 4 spaces
        return true;
    } catch (const std::exception& e) {
        lastError = "JSON write error: " + std::string(e.what());
        return false;
    }
}

std::string JsonYamlUtil::serializeJson(const json& data, bool pretty) noexcept
{
    try {
        return pretty ? data.dump(4) : data.dump();
    } catch (const std::exception& e) {
        lastError = "JSON serialization error: " + std::string(e.what());
        return "{}";
    }
}

// YAML Operations
std::optional<YAML::Node> JsonYamlUtil::parseYaml(const std::string& yamlStr) noexcept
{
    try {
        if (yamlStr.empty()) {
            lastError = "Empty YAML string";
            return std::nullopt;
        }
        return YAML::Load(yamlStr);
    } catch (const YAML::Exception& e) {
        lastError = "YAML parse error: " + std::string(e.what());
        return std::nullopt;
    } catch (const std::exception& e) {
        lastError = "YAML error: " + std::string(e.what());
        return std::nullopt;
    }
}

std::optional<YAML::Node> JsonYamlUtil::parseYamlFromFile(const std::string& filePath) noexcept
{
    try {
        return YAML::LoadFile(filePath);
    } catch (const YAML::Exception& e) {
        lastError = "YAML file parse error: " + std::string(e.what());
        return std::nullopt;
    } catch (const std::exception& e) {
        lastError = "YAML file error: " + std::string(e.what());
        return std::nullopt;
    }
}

bool JsonYamlUtil::writeYamlToFile(const YAML::Node& data, const std::string& filePath) noexcept
{
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            lastError = "Cannot create file: " + filePath;
            return false;
        }
        
        file << data;
        return true;
    } catch (const std::exception& e) {
        lastError = "YAML write error: " + std::string(e.what());
        return false;
    }
}

std::string JsonYamlUtil::serializeYaml(const YAML::Node& data) noexcept
{
    try {
        std::stringstream ss;
        ss << data;
        return ss.str();
    } catch (const std::exception& e) {
        lastError = "YAML serialization error: " + std::string(e.what());
        return "";
    }
}

// Conversion utilities
std::optional<JsonYamlUtil::json> JsonYamlUtil::yamlToJson(const YAML::Node& yamlNode) noexcept
{
    try {
        std::function<json(const YAML::Node&)> convertNode = [&](const YAML::Node& node) -> json {
            if (node.IsScalar()) {
                std::string value = node.as<std::string>();
                // Try to parse as different types
                if (value == "true" || value == "false") {
                    return node.as<bool>();
                }
                try {
                    if (value.find('.') != std::string::npos) {
                        return node.as<double>();
                    } else {
                        return node.as<int64_t>();
                    }
                } catch (...) {
                    return value;
                }
            } else if (node.IsSequence()) {
                json array = json::array();
                for (const auto& item : node) {
                    array.push_back(convertNode(item));
                }
                return array;
            } else if (node.IsMap()) {
                json object = json::object();
                for (const auto& pair : node) {
                    object[pair.first.as<std::string>()] = convertNode(pair.second);
                }
                return object;
            }
            return json(nullptr);
        };
        
        return convertNode(yamlNode);
    } catch (const std::exception& e) {
        lastError = "YAML to JSON conversion error: " + std::string(e.what());
        return std::nullopt;
    }
}

std::optional<YAML::Node> JsonYamlUtil::jsonToYaml(const json& jsonData) noexcept
{
    try {
        std::function<YAML::Node(const json&)> convertJson = [&](const json& j) -> YAML::Node {
            YAML::Node node;
            
            if (j.is_null()) {
                return node; // null node
            } else if (j.is_boolean()) {
                node = j.get<bool>();
            } else if (j.is_number_integer()) {
                node = j.get<int64_t>();
            } else if (j.is_number_float()) {
                node = j.get<double>();
            } else if (j.is_string()) {
                node = j.get<std::string>();
            } else if (j.is_array()) {
                for (const auto& item : j) {
                    node.push_back(convertJson(item));
                }
            } else if (j.is_object()) {
                for (const auto& [key, value] : j.items()) {
                    node[key] = convertJson(value);
                }
            }
            
            return node;
        };
        
        return convertJson(jsonData);
    } catch (const std::exception& e) {
        lastError = "JSON to YAML conversion error: " + std::string(e.what());
        return std::nullopt;
    }
}

// Validation utilities
bool JsonYamlUtil::isValidJson(const std::string& jsonStr) noexcept
{
    try {
        (void)json::parse(jsonStr);  // Explicitly cast to void to suppress [[nodiscard]] warning
        return true;
    } catch (...) {
        return false;
    }
}

bool JsonYamlUtil::isValidYaml(const std::string& yamlStr) noexcept
{
    try {
        (void)YAML::Load(yamlStr);  // Explicitly cast to void to suppress any potential [[nodiscard]] warning
        return true;
    } catch (...) {
        return false;
    }
}

// Response builders
JsonYamlUtil::json JsonYamlUtil::createSuccessResponse(const std::string& message) noexcept
{
    json response;
    response["success"] = true;
    if (!message.empty()) {
        response["message"] = message;
    }
    return response;
}

JsonYamlUtil::json JsonYamlUtil::createErrorResponse(const std::string& error) noexcept
{
    json response;
    response["success"] = false;
    response["error"] = error;
    return response;
}

JsonYamlUtil::json JsonYamlUtil::createDataResponse(const json& data, const std::string& message) noexcept
{
    json response;
    response["success"] = true;
    response["data"] = data;
    if (!message.empty()) {
        response["message"] = message;
    }
    return response;
}

// Safe extraction utilities
std::optional<std::string> JsonYamlUtil::safeGetString(const json& data, const std::string& key) noexcept
{
    try {
        if (data.contains(key) && data[key].is_string()) {
            return data[key].get<std::string>();
        }
    } catch (const std::exception&) {
        // Silent fail for type conversion errors
    }
    return std::nullopt;
}

std::optional<int> JsonYamlUtil::safeGetInt(const json& data, const std::string& key) noexcept
{
    try {
        if (data.contains(key) && data[key].is_number_integer()) {
            return data[key].get<int>();
        }
    } catch (const std::exception&) {
        // Silent fail for type conversion errors
    }
    return std::nullopt;
}

std::optional<bool> JsonYamlUtil::safeGetBool(const json& data, const std::string& key) noexcept
{
    try {
        if (data.contains(key) && data[key].is_boolean()) {
            return data[key].get<bool>();
        }
    } catch (const std::exception&) {
        // Silent fail for type conversion errors
    }
    return std::nullopt;
}

std::optional<JsonYamlUtil::json> JsonYamlUtil::safeGetObject(const json& data, const std::string& key) noexcept
{
    return data.contains(key) && data[key].is_object() ? std::optional<json>(data[key]) : std::nullopt;
}

std::string JsonYamlUtil::getLastError()
{
    return lastError;
}

} // namespace util
} // namespace shared