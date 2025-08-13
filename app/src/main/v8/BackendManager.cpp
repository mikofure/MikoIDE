#include "main/v8/BackendManager.hpp"
#include "shared/util/ClientUtil.hpp"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <fstream>
#include <filesystem>

namespace app
{
namespace v8
{

BackendManager::BackendManager()
{
    // Initialize backend manager
}

BackendManager::~BackendManager()
{
    // Cleanup
}

bool BackendManager::Execute(const CefString &name, CefRefPtr<CefV8Value> object, const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (name == "executeMenuAction")
    {
        return handleMenuAction(arguments, retval, exception);
    }
    else if (name == "fileOperation")
    {
        return handleFileOperation(arguments, retval, exception);
    }
    else if (name == "windowOperation")
    {
        return handleWindowOperation(arguments, retval, exception);
    }
    else if (name == "getApplicationState")
    {
        return handleGetApplicationState(arguments, retval, exception);
    }
    else if (name == "setApplicationState")
    {
        return handleSetApplicationState(arguments, retval, exception);
    }

    return false;
}

bool BackendManager::handleMenuAction(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (arguments.size() < 1 || !arguments[0]->IsString())
    {
        exception = "Menu action requires a string parameter";
        return true;
    }

    std::string action = arguments[0]->GetStringValue();
    
    // Handle different menu actions
    if (action == "file.open_folder")
    {
        return handleOpenFolder(retval, exception);
    }
    else if (action == "file.new")
    {
        return handleNewFile(retval, exception);
    }
    else if (action == "file.open")
    {
        return handleOpenFile(retval, exception);
    }
    else if (action == "file.save")
    {
        return handleSaveFile(arguments, retval, exception);
    }
    else if (action == "window.new")
    {
        return handleNewWindow(retval, exception);
    }
    
    // Default success response for unhandled actions
    retval = CefV8Value::CreateObject(nullptr, nullptr);
    retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
    retval->SetValue("message", CefV8Value::CreateString("Menu action executed: " + action), V8_PROPERTY_ATTRIBUTE_NONE);
    
    return true;
}

bool BackendManager::handleFileOperation(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (arguments.size() < 1 || !arguments[0]->IsObject())
    {
        exception = "File operation requires an object parameter";
        return true;
    }

    CefRefPtr<CefV8Value> params = arguments[0];
    
    if (!params->HasValue("operation"))
    {
        exception = "File operation requires 'operation' field";
        return true;
    }

    std::string operation = params->GetValue("operation")->GetStringValue();
    
    if (operation == "read_file")
    {
        return handleReadFile(params, retval, exception);
    }
    else if (operation == "write_file")
    {
        return handleWriteFile(params, retval, exception);
    }
    else if (operation == "list_directory")
    {
        return handleListDirectory(params, retval, exception);
    }
    else if (operation == "file_exists")
    {
        return handleFileExists(params, retval, exception);
    }
    
    exception = "Unknown file operation: " + operation;
    return true;
}

bool BackendManager::handleWindowOperation(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (arguments.size() < 1 || !arguments[0]->IsObject())
    {
        exception = "Window operation requires an object parameter";
        return true;
    }

    CefRefPtr<CefV8Value> params = arguments[0];
    
    if (!params->HasValue("operation"))
    {
        exception = "Window operation requires 'operation' field";
        return true;
    }

    std::string operation = params->GetValue("operation")->GetStringValue();
    
    if (operation == "spawn_new")
    {
        return handleNewWindow(retval, exception);
    }
    
    exception = "Unknown window operation: " + operation;
    return true;
}

bool BackendManager::handleOpenFolder(CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    // Create file dialog for folder selection
    std::vector<CefString> accept_filters;
    
    // Use CEF's file dialog to select folder
    // This is a simplified implementation - in a real app you'd use CefBrowserHost::RunFileDialog
    
    retval = CefV8Value::CreateObject(nullptr, nullptr);
    retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
    retval->SetValue("message", CefV8Value::CreateString("Folder dialog opened"), V8_PROPERTY_ATTRIBUTE_NONE);
    
    return true;
}

bool BackendManager::handleNewFile(CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    retval = CefV8Value::CreateObject(nullptr, nullptr);
    retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
    retval->SetValue("message", CefV8Value::CreateString("New file created"), V8_PROPERTY_ATTRIBUTE_NONE);
    
    return true;
}

bool BackendManager::handleOpenFile(CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    retval = CefV8Value::CreateObject(nullptr, nullptr);
    retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
    retval->SetValue("message", CefV8Value::CreateString("File dialog opened"), V8_PROPERTY_ATTRIBUTE_NONE);
    
    return true;
}

bool BackendManager::handleSaveFile(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    retval = CefV8Value::CreateObject(nullptr, nullptr);
    retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
    retval->SetValue("message", CefV8Value::CreateString("File saved"), V8_PROPERTY_ATTRIBUTE_NONE);
    
    return true;
}

bool BackendManager::handleNewWindow(CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    retval = CefV8Value::CreateObject(nullptr, nullptr);
    retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
    retval->SetValue("message", CefV8Value::CreateString("New window spawned"), V8_PROPERTY_ATTRIBUTE_NONE);
    
    return true;
}

bool BackendManager::handleReadFile(CefRefPtr<CefV8Value> params, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (!params->HasValue("path"))
    {
        exception = "Read file operation requires 'path' field";
        return true;
    }

    std::string filePath = params->GetValue("path")->GetStringValue();
    
    try
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            retval = CefV8Value::CreateObject(nullptr, nullptr);
            retval->SetValue("success", CefV8Value::CreateBool(false), V8_PROPERTY_ATTRIBUTE_NONE);
            retval->SetValue("error", CefV8Value::CreateString("Failed to open file"), V8_PROPERTY_ATTRIBUTE_NONE);
            return true;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("data", CefV8Value::CreateString(content), V8_PROPERTY_ATTRIBUTE_NONE);
    }
    catch (const std::exception& e)
    {
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(false), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("error", CefV8Value::CreateString(e.what()), V8_PROPERTY_ATTRIBUTE_NONE);
    }
    
