#pragma once

#ifdef __linux__

#include "../../ui_interface.hpp"
#include <string>

class LinuxSplashScreen : public ISplashScreen {
public:
  LinuxSplashScreen() = default;
  ~LinuxSplashScreen() override = default;

  bool Create(NativeInstanceHandle instance, const std::string &title = "MikoIDE") override;
  void Show() override;
  void Hide() override;
  void UpdateStatus(const std::string &status) override;
  void SetTitle(const std::string &title) override;
  NativeWindowHandle GetNativeHandle() const override;
  bool IsVisible() const override;

  static bool PreloadSplashImage();
  static void CleanupPreloadedImage();

private:
  NativeWindowHandle window_handle_ = nullptr;
  bool visible_ = false;
  std::string title_;
  std::string status_;
};

#endif // __linux__
