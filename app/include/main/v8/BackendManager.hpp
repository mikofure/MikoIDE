#pragma once

#include "include/cef_v8.h"
#include "include/cef_client.h"

namespace app
{
namespace v8
{

class BackendManager : public CefV8Handler
{
public:
    BackendManager();
    virtual ~BackendManager();

    // CefV8Handler methods
    virtual bool Execute(const CefString &name,
                        CefRefPtr<CefV8Value> object,
                        const CefV8ValueList &arguments,
                        CefRefPtr<CefV8Value> &retval,
                        CefString &exception) override;

    // Initialize the backend manager and register V8 extensions
    static void init();

private:
    // Menu action handlers
    bool handleMenuAction(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleOpenFolder(CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleNewFile(CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleOpenFile(CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleSaveFile(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleNewWindow(CefRefPtr<CefV8Value> &retval, CefString &exception);

    // File operation handlers
    bool handleFileOperation(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleReadFile(CefRefPtr<CefV8Value> params, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleWriteFile(CefRefPtr<CefV8Value> params, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleListDirectory(CefRefPtr<CefV8Value> params, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleFileExists(CefRefPtr<CefV8Value> params, CefRefPtr<CefV8Value> &retval, CefString &exception);

    // Window operation handlers
    bool handleWindowOperation(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception);

    // Application state handlers
    bool handleGetApplicationState(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception);
    bool handleSetApplicationState(const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception);

    IMPLEMENT_REFCOUNTING(BackendManager);
};

} // namespace v8
} // namespace app