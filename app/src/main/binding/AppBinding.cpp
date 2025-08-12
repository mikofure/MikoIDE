#include "main/binding/AppBinding.hpp"
#include "main/net/RequestClient.hpp"
#include "shared/AppConfig.hpp"
#include "shared/util/ResourceUtil.hpp"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <fstream>

using json = nlohmann::json;

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
    // create a CefRequest object
    CefRefPtr<CefRequest> httpRequest = CefRequest::Create();

    // set the request URL
    httpRequest->SetURL(requestMessage.substr(messageName.size() + 1));

    // set the request method (supported methods include GET, POST, HEAD, DELETE and PUT)
    httpRequest->SetMethod("POST");

    // optionally specify custom headers
    CefRequest::HeaderMap headerMap;
    headerMap.insert(std::make_pair("X-My-Header", "My Header Value"));
    httpRequest->SetHeaderMap(headerMap);

    // 1. optionally specify upload content
    // 2. the default "Content-Type" header value is "application/x-www-form-urlencoded"
    // 3. set "Content-Type" via the HeaderMap if a different value is desired
    const std::string &uploadData = "arg1=val1&arg2=val2";

    CefRefPtr<CefPostData> postData = CefPostData::Create();
    CefRefPtr<CefPostDataElement> element = CefPostDataElement::Create();

    element->SetToBytes(uploadData.size(), uploadData.c_str());
    postData->AddElement(element);
    httpRequest->SetPostData(postData);

    // optionally set flags
    httpRequest->SetFlags(UR_FLAG_SKIP_CACHE);

    // create client and load
    CefRefPtr<CefURLRequest> urlRequest = frame->CreateURLRequest(httpRequest, new app::net::RequestClient(callback, base::BindOnce(&AppBinding::onRequestComplete, base::Unretained(this))));

    return true;
}

bool AppBinding::onTaskReverseData(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t queryId, const CefString &request, bool persistent, CefRefPtr<Callback> callback, const std::string &messageName, const std::string &requestMessage)
{
    std::string result = requestMessage.substr(messageName.size() + 1);
    std::reverse(result.begin(), result.end());
    callback->Success(result);

    return true;
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
    try
    {
        // Parse the JSON message
        std::string jsonStr = requestMessage.substr(messageName.size() + 1);
        json data = json::parse(jsonStr);
        std::string action = data["action"];
        
        json response;
        response["success"] = true;
        
        if (action == "file.new")
        {
            response["message"] = "New file action triggered";
        }
        else if (action == "file.open")
        {
            response["message"] = "Open file action triggered";
        }
        else if (action == "file.open_folder")
        {
            response["message"] = "Open folder action triggered";
            // TODO: Implement native folder picker dialog
        }
        else if (action == "file.save")
        {
            response["message"] = "Save file action triggered";
        }
        else if (action == "file.save_as")
        {
            response["message"] = "Save as action triggered";
        }
        else if (action == "git.clone")
        {
            response["message"] = "Git clone action triggered";
            // TODO: Implement git clone functionality
        }
        else if (action == "git.init")
        {
            response["message"] = "Git init action triggered";
        }
        else if (action == "git.commit")
        {
            response["message"] = "Git commit action triggered";
        }
        else if (action == "git.push")
        {
            response["message"] = "Git push action triggered";
        }
        else if (action == "git.pull")
        {
            response["message"] = "Git pull action triggered";
        }
        else
        {
            response["success"] = false;
            response["error"] = "Unknown menu action: " + action;
        }
        
        callback->Success(response.dump());
        return true;
    }
    catch (const std::exception &e)
    {
        json errorResponse;
        errorResponse["success"] = false;
        errorResponse["error"] = "Failed to parse menu action: " + std::string(e.what());
        callback->Success(errorResponse.dump());
        return true;
    }
}

