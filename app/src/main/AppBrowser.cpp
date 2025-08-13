#include "main/AppBrowser.hpp"
#include "shared/AppConfig.hpp"
#include "shared/util/BrowserUtil.hpp"

namespace app
{

CefRefPtr<BrowserApp> BrowserApp::instance = nullptr;

CefRefPtr<BrowserApp> BrowserApp::sharedHandler()
{
    if (instance == nullptr)
    {
        instance = new BrowserApp();
    }

    return instance;
}

CefRefPtr<CefBrowserProcessHandler> BrowserApp::GetBrowserProcessHandler()
{
    return this;
}

void BrowserApp::OnBeforeCommandLineProcessing(const CefString &processType, CefRefPtr<CefCommandLine> commandLine)
{
    // CommandLine flags can be modified in this callback.
    // ProcessType is empty for the browser process.
    if (processType.empty())
    {
        // Fast startup and renderer preloading switches (reduced for window visibility)
        // commandLine->AppendSwitch("disable-extensions");
        // commandLine->AppendSwitch("disable-pdf-extension");
        
#if defined(OS_MACOSX)
        // Disable the macOS keychain prompt.
        // Cookies will not be encrypted.
        commandLine->AppendSwitch("use-mock-keychain");
#endif
    }
}

void BrowserApp::OnContextInitialized()
{
    // create the browser window
    clientInstance = new AppClient();
    
    // Configure browser settings for fast startup and background color
    CefBrowserSettings browserSettings;
    browserSettings.background_color = 0xFF0e0e0e; // #0e0e0e background color
    browserSettings.javascript_close_windows = STATE_DISABLED;
    browserSettings.javascript_access_clipboard = STATE_DISABLED;
    browserSettings.javascript_dom_paste = STATE_DISABLED;
    
    shared::util::BrowserUtil::create(getAppClient(), APP_CONFIG_START_URL, browserSettings);
}

CefRefPtr<AppClient> BrowserApp::getAppClient()
{
    if (!clientInstance.get())
    {
        return nullptr;
    }

    return clientInstance;
}

} // namespace app