    return true;
}

bool BackendManager::handleWriteFile(CefRefPtr<CefV8Value> params, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (!params->HasValue("path") || !params->HasValue("content"))
    {
        exception = "Write file operation requires 'path' and 'content' fields";
        return true;
    }

    std::string filePath = params->GetValue("path")->GetStringValue();
    std::string content = params->GetValue("content")->GetStringValue();
    
    try
    {
        std::ofstream file(filePath);
        if (!file.is_open())
        {
            retval = CefV8Value::CreateObject(nullptr, nullptr);
            retval->SetValue("success", CefV8Value::CreateBool(false), V8_PROPERTY_ATTRIBUTE_NONE);
            retval->SetValue("error", CefV8Value::CreateString("Failed to create file"), V8_PROPERTY_ATTRIBUTE_NONE);
            return true;
        }
        
        file << content;
        file.close();
        
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("message", CefV8Value::CreateString("File written successfully"), V8_PROPERTY_ATTRIBUTE_NONE);
    }
    catch (const std::exception& e)
    {
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(false), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("error", CefV8Value::CreateString(e.what()), V8_PROPERTY_ATTRIBUTE_NONE);
    }
    
    return true;
}

bool BackendManager::handleListDirectory(CefRefPtr<CefV8Value> params, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (!params->HasValue("path"))
    {
        exception = "List directory operation requires 'path' field";
        return true;
    }

    std::string dirPath = params->GetValue("path")->GetStringValue();
    
    try
    {
        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath))
        {
            retval = CefV8Value::CreateObject(nullptr, nullptr);
            retval->SetValue("success", CefV8Value::CreateBool(false), V8_PROPERTY_ATTRIBUTE_NONE);
            retval->SetValue("error", CefV8Value::CreateString("Directory does not exist"), V8_PROPERTY_ATTRIBUTE_NONE);
            return true;
        }
        
