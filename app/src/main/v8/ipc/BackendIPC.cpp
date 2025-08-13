#include "main/v8/ipc/BackendIPC.hpp"
#include "shared/util/ClientUtil.hpp"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <fstream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#elif __linux__
#include <gtk/gtk.h>
#elif __APPLE__
#include <Cocoa/Cocoa.h>
#endif

namespace app
{
namespace v8
{
namespace ipc
{

BackendIPC::BackendIPC()
{
    // Initialize IPC handler
}

BackendIPC::~BackendIPC()
{
    // Cleanup
}

bool BackendIPC::Execute(const CefString &name, CefRefPtr<CefV8Value> object, const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (name == "sendIPCMessage")
    {
        return handleIPCMessage(arguments, retval, exception);
    }
    else if (name == "openFolderDialog")
    {
        return handleOpenFolderDialog(arguments, retval, exception);
    }
    else if (name == "openFileDialog")
    {
        return handleOpenFileDialog(arguments, retval, exception);
    }
    else if (name == "saveFileDialog")
    {
        return handleSaveFileDialog(arguments, retval, exception);
    }
    else if (name == "showMessageBox")
    {
        return handleShowMessageBox(arguments, retval, exception);
    }

    return false;
}

bool BackendIPC::handleIPCMessage(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (arguments.size() < 1 || !arguments[0]->IsString())
    {
        exception = "IPC message requires a string parameter";
        return true;
    }

    std::string messageJson = arguments[0]->GetStringValue();
    
    // Parse the JSON message
    rapidjson::Document document;
    document.Parse(messageJson.c_str());
    
    if (document.HasParseError())
    {
        exception = "Invalid JSON message";
        return true;
    }
    
    // Extract message type and payload
    if (!document.HasMember("type") || !document["type"].IsString())
    {
        exception = "Message must have a 'type' field";
        return true;
    }
    
    std::string messageType = document["type"].GetString();
    
    // Handle different message types
    if (messageType == "menu_action")
    {
        return handleMenuActionIPC(document, retval, exception);
    }
    else if (messageType == "file_operation")
    {
        return handleFileOperationIPC(document, retval, exception);
    }
    else if (messageType == "window_control")
    {
        return handleWindowControlIPC(document, retval, exception);
    }
    else if (messageType == "app_state")
    {
        return handleAppStateIPC(document, retval, exception);
    }
    
    // Create success response for unknown message types
    retval = createSuccessResponse("Message processed: " + messageType);
    return true;
}

bool BackendIPC::handleMenuActionIPC(const rapidjson::Document &document, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (!document.HasMember("payload") || !document["payload"].IsObject())
    {
        exception = "Menu action message must have a 'payload' object";
        return true;
    }
    
    const rapidjson::Value &payload = document["payload"];
    
    if (!payload.HasMember("action") || !payload["action"].IsString())
    {
        exception = "Menu action payload must have an 'action' field";
        return true;
    }
    
    std::string action = payload["action"].GetString();
    
    // Handle specific menu actions
    if (action == "file.open_folder")
    {
        return handleOpenFolderAction(retval, exception);
    }
    else if (action == "file.open")
    {
        return handleOpenFileAction(retval, exception);
    }
    else if (action == "file.save" || action == "file.saveAs")
    {
        return handleSaveFileAction(action, retval, exception);
    }
    else if (action == "window.new")
    {
        return handleNewWindowAction(retval, exception);
    }
    
    retval = createSuccessResponse("Menu action executed: " + action);
    return true;
}

bool BackendIPC::handleFileOperationIPC(const rapidjson::Document &document, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (!document.HasMember("payload") || !document["payload"].IsObject())
    {
        exception = "File operation message must have a 'payload' object";
        return true;
    }
    
    const rapidjson::Value &payload = document["payload"];
    
    if (!payload.HasMember("operation") || !payload["operation"].IsString())
    {
        exception = "File operation payload must have an 'operation' field";
        return true;
    }
    
    std::string operation = payload["operation"].GetString();
    
    if (operation == "read_file")
    {
        return handleReadFileIPC(payload, retval, exception);
    }
    else if (operation == "write_file")
    {
        return handleWriteFileIPC(payload, retval, exception);
    }
    else if (operation == "list_directory")
    {
        return handleListDirectoryIPC(payload, retval, exception);
    }
    else if (operation == "file_exists")
    {
        return handleFileExistsIPC(payload, retval, exception);
    }
    
    retval = createErrorResponse("Unknown file operation: " + operation);
    return true;
}

bool BackendIPC::handleWindowControlIPC(const rapidjson::Document &document, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (!document.HasMember("payload") || !document["payload"].IsObject())
    {
        exception = "Window control message must have a 'payload' object";
        return true;
    }
    
    const rapidjson::Value &payload = document["payload"];
    
    if (!payload.HasMember("action") || !payload["action"].IsString())
    {
        exception = "Window control payload must have an 'action' field";
        return true;
    }
    
    std::string action = payload["action"].GetString();
    
    // Handle window control actions
    retval = createSuccessResponse("Window control executed: " + action);
    return true;
}

bool BackendIPC::handleAppStateIPC(const rapidjson::Document &document, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (!document.HasMember("payload") || !document["payload"].IsObject())
    {
        exception = "App state message must have a 'payload' object";
        return true;
    }
    
    const rapidjson::Value &payload = document["payload"];
    
    if (!payload.HasMember("action") || !payload["action"].IsString())
    {
        exception = "App state payload must have an 'action' field";
        return true;
    }
    
    std::string action = payload["action"].GetString();
    
    if (action == "get")
    {
        // Return current app state
        CefRefPtr<CefV8Value> state = CefV8Value::CreateObject(nullptr, nullptr);
        state->SetValue("theme", CefV8Value::CreateString("dark"), V8_PROPERTY_ATTRIBUTE_NONE);
        state->SetValue("language", CefV8Value::CreateString("en"), V8_PROPERTY_ATTRIBUTE_NONE);
        
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("data", state, V8_PROPERTY_ATTRIBUTE_NONE);
    }
    else if (action == "set")
    {
        // Set app state
        retval = createSuccessResponse("App state updated");
    }
    else
    {
        retval = createErrorResponse("Unknown app state action: " + action);
    }
    
    return true;
}

bool BackendIPC::handleOpenFolderDialog(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    std::string selectedPath = showFolderDialog();
    
    if (!selectedPath.empty())
    {
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("path", CefV8Value::CreateString(selectedPath), V8_PROPERTY_ATTRIBUTE_NONE);
    }
    else
    {
        retval = createErrorResponse("No folder selected");
    }
    
    return true;
}

bool BackendIPC::handleOpenFileDialog(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    std::string selectedPath = showOpenFileDialog();
    
    if (!selectedPath.empty())
    {
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("path", CefV8Value::CreateString(selectedPath), V8_PROPERTY_ATTRIBUTE_NONE);
    }
    else
    {
        retval = createErrorResponse("No file selected");
    }
    
    return true;
}

bool BackendIPC::handleSaveFileDialog(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    std::string selectedPath = showSaveFileDialog();
    
    if (!selectedPath.empty())
    {
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("path", CefV8Value::CreateString(selectedPath), V8_PROPERTY_ATTRIBUTE_NONE);
    }
    else
    {
        retval = createErrorResponse("No file selected");
    }
    
    return true;
}

bool BackendIPC::handleShowMessageBox(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (arguments.size() < 2 || !arguments[0]->IsString() || !arguments[1]->IsString())
    {
        exception = "Message box requires title and message parameters";
        return true;
    }

    std::string title = arguments[0]->GetStringValue();
    std::string message = arguments[1]->GetStringValue();
    
    showMessageBox(title, message);
    
    retval = createSuccessResponse("Message box shown");
    return true;
}

// Platform-specific dialog implementations
std::string BackendIPC::showFolderDialog()
{
#ifdef _WIN32
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = L"Select Folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    
    if (pidl != nullptr)
    {
        wchar_t path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path))
        {
            CoTaskMemFree(pidl);
            // Convert wide string to string
            std::wstring ws(path);
            return std::string(ws.begin(), ws.end());
        }
        CoTaskMemFree(pidl);
    }
#elif __linux__
    // GTK implementation would go here
    // For now, return empty string
#elif __APPLE__
    // Cocoa implementation would go here
    // For now, return empty string
#endif
    return "";
}

std::string BackendIPC::showOpenFileDialog()
{
#ifdef _WIN32
    OPENFILENAME ofn;
    wchar_t szFile[260] = { 0 };
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All Files\0*.*\0Text Files\0*.txt\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn))
    {
        std::wstring ws(szFile);
        return std::string(ws.begin(), ws.end());
    }
