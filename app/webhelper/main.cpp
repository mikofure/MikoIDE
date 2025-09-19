#include <windows.h>

// Undefine conflicting Windows macros
#undef min
#undef max
#undef GetMessage

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/base/cef_bind.h"

#include "../client/app.hpp"
#include "../utils/logger.hpp"

// WebHelper subprocess entry point
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // Initialize logger for subprocess
    Logger::Initialize();
    Logger::LogMessage("MikoWebHelper subprocess starting...");

    // CEF main args for subprocess
    CefMainArgs main_args(hInstance);
    
    // Create CEF app instance
    CefRefPtr<SimpleApp> app(new SimpleApp);

    // Execute the subprocess - this handles renderer, GPU, utility processes
    int exit_code = CefExecuteProcess(main_args, app, nullptr);
    
    Logger::LogMessage("MikoWebHelper subprocess exiting with code: " + std::to_string(exit_code));
    Logger::Shutdown();
    
    return exit_code;
}