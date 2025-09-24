#include "../client/client.hpp"
#include "../internal/simpleipc.hpp"
#include "../resources/resourceutil.hpp"
#include "../utils/config.hpp"
#include "../utils/logger.hpp"
#include <SDL3/SDL.h>

// CloseBrowserTask implementation
CloseBrowserTask::CloseBrowserTask(CefRefPtr<HyperionClient> client,
                                   bool force_close)
    : client_(client), force_close_(force_close) {}

void CloseBrowserTask::Execute() { client_->DoCloseAllBrowsers(force_close_); }
