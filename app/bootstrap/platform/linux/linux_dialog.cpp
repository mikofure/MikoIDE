#ifdef __linux__

#include "linux_dialog.hpp"
#include "../../../utils/logger.hpp"

bool LinuxModernDialog::Create(PlatformInstance instance, PlatformWindow parent, const std::wstring &title) {
  (void)instance;
  (void)parent;
  (void)title;
  cancelled_.store(false);
  window_ = nullptr;
  Logger::LogMessage("Linux modern dialog support is not yet implemented.");
  return false;
}

void LinuxModernDialog::Show() {}

void LinuxModernDialog::Hide() {}

void LinuxModernDialog::SetProgress(int) {}

void LinuxModernDialog::SetStatus(const std::string &) {}

void LinuxModernDialog::SetDownloadInfo(size_t, size_t, size_t) {}

void LinuxModernDialog::UpdateProgress(int, const std::wstring &, size_t, size_t) {}

bool LinuxModernDialog::IsCancelled() const { return cancelled_.load(); }

PlatformWindow LinuxModernDialog::GetNativeHandle() const { return window_; }

#endif // __linux__
