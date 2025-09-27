#ifdef __linux__

#include "linux_splash.hpp"
#include "../../../utils/logger.hpp"

#include <iostream>

bool LinuxSplashScreen::Create(NativeInstanceHandle instance, const std::string &title) {
  (void)instance;
  title_ = title;
  status_.clear();
  visible_ = false;
  Logger::LogMessage("Linux splash screen support is not yet implemented.");
  return false;
}

void LinuxSplashScreen::Show() { visible_ = false; }

void LinuxSplashScreen::Hide() { visible_ = false; }

void LinuxSplashScreen::UpdateStatus(const std::string &status) {
  status_ = status;
}

void LinuxSplashScreen::SetTitle(const std::string &title) { title_ = title; }

NativeWindowHandle LinuxSplashScreen::GetNativeHandle() const { return nullptr; }

bool LinuxSplashScreen::IsVisible() const { return visible_; }

bool LinuxSplashScreen::PreloadSplashImage() { return false; }

void LinuxSplashScreen::CleanupPreloadedImage() {}

#endif // __linux__
