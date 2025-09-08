#include "app.hpp"

// SimpleRenderProcessHandler implementation
SimpleRenderProcessHandler::SimpleRenderProcessHandler() {
    // Create the renderer-side router for query handling
    CefMessageRouterConfig config;
    message_router_ = CefMessageRouterRendererSide::Create(config);
}

void SimpleRenderProcessHandler::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                                 CefRefPtr<CefFrame> frame,
                                                 CefRefPtr<CefV8Context> context) {
    // Register JavaScript functions with the new context
    message_router_->OnContextCreated(browser, frame, context);
}

void SimpleRenderProcessHandler::OnContextReleased(CefRefPtr<CefBrowser> browser,
                                                   CefRefPtr<CefFrame> frame,
                                                   CefRefPtr<CefV8Context> context) {
    // Clean up context
    message_router_->OnContextReleased(browser, frame, context);
}

bool SimpleRenderProcessHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                                         CefRefPtr<CefFrame> frame,
                                                         CefProcessId source_process,
                                                         CefRefPtr<CefProcessMessage> message) {
    // Handle process messages
    return message_router_->OnProcessMessageReceived(browser, frame, source_process, message);
}

// SimpleApp implementation
SimpleApp::SimpleApp() {
    render_process_handler_ = new SimpleRenderProcessHandler();
}

void SimpleApp::OnBeforeCommandLineProcessing(const CefString& process_type,
                                            CefRefPtr<CefCommandLine> command_line) {
    // Enable Window Controls Overlay feature for PWA-like window controls
    // This allows web content to extend into the title bar area
    command_line->AppendSwitchWithValue("enable-features", 
        "WindowControlsOverlay,WebAppWindowControlsOverlay");
        
    // Enable experimental web platform features that may be needed for WCO
    command_line->AppendSwitch("enable-experimental-web-platform-features");
    
    // Enable blink features for better WCO support
    command_line->AppendSwitchWithValue("enable-blink-features", 
        "WindowControlsOverlay,CSSEnvironmentVariables");
}

void SimpleApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) {
    // Register the miko:// custom scheme
    registrar->AddCustomScheme("miko", 
        CEF_SCHEME_OPTION_STANDARD | 
        CEF_SCHEME_OPTION_LOCAL | 
        CEF_SCHEME_OPTION_CORS_ENABLED |
        CEF_SCHEME_OPTION_SECURE);
}