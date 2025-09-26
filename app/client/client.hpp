#pragma once

// Windows includes for dwmapi
#include <dwmapi.h>
#include <windows.h>

// SDL3 includes
#include <SDL3/SDL.h>

// CEF includes
#include "include/cef_task.h"
#include "include/wrapper/cef_helpers.h"

// Forward declarations
class HyperionClient;
#include "../renderer/renderer_interface.hpp"
#include "../resources/resources.hpp"

#include <list>
#include <memory>
#include <mutex>
#include <string>

// import
#include "mikoclient.hpp"
#include "offscreenrender.hpp"
#include "windowed.hpp"

// Forward declarations
std::string GetDataURI(const std::string &data, const std::string &mime_type);
std::string GetDownloadPath(const std::string &suggested_name);

// Task class for CefPostTask compatibility
class CloseBrowserTask : public CefTask {
public:
  CloseBrowserTask(CefRefPtr<HyperionClient> client, bool force_close);
  void Execute() override;

private:
  CefRefPtr<HyperionClient> client_;
  bool force_close_;
  IMPLEMENT_REFCOUNTING(CloseBrowserTask);
};
