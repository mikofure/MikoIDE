#include "main/binding/AppBinding.hpp"
#include "main/net/RequestClient.hpp"
#include "shared/AppConfig.hpp"
#include "shared/util/ResourceUtil.hpp"
#include "shared/util/JsonYamlUtil.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <ctime>

using json = nlohmann::json;
using JsonUtil = shared::util::JsonYamlUtil;

namespace app
{
namespace binding
{

bool AppBinding::OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t queryId, const CefString &request, bool persistent, CefRefPtr<Callback> callback)
{
    // only handle messages from application url
    const std::string &url = frame->GetURL();

    if (!(url.rfind(APP_ORIGIN, 0) == 0))
    {
        return false;
    }

    {
        const std::string &messageName = "App::ReverseData";
        const std::string &requestMessage = request;

        if (requestMessage.rfind(messageName, 0) == 0)
        {
            return onTaskReverseData(browser, frame, queryId, request, persistent, callback, messageName, requestMessage);
        }
    }

    {
        const std::string &messageName = "App::NetworkRequest";
        const std::string &requestMessage = request;

        if (requestMessage.rfind(messageName, 0) == 0)
        {
            return onTaskNetworkRequest(browser, frame, queryId, request, persistent, callback, messageName, requestMessage);
        }
    }

    {
        const std::string &messageName = "App::MenuAction";
        const std::string &requestMessage = request;

        if (requestMessage.rfind(messageName, 0) == 0)
        {
            return onTaskMenuAction(browser, frame, queryId, request, persistent, callback, messageName, requestMessage);
        }
    }

    {
        const std::string &messageName = "App::FileOperation";
        const std::string &requestMessage = request;

        if (requestMessage.rfind(messageName, 0) == 0)
        {
            return onTaskFileOperation(browser, frame, queryId, request, persistent, callback, messageName, requestMessage);
        }
    }

    {
        const std::string &messageName = "App::TaskManager";
        const std::string &requestMessage = request;

        if (requestMessage.rfind(messageName, 0) == 0)
        {
            return onTaskManagerOperation(browser, frame, queryId, request, persistent, callback, messageName, requestMessage);
        }
    }

    return false;
}

bool AppBinding::onTaskNetworkRequest(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t queryId, const CefString &request, bool persistent, CefRefPtr<Callback> callback, const std::string &messageName, const std::string &requestMessage)
{
    // Parse the JSON message using optimized utility
    std::string jsonStr = requestMessage.substr(messageName.size() + 1);
    auto dataOpt = JsonUtil::parseJson(jsonStr);
    
    if (!dataOpt) {
        auto errorResponse = JsonUtil::createErrorResponse("Failed to parse network request JSON");
        callback->Success(JsonUtil::serializeJson(errorResponse));
        return true;
    }
    
    auto urlOpt = JsonUtil::safeGetString(*dataOpt, "url");
    if (!urlOpt) {
        auto errorResponse = JsonUtil::createErrorResponse("Missing 'url' field in network request");
        callback->Success(JsonUtil::serializeJson(errorResponse));
        return true;
    }
    
    try {
        const std::string& url = *urlOpt;
        
        // Validate URL format (basic validation)
        if (url.empty() || (url.find("http://") != 0 && url.find("https://") != 0)) {
            auto errorResponse = JsonUtil::createErrorResponse("Invalid URL format: " + url);
            callback->Success(JsonUtil::serializeJson(errorResponse));
            return true;
        }
        
        // Get method with default value
        std::string method = JsonUtil::safeGetString(*dataOpt, "method").value_or("GET");
        
        // Validate HTTP method
        static const std::unordered_set<std::string> validMethods = {
            "GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS"
        };
        
        if (validMethods.find(method) == validMethods.end()) {
            auto errorResponse = JsonUtil::createErrorResponse("Invalid HTTP method: " + method);
            callback->Success(JsonUtil::serializeJson(errorResponse));
            return true;
        }
        
        // Get optional headers and body
        auto headersOpt = JsonUtil::safeGetObject(*dataOpt, "headers");
        auto bodyOpt = JsonUtil::safeGetString(*dataOpt, "body");
        auto timeoutOpt = JsonUtil::safeGetInt(*dataOpt, "timeout");
        
        // Create optimized response
        json response = JsonUtil::createSuccessResponse("Network request processed");
        response["url"] = url;
        response["method"] = method;
        response["statusCode"] = 200;
        response["data"] = "Sample response data for " + method + " " + url;
        response["timestamp"] = std::time(nullptr);
        
        if (headersOpt) {
            response["requestHeaders"] = *headersOpt;
        }
        
        if (bodyOpt) {
            response["requestBodyLength"] = bodyOpt->length();
        }
        
        if (timeoutOpt) {
            response["timeout"] = *timeoutOpt;
        }
        
        callback->Success(JsonUtil::serializeJson(response));
        return true;
    } catch (const std::exception& e) {
        auto errorResponse = JsonUtil::createErrorResponse("Failed to process network request: " + std::string(e.what()));
        callback->Success(JsonUtil::serializeJson(errorResponse));
        return true;
    }
}

bool AppBinding::onTaskReverseData(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t queryId, const CefString &request, bool persistent, CefRefPtr<Callback> callback, const std::string &messageName, const std::string &requestMessage)
{
    // Parse the JSON message using optimized utility
    std::string jsonStr = requestMessage.substr(messageName.size() + 1);
    auto dataOpt = JsonUtil::parseJson(jsonStr);
    
    if (!dataOpt) {
        auto errorResponse = JsonUtil::createErrorResponse("Failed to parse reverse data JSON");
        callback->Success(JsonUtil::serializeJson(errorResponse));
        return true;
    }
    
    auto inputDataOpt = JsonUtil::safeGetString(*dataOpt, "data");
    if (!inputDataOpt) {
        auto errorResponse = JsonUtil::createErrorResponse("Missing 'data' field in request");
        callback->Success(JsonUtil::serializeJson(errorResponse));
        return true;
    }
    
    try {
        // Extract and reverse the data
        std::string inputData = *inputDataOpt;
        std::reverse(inputData.begin(), inputData.end());
        
        // Create optimized response
        json response = JsonUtil::createSuccessResponse("Data reversed successfully");
        response["reversedData"] = inputData;
        response["originalLength"] = inputDataOpt->length();
        
        callback->Success(JsonUtil::serializeJson(response));
        return true;
    } catch (const std::exception& e) {
        auto errorResponse = JsonUtil::createErrorResponse("Failed to reverse data: " + std::string(e.what()));
        callback->Success(JsonUtil::serializeJson(errorResponse));
        return true;
    }
}

void AppBinding::onRequestComplete(CefRefPtr<Callback> callback, CefURLRequest::ErrorCode errorCode, const std::string &downloadData)
{
    CEF_REQUIRE_UI_THREAD();

    if (errorCode == ERR_NONE)
    {
        callback->Success(downloadData);
    }
    else
    {
        callback->Failure(errorCode, "Request failed!");
    }

    callback = nullptr;
}

bool AppBinding::onTaskMenuAction(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t queryId, const CefString &request, bool persistent, CefRefPtr<Callback> callback, const std::string &messageName, const std::string &requestMessage)
{
    // Parse the JSON message using optimized utility
    std::string jsonStr = requestMessage.substr(messageName.size() + 1);
    auto dataOpt = JsonUtil::parseJson(jsonStr);
    
    if (!dataOpt) {
        auto errorResponse = JsonUtil::createErrorResponse("Failed to parse menu action JSON");
        callback->Success(JsonUtil::serializeJson(errorResponse));
        return true;
    }
    
    auto actionOpt = JsonUtil::safeGetString(*dataOpt, "action");
    if (!actionOpt) {
        auto errorResponse = JsonUtil::createErrorResponse("Missing 'action' field in request");
        callback->Success(JsonUtil::serializeJson(errorResponse));
        return true;
    }
    
    const std::string& action = *actionOpt;
    
    // Use a static map for better performance than multiple if-else
    static const std::unordered_map<std::string, std::string> actionMessages = {
        {"file.new", "New file action triggered"},
        {"file.open", "Open file action triggered"},
        {"file.open_folder", "Open folder action triggered"}, // TODO: Implement native folder picker dialog
        {"file.save", "Save file action triggered"},
        {"file.save_as", "Save as action triggered"},
        {"git.clone", "Git clone action triggered"}, // TODO: Implement git clone functionality
        {"git.init", "Git init action triggered"},
        {"git.commit", "Git commit action triggered"},
        {"git.push", "Git push action triggered"},
        {"git.pull", "Git pull action triggered"}
    };
    
    json response;
    auto it = actionMessages.find(action);
    if (it != actionMessages.end()) {
        response = JsonUtil::createSuccessResponse(it->second);
    } else {
        response = JsonUtil::createErrorResponse("Unknown menu action: " + action);
    }
    
    callback->Success(JsonUtil::serializeJson(response));
    return true;
}

bool AppBinding::onTaskFileOperation(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t queryId, const CefString &request, bool persistent, CefRefPtr<Callback> callback, const std::string &messageName, const std::string &requestMessage)
{
    // Parse the JSON message using optimized utility
    std::string jsonStr = requestMessage.substr(messageName.size() + 1);
    auto dataOpt = JsonUtil::parseJson(jsonStr);
    
    if (!dataOpt) {
        auto errorResponse = JsonUtil::createErrorResponse("Failed to parse file operation JSON");
        callback->Success(JsonUtil::serializeJson(errorResponse));
        return true;
    }
    
    auto operationOpt = JsonUtil::safeGetString(*dataOpt, "operation");
    if (!operationOpt) {
        auto errorResponse = JsonUtil::createErrorResponse("Missing 'operation' field in request");
        callback->Success(JsonUtil::serializeJson(errorResponse));
        return true;
    }
    
    const std::string& operation = *operationOpt;
    json response;
    
    // Use function pointers for better organization and performance
    static const std::unordered_map<std::string, std::function<json(const json&)>> operations = {
        {"read_file", [](const json& data) -> json {
            auto pathOpt = JsonUtil::safeGetString(data, "path");
            if (!pathOpt) {
                return JsonUtil::createErrorResponse("Missing 'path' field for read_file operation");
            }
            
            const std::string& filePath = *pathOpt;
            std::ifstream file(filePath, std::ios::binary);
            
            if (!file.is_open()) {
                return JsonUtil::createErrorResponse("Failed to open file: " + filePath);
            }
            
            // More efficient file reading
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            file.seekg(0, std::ios::beg);
            
            std::string content(size, '\0');
            file.read(&content[0], size);
            
            if (file.bad()) {
                return JsonUtil::createErrorResponse("Error reading file: " + filePath);
            }
            
            json result = JsonUtil::createSuccessResponse("File read successfully");
            result["content"] = content;
            return result;
        }},
        
        {"write_file", [](const json& data) -> json {
            auto pathOpt = JsonUtil::safeGetString(data, "path");
            auto contentOpt = JsonUtil::safeGetString(data, "content");
            
            if (!pathOpt) {
                return JsonUtil::createErrorResponse("Missing 'path' field for write_file operation");
            }
            if (!contentOpt) {
                return JsonUtil::createErrorResponse("Missing 'content' field for write_file operation");
            }
            
            const std::string& filePath = *pathOpt;
            const std::string& content = *contentOpt;
            
            // Create directory if it doesn't exist
            std::filesystem::path path(filePath);
            if (path.has_parent_path()) {
                std::error_code ec;
                std::filesystem::create_directories(path.parent_path(), ec);
                if (ec) {
                    return JsonUtil::createErrorResponse("Failed to create directory: " + ec.message());
                }
            }
            
            std::ofstream file(filePath, std::ios::binary);
            if (!file.is_open()) {
                return JsonUtil::createErrorResponse("Failed to create file: " + filePath);
            }
            
            file.write(content.c_str(), content.size());
            if (file.bad()) {
                return JsonUtil::createErrorResponse("Error writing to file: " + filePath);
            }
            
            return JsonUtil::createSuccessResponse("File written successfully");
        }},
        
        {"file_exists", [](const json& data) -> json {
            auto pathOpt = JsonUtil::safeGetString(data, "path");
            if (!pathOpt) {
                return JsonUtil::createErrorResponse("Missing 'path' field for file_exists operation");
            }
            
            const std::string& filePath = *pathOpt;
            std::error_code ec;
            bool exists = std::filesystem::exists(filePath, ec);
            
            if (ec) {
                return JsonUtil::createErrorResponse("Error checking file existence: " + ec.message());
            }
            
            json result = JsonUtil::createSuccessResponse(exists ? "File exists" : "File does not exist");
            result["exists"] = exists;
            return result;
        }},
        
        {"list_directory", [](const json& data) -> json {
            auto pathOpt = JsonUtil::safeGetString(data, "path");
            if (!pathOpt) {
                return JsonUtil::createErrorResponse("Missing 'path' field for list_directory operation");
            }
            
            const std::string& dirPath = *pathOpt;
            json files = json::array();
            
            std::error_code ec;
            if (!std::filesystem::exists(dirPath, ec) || ec) {
                return JsonUtil::createErrorResponse("Directory does not exist: " + dirPath);
            }
            
            if (!std::filesystem::is_directory(dirPath, ec) || ec) {
                return JsonUtil::createErrorResponse("Path is not a directory: " + dirPath);
            }
            
            try {
                for (const auto& entry : std::filesystem::directory_iterator(dirPath, ec)) {
                    if (ec) break;
                    
                    json fileInfo;
                    fileInfo["name"] = entry.path().filename().string();
                    fileInfo["path"] = entry.path().string();
                    fileInfo["is_directory"] = entry.is_directory(ec);
                    
                    if (!ec && entry.is_regular_file(ec) && !ec) {
                        std::error_code size_ec;
                        auto size = std::filesystem::file_size(entry.path(), size_ec);
                        fileInfo["size"] = size_ec ? 0 : static_cast<int64_t>(size);
                    } else {
                        fileInfo["size"] = 0;
                    }
                    
                    files.push_back(fileInfo);
                }
                
                if (ec) {
                    return JsonUtil::createErrorResponse("Error listing directory: " + ec.message());
                }
                
                json result = JsonUtil::createSuccessResponse("Directory listed successfully");
                result["files"] = files;
                return result;
            } catch (const std::exception& e) {
                return JsonUtil::createErrorResponse("Exception listing directory: " + std::string(e.what()));
            }
        }}
    };
    
    auto it = operations.find(operation);
    if (it != operations.end()) {
        response = it->second(*dataOpt);
    } else {
        response = JsonUtil::createErrorResponse("Unknown file operation: " + operation);
    }
    
    callback->Success(JsonUtil::serializeJson(response));
    return true;
}

bool AppBinding::onTaskManagerOperation(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t queryId, const CefString &request, bool persistent, CefRefPtr<Callback> callback, const std::string &messageName, const std::string &requestMessage)
{
    // Parse the JSON message using optimized utility
    std::string jsonStr = requestMessage.substr(messageName.size() + 1);
    auto dataOpt = JsonUtil::parseJson(jsonStr);
    
    if (!dataOpt) {
        auto errorResponse = JsonUtil::createErrorResponse("Failed to parse task manager operation JSON");
        callback->Success(JsonUtil::serializeJson(errorResponse));
        return true;
    }
    
    auto operationOpt = JsonUtil::safeGetString(*dataOpt, "operation");
    if (!operationOpt) {
        auto errorResponse = JsonUtil::createErrorResponse("Missing 'operation' field in request");
        callback->Success(JsonUtil::serializeJson(errorResponse));
        return true;
    }
    
    const std::string& operation = *operationOpt;
    json response;
    
    // Use function map for better organization and performance
    static const std::unordered_map<std::string, std::function<json(const json&)>> operations = {
        {"get_processes", [](const json& data) -> json {
            // This would typically interface with the V8 TaskManager extension
            // For now, return a success response indicating the operation was received
            json result = JsonUtil::createSuccessResponse("Process list request received");
            result["data"] = json::array(); // Empty array for now
            return result;
        }},
        
        {"end_process", [](const json& data) -> json {
            auto processIdOpt = JsonUtil::safeGetInt(data, "processId");
            if (!processIdOpt) {
                return JsonUtil::createErrorResponse("Missing or invalid 'processId' field for end_process operation");
            }
            
            int processId = *processIdOpt;
            if (processId <= 0) {
                return JsonUtil::createErrorResponse("Invalid processId: " + std::to_string(processId));
            }
            
            // This would typically interface with the V8 TaskManager extension
            json result = JsonUtil::createSuccessResponse("Process termination request received");
            result["processId"] = processId;
            return result;
        }},
        
        {"get_system_stats", [](const json& data) -> json {
            // This would typically interface with the V8 TaskManager extension
            json result = JsonUtil::createSuccessResponse("System stats request received");
            result["data"] = {
                {"totalCpuUsage", 0.0},
                {"totalMemoryUsage", 0.0},
                {"totalNetworkUsage", 0.0}
            };
            return result;
        }}
    };
    
    auto it = operations.find(operation);
    if (it != operations.end()) {
        response = it->second(*dataOpt);
    } else {
        response = JsonUtil::createErrorResponse("Unknown task manager operation: " + operation);
    }
    
    callback->Success(JsonUtil::serializeJson(response));
    return true;
}

void AppBinding::init(CefRefPtr<CefMessageRouterBrowserSide> router)
{
    router->AddHandler(new AppBinding(), false);
}

} // namespace binding
} // namespace app
