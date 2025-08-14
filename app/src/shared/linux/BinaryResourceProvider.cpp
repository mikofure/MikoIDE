#include "shared/util/BinaryResourceProvider.hpp"
#include "shared/AppConfig.hpp"
#include "shared/MikoApplication.hpp"
#include "include/base/cef_logging.h"
#include "include/wrapper/cef_helpers.h"

namespace shared
{

namespace util
{

bool BinaryResourceProvider::OnRequest(scoped_refptr<CefResourceManager::Request> request)
{
    CEF_REQUIRE_IO_THREAD();

    const std::string &url = request->url();

    if (url.find(rootURL) != 0L)
    {
        // not handled by this provider
        return false;
    }

    CefRefPtr<CefResourceHandler> handler;
    const std::string &relativePath = url.substr(rootURL.length());

    if (!relativePath.empty())
    {
        auto& manager = miko::BinaryResourceManager::getInstance();
        
        if (manager.hasResource(relativePath))
        {
            CefRefPtr<CefStreamReader> stream = manager.getResourceReader(relativePath);
            
            if (stream.get())
            {
                std::string mimeType = manager.getResourceMimeType(relativePath);
                handler = new CefStreamResourceHandler(mimeType, stream);
            }
        }
    }

    request->Continue(handler);
    return true;
}

} // namespace util
} // namespace shared