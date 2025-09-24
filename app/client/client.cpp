#include "client.hpp"
#include "../internal/simpleipc.hpp"
#include "../resources/resourceutil.hpp"
#include "../utils/config.hpp"
#include "../utils/logger.hpp"
#include <SDL3/SDL.h>

#include <windows.h>
#include <shlobj.h>   // SHGetKnownFolderPath
#include <filesystem>

// CloseBrowserTask implementation
CloseBrowserTask::CloseBrowserTask(CefRefPtr<HyperionClient> client,
                                   bool force_close)
    : client_(client), force_close_(force_close) {}

void CloseBrowserTask::Execute() { client_->DoCloseAllBrowsers(force_close_); }

std::string GetDownloadPath(const std::string &suggested_name) {
    PWSTR path_tmp = nullptr;
    std::string result;

    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Downloads, 0, NULL, &path_tmp))) {
        std::filesystem::path downloads(path_tmp);
        result = (downloads / suggested_name).string();
        CoTaskMemFree(path_tmp);
    } else {
        // fallback: current directory
        result = suggested_name;
    }

    return result;
}