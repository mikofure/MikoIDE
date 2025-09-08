#pragma once
#include "include/cef_resource_handler.h"
#include "include/cef_scheme.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_request.h"
#include "include/wrapper/cef_helpers.h"
#include "include/base/cef_ref_counted.h"
#include <string>
#include <vector>

class BinaryResourceProvider : public CefSchemeHandlerFactory {
public:
    BinaryResourceProvider();
    
    // CefSchemeHandlerFactory method
    CefRefPtr<CefResourceHandler> Create(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const CefString& scheme_name,
        CefRefPtr<CefRequest> request) override;
        
private:
    IMPLEMENT_REFCOUNTING(BinaryResourceProvider);
    DISALLOW_COPY_AND_ASSIGN(BinaryResourceProvider);
};