#pragma once
#include "include/cef_app.h"
#include "include/cef_render_process_handler.h"
#include "include/wrapper/cef_message_router.h"

// Render process handler for message router
class SimpleRenderProcessHandler : public CefRenderProcessHandler {
public:
    SimpleRenderProcessHandler();

    // CefRenderProcessHandler methods
    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) override;
    
    void OnContextReleased(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override;
    
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefProcessId source_process,
                                 CefRefPtr<CefProcessMessage> message) override;

private:
    CefRefPtr<CefMessageRouterRendererSide> message_router_;
    IMPLEMENT_REFCOUNTING(SimpleRenderProcessHandler);
};

class SimpleApp : public CefApp {
public:
    SimpleApp();
    
    // CefApp methods
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return render_process_handler_;
    }
    
    // Override to add command line switches for Window Controls Overlay
    void OnBeforeCommandLineProcessing(const CefString& process_type,
                                     CefRefPtr<CefCommandLine> command_line) override;
    
    // Override to register custom schemes
    void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override;

private:
    CefRefPtr<SimpleRenderProcessHandler> render_process_handler_;
    IMPLEMENT_REFCOUNTING(SimpleApp);
};