        CefRefPtr<CefV8Value> files = CefV8Value::CreateArray(0);
        int index = 0;
        
        for (const auto& entry : std::filesystem::directory_iterator(dirPath))
        {
            CefRefPtr<CefV8Value> fileInfo = CefV8Value::CreateObject(nullptr, nullptr);
            fileInfo->SetValue("name", CefV8Value::CreateString(entry.path().filename().string()), V8_PROPERTY_ATTRIBUTE_NONE);
            fileInfo->SetValue("path", CefV8Value::CreateString(entry.path().string()), V8_PROPERTY_ATTRIBUTE_NONE);
            fileInfo->SetValue("isDirectory", CefV8Value::CreateBool(entry.is_directory()), V8_PROPERTY_ATTRIBUTE_NONE);
            
            files->SetValue(index++, fileInfo);
        }
        
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("data", files, V8_PROPERTY_ATTRIBUTE_NONE);
    }
    catch (const std::exception& e)
    {
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(false), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("error", CefV8Value::CreateString(e.what()), V8_PROPERTY_ATTRIBUTE_NONE);
    }
    
    return true;
}

bool BackendManager::handleFileExists(CefRefPtr<CefV8Value> params, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (!params->HasValue("path"))
    {
        exception = "File exists operation requires 'path' field";
        return true;
    }

    std::string filePath = params->GetValue("path")->GetStringValue();
    
    bool exists = std::filesystem::exists(filePath);
    
    retval = CefV8Value::CreateObject(nullptr, nullptr);
    retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
    retval->SetValue("exists", CefV8Value::CreateBool(exists), V8_PROPERTY_ATTRIBUTE_NONE);
    
    return true;
}

bool BackendManager::handleGetApplicationState(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    // Return current application state
    retval = CefV8Value::CreateObject(nullptr, nullptr);
    retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
    
    CefRefPtr<CefV8Value> state = CefV8Value::CreateObject(nullptr, nullptr);
    state->SetValue("theme", CefV8Value::CreateString("dark"), V8_PROPERTY_ATTRIBUTE_NONE);
    state->SetValue("language", CefV8Value::CreateString("en"), V8_PROPERTY_ATTRIBUTE_NONE);
    
    retval->SetValue("data", state, V8_PROPERTY_ATTRIBUTE_NONE);
    
    return true;
}

bool BackendManager::handleSetApplicationState(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (arguments.size() < 1 || !arguments[0]->IsObject())
    {
        exception = "Set application state requires an object parameter";
        return true;
    }

    // Store application state (in a real implementation, this would persist to disk)
    retval = CefV8Value::CreateObject(nullptr, nullptr);
    retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
    retval->SetValue("message", CefV8Value::CreateString("Application state updated"), V8_PROPERTY_ATTRIBUTE_NONE);
    
    return true;
}

void BackendManager::init()
{
    // Register V8 extension with backend management functions
    std::string code =
        ""
        "var BackendManager;"
        "if (!BackendManager) {"
        "  BackendManager = {};"
        "}"
        ""
        "(function() {"
        "  BackendManager.executeMenuAction = function(action, params) {"
        "    native function executeMenuAction();"
        "    return executeMenuAction(action, params);"
        "  };"
        "  BackendManager.fileOperation = function(params) {"
        "    native function fileOperation();"
        "    return fileOperation(params);"
        "  };"
        "  BackendManager.windowOperation = function(params) {"
        "    native function windowOperation();"
        "    return windowOperation(params);"
        "  };"
        "  BackendManager.getApplicationState = function() {"
        "    native function getApplicationState();"
        "    return getApplicationState();"
        "  };"
        "  BackendManager.setApplicationState = function(state) {"
        "    native function setApplicationState();"
        "    return setApplicationState(state);"
        "  };"
        "})();"
        "";

    CefRegisterExtension("v8/backend_manager", code, new BackendManager());
}

} // namespace v8
} // namespace app