#elif __linux__
    // GTK implementation would go here
#elif __APPLE__
    // Cocoa implementation would go here
#endif
    return "";
}

std::string BackendIPC::showSaveFileDialog()
{
#ifdef _WIN32
    OPENFILENAME ofn;
    wchar_t szFile[260] = { 0 };
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All Files\0*.*\0Text Files\0*.txt\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileName(&ofn))
    {
        std::wstring ws(szFile);
        return std::string(ws.begin(), ws.end());
    }
#elif __linux__
    // GTK implementation would go here
#elif __APPLE__
    // Cocoa implementation would go here
#endif
    return "";
}

void BackendIPC::showMessageBox(const std::string &title, const std::string &message)
{
#ifdef _WIN32
    std::wstring wtitle(title.begin(), title.end());
    std::wstring wmessage(message.begin(), message.end());
    MessageBox(NULL, wmessage.c_str(), wtitle.c_str(), MB_OK);
#elif __linux__
    // GTK implementation would go here
#elif __APPLE__
    // Cocoa implementation would go here
#endif
}

// Helper methods for file operations
bool BackendIPC::handleOpenFolderAction(CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    std::string selectedPath = showFolderDialog();
    
    if (!selectedPath.empty())
    {
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("data", CefV8Value::CreateString(selectedPath), V8_PROPERTY_ATTRIBUTE_NONE);
    }
    else
    {
        retval = createErrorResponse("No folder selected");
    }
    
    return true;
}

