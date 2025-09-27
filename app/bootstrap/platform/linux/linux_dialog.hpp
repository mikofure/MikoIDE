#pragma once

#ifdef __linux__

#include "../../ui_interface.hpp"
#include <atomic>
#include <string>

class LinuxModernDialog : public IModernDialog {
public:
  LinuxModernDialog() = default;
  ~LinuxModernDialog() override = default;

  bool Create(PlatformInstance instance, PlatformWindow parent, const std::wstring &title) override;
  void Show() override;
  void Hide() override;
  void SetProgress(int percentage) override;
  void SetStatus(const std::string &status) override;
  void SetDownloadInfo(size_t bytesDownloaded, size_t totalBytes, size_t speed) override;
  void UpdateProgress(int percentage, const std::wstring &status, size_t bytesDownloaded, size_t totalBytes) override;
  bool IsCancelled() const override;
  PlatformWindow GetNativeHandle() const override;

private:
  std::atomic<bool> cancelled_{false};
  PlatformWindow window_{nullptr};
};

#endif // __linux__
