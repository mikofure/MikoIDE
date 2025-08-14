#pragma once

#include "include/cef_v8.h"
#include "include/cef_client.h"
#include "shared/gitcontrol/GitIPCHandler.hpp"
#include <rapidjson/document.h>
#include <string>
#include <memory>

namespace app
{
namespace v8
{
namespace ipc
{

class BackendIPC : public CefV8Handler
{
public:
    BackendIPC();
    virtual ~BackendIPC();

    // CefV8Handler methods
    virtual bool Execute(const CefString &name,
                        CefRefPtr<CefV8Value> object,
                        const CefV8ValueList &arguments,
                        CefRefPtr<CefV8Value> &retval,
                        CefString &exception) override;

    // Initialize the IPC handler and register V8 extensions
    static void init();

private:
    // Main IPC message handler
    bool handleIPCMessage(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception);

    // Message type handlers
    bool handleMenuActionIPC(const rapidjson::Document &document, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleFileOperationIPC(const rapidjson::Document &document, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleWindowControlIPC(const rapidjson::Document &document, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleAppStateIPC(const rapidjson::Document &document, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleGitOperationIPC(const rapidjson::Document &document, CefRefPtr<CefV8Value> &retval, CefString &exception);

    // Dialog handlers
    bool handleOpenFolderDialog(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleOpenFileDialog(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleSaveFileDialog(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleShowMessageBox(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception);

    // Platform-specific dialog implementations
    std::string showFolderDialog();
    std::string showOpenFileDialog();
    std::string showSaveFileDialog();
    void showMessageBox(const std::string &title, const std::string &message);

    // Menu action helpers
    bool handleOpenFolderAction(CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleOpenFileAction(CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleSaveFileAction(const std::string &action, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleNewWindowAction(CefRefPtr<CefV8Value> &retval, CefString &exception);

    // File operation helpers
    bool handleReadFileIPC(const rapidjson::Value &payload, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleWriteFileIPC(const rapidjson::Value &payload, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleListDirectoryIPC(const rapidjson::Value &payload, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleFileExistsIPC(const rapidjson::Value &payload, CefRefPtr<CefV8Value> &retval, CefString &exception);

    // Git operation helpers
    CefRefPtr<CefV8Value> convertGitResponseToV8(const miko::gitcontrol::GitIPCResponse &response);
    std::map<std::string, std::any> convertV8ToParamsMap(const rapidjson::Value &payload);
    
    // Response helpers
    CefRefPtr<CefV8Value> createSuccessResponse(const std::string &message);
    CefRefPtr<CefV8Value> createErrorResponse(const std::string &error);
    
    // Git IPC handler
    std::unique_ptr<miko::gitcontrol::GitIPCHandler> gitHandler;

    IMPLEMENT_REFCOUNTING(BackendIPC);
};

} // namespace ipc
} // namespace v8
} // namespace app