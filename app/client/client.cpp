#include "client.hpp"
#include "../internal/simpleipc.hpp"
#include "../resources/resourceutil.hpp"
#include "../utils/config.hpp"
#include "../utils/logger.hpp"
#include <SDL3/SDL.h>

#include "client.hpp"
#include "../utils/logger.hpp"
#include "platform/sysplatform.hpp"
#include <filesystem>

CloseBrowserTask::CloseBrowserTask(CefRefPtr<HyperionClient> client, bool force_close)
    : client_(client), force_close_(force_close) {}

void CloseBrowserTask::Execute() { client_->DoCloseAllBrowsers(force_close_); }

std::string GetDownloadPath(const std::string &suggested_name) {
  auto filesystem = PlatformFactory::CreatePlatformFileSystem();
  if (filesystem) {
    std::string downloads_path = filesystem->GetKnownFolderPath(KnownFolder::Downloads);
    if (!downloads_path.empty()) {
      std::filesystem::path downloads(downloads_path);
      return (downloads / suggested_name).string();
    }
  }
  
  // fallback: current directory
  return suggested_name;
}