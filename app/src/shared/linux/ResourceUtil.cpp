#include "shared/util/ResourceUtil.hpp"
#include "shared/AppConfig.hpp"
#include "shared/util/BinaryResourceProvider.hpp"

#include <libgen.h>
#include <linux/limits.h>
#include <string>
#include <unistd.h>

namespace shared
{

namespace util
{

bool ResourceUtil::getResourceDir(std::string &dir)
{
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    const char *path = nullptr;

    if (count != -1)
    {
        path = dirname(result);
    }
    else
    {
        // Fallback to current directory if readlink fails
        path = ".";
    }

    dir = std::string(path) + "/" + ASSETS_PATH;

    return true;
}

CefResourceManager::Provider *ResourceUtil::createBinaryResourceProvider(const std::string &rootURL)
{
    // only URLs beginning with root URL will be handled by this provider
    return new BinaryResourceProvider(rootURL);
}

} // namespace util
} // namespace shared