bool BackendIPC::handleOpenFileAction(CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    std::string selectedPath = showOpenFileDialog();
    
    if (!selectedPath.empty())
    {
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("data", CefV8Value::CreateString(selectedPath), V8_PROPERTY_ATTRIBUTE_NONE);
    }
    else
    {
        retval = createErrorResponse("No file selected");
    }
    
    return true;
}

bool BackendIPC::handleSaveFileAction(const std::string &action, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    std::string selectedPath = showSaveFileDialog();
    
    if (!selectedPath.empty())
    {
        retval = CefV8Value::CreateObject(nullptr, nullptr);
        retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
        retval->SetValue("data", CefV8Value::CreateString(selectedPath), V8_PROPERTY_ATTRIBUTE_NONE);
    }
    else
    {
        retval = createErrorResponse("No file selected");
    }
    
    return true;
}

bool BackendIPC::handleNewWindowAction(CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    // In a real implementation, this would spawn a new window
    retval = createSuccessResponse("New window spawned");
    return true;
}

bool BackendIPC::handleReadFileIPC(const rapidjson::Value &payload, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (!payload.HasMember("path") || !payload["path"].IsString())
    {
        retval = createErrorResponse("Read file operation requires 'path' field");
        return true;
    }

    std::string filePath = payload["path"].GetString();
    
    try
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            retval = createErrorResponse("Failed to open file");
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
        retval = createErrorResponse(e.what());
    }
    
    return true;
}

bool BackendIPC::handleWriteFileIPC(const rapidjson::Value &payload, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (!payload.HasMember("path") || !payload["path"].IsString() ||
        !payload.HasMember("content") || !payload["content"].IsString())
    {
        retval = createErrorResponse("Write file operation requires 'path' and 'content' fields");
        return true;
    }

    std::string filePath = payload["path"].GetString();
    std::string content = payload["content"].GetString();
    
    try
    {
        std::ofstream file(filePath);
        if (!file.is_open())
        {
            retval = createErrorResponse("Failed to create file");
            return true;
        }
        
        file << content;
        file.close();
        
        retval = createSuccessResponse("File written successfully");
    }
    catch (const std::exception& e)
    {
        retval = createErrorResponse(e.what());
    }
    
    return true;
}

bool BackendIPC::handleListDirectoryIPC(const rapidjson::Value &payload, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (!payload.HasMember("path") || !payload["path"].IsString())
    {
        retval = createErrorResponse("List directory operation requires 'path' field");
        return true;
    }

    std::string dirPath = payload["path"].GetString();
    
    try
    {
        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath))
        {
            retval = createErrorResponse("Directory does not exist");
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
        retval = createErrorResponse(e.what());
    }
    
    return true;
}

bool BackendIPC::handleFileExistsIPC(const rapidjson::Value &payload, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (!payload.HasMember("path") || !payload["path"].IsString())
    {
        retval = createErrorResponse("File exists operation requires 'path' field");
        return true;
    }

    std::string filePath = payload["path"].GetString();
    
    bool exists = std::filesystem::exists(filePath);
    
    retval = CefV8Value::CreateObject(nullptr, nullptr);
    retval->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
    retval->SetValue("exists", CefV8Value::CreateBool(exists), V8_PROPERTY_ATTRIBUTE_NONE);
    
    return true;
}

CefRefPtr<CefV8Value> BackendIPC::createSuccessResponse(const std::string &message)
{
    CefRefPtr<CefV8Value> response = CefV8Value::CreateObject(nullptr, nullptr);
    response->SetValue("success", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_NONE);
    response->SetValue("message", CefV8Value::CreateString(message), V8_PROPERTY_ATTRIBUTE_NONE);
    return response;
}

CefRefPtr<CefV8Value> BackendIPC::createErrorResponse(const std::string &error)
{
    CefRefPtr<CefV8Value> response = CefV8Value::CreateObject(nullptr, nullptr);
    response->SetValue("success", CefV8Value::CreateBool(false), V8_PROPERTY_ATTRIBUTE_NONE);
    response->SetValue("error", CefV8Value::CreateString(error), V8_PROPERTY_ATTRIBUTE_NONE);
    return response;
}

void BackendIPC::init()
{
    std::string code =
        ""
        "var BackendIPC;"
        "if (!BackendIPC) {"
        "  BackendIPC = {};"
        "}"
        ""
        "(function() {"
        "  BackendIPC.sendMessage = function(type, payload) {"
        "    native function sendMessage();"
        "    return sendMessage(type, payload);"
        "  };"
        "  BackendIPC.receiveMessage = function() {"
        "    native function receiveMessage();"
        "    return receiveMessage();"
        "  };"
        "  BackendIPC.isConnected = function() {"
        "    native function isConnected();"
        "    return isConnected();"
        "  };"
        "})();"
        "";

    CefRegisterExtension("v8/backend_ipc", code, new BackendIPC());
}

} // namespace ipc
} // namespace v8
} // namespace app