bool AppBinding::onTaskFileOperation(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t queryId, const CefString &request, bool persistent, CefRefPtr<Callback> callback, const std::string &messageName, const std::string &requestMessage)
{
    try
    {
        // Parse the JSON message
        std::string jsonStr = requestMessage.substr(messageName.size() + 1);
        json data = json::parse(jsonStr);
        std::string operation = data["operation"];
        
        json response;
        response["success"] = true;
        
        if (operation == "read_file")
        {
            std::string filePath = data["path"];
            std::ifstream file(filePath);
            
            if (file.is_open())
            {
                std::string content((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
                file.close();
                response["content"] = content;
                response["message"] = "File read successfully";
            }
            else
            {
                response["success"] = false;
                response["error"] = "Failed to read file: " + filePath;
            }
        }
        else if (operation == "write_file")
        {
            std::string filePath = data["path"];
            std::string content = data["content"];
            
            std::ofstream file(filePath);
            if (file.is_open())
            {
                file << content;
                file.close();
                response["message"] = "File written successfully";
            }
            else
            {
                response["success"] = false;
                response["error"] = "Failed to write file: " + filePath;
            }
        }
        else if (operation == "file_exists")
        {
            std::string filePath = data["path"];
            bool exists = std::filesystem::exists(filePath);
            response["exists"] = exists;
            response["message"] = exists ? "File exists" : "File does not exist";
        }
        else if (operation == "list_directory")
        {
            std::string dirPath = data["path"];
            json files = json::array();
            
            try
            {
                for (const auto &entry : std::filesystem::directory_iterator(dirPath))
                {
                    json fileInfo;
                    fileInfo["name"] = entry.path().filename().string();
                    fileInfo["path"] = entry.path().string();
                    fileInfo["is_directory"] = entry.is_directory();
                    fileInfo["size"] = entry.is_regular_file() ? entry.file_size() : 0;
                    files.push_back(fileInfo);
                }
                response["files"] = files;
                response["message"] = "Directory listed successfully";
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                response["success"] = false;
                response["error"] = "Failed to list directory: " + std::string(e.what());
            }
        }
        else
        {
            response["success"] = false;
            response["error"] = "Unknown file operation: " + operation;
        }
        
        callback->Success(response.dump());
        return true;
    }
    catch (const std::exception &e)
    {
        json errorResponse;
        errorResponse["success"] = false;
        errorResponse["error"] = "Failed to parse file operation: " + std::string(e.what());
        callback->Success(errorResponse.dump());
        return true;
    }
}

bool AppBinding::onTaskManagerOperation(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t queryId, const CefString &request, bool persistent, CefRefPtr<Callback> callback, const std::string &messageName, const std::string &requestMessage)
{
    try
    {
        // Parse the JSON message
        std::string jsonStr = requestMessage.substr(messageName.size() + 1);
        json data = json::parse(jsonStr);
        std::string operation = data["operation"];
        
        json response;
        response["success"] = true;
        
        if (operation == "get_processes")
        {
            // This would typically interface with the V8 TaskManager extension
            // For now, return a success response indicating the operation was received
            response["message"] = "Process list request received";
            response["data"] = json::array(); // Empty array for now
        }
        else if (operation == "end_process")
        {
            int processId = data["processId"];
            // This would typically interface with the V8 TaskManager extension
            response["message"] = "Process termination request received";
            response["processId"] = processId;
        }
        else if (operation == "get_system_stats")
        {
            // This would typically interface with the V8 TaskManager extension
            response["message"] = "System stats request received";
            response["data"] = {
                {"totalCpuUsage", 0.0},
                {"totalMemoryUsage", 0.0},
                {"totalNetworkUsage", 0.0}
            };
        }
        else
        {
            response["success"] = false;
            response["error"] = "Unknown task manager operation: " + operation;
        }
        
        callback->Success(response.dump());
        return true;
    }
    catch (const std::exception &e)
    {
        json errorResponse;
        errorResponse["success"] = false;
        errorResponse["error"] = "Failed to parse task manager operation: " + std::string(e.what());
        callback->Success(errorResponse.dump());
        return true;
    }
}

void AppBinding::init(CefRefPtr<CefMessageRouterBrowserSide> router)
{
    router->AddHandler(new AppBinding(), false);
}

} // namespace binding
} // namespace app
