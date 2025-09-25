#ifndef BOOTSTRAP_HPP
#define BOOTSTRAP_HPP

#include <commctrl.h>
#include <windows.h>
#include <wininet.h>

#include <wincodec.h>

#include <atomic>
#include <filesystem>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "windowscodecs.lib")

#include "dialog.hpp"

// Bootstrap result codes
enum class BootstrapResult {
  SUCCESS,
  ALREADY_EXISTS,
  DOWNLOAD_FAILED,
  EXTRACT_FAILED,
  USER_CANCELLED,
  RELAUNCH_REQUIRED
};

// Progress callback function type
using ProgressCallback =
    std::function<void(int percentage, const std::string &status,
                       size_t bytesDownloaded, size_t totalBytes)>;

// Download chunk info for multi-connection downloads
struct DownloadChunk {
  size_t startByte = 0;
  size_t endByte = 0;
  size_t bytesDownloaded = 0;
  std::vector<char> buffer;
  bool completed = false;
  bool failed = false;

  DownloadChunk() = default;
  DownloadChunk(size_t start, size_t end)
      : startByte(start), endByte(end), bytesDownloaded(0), completed(false),
        failed(false) {}
};

// Bootstrap class for managing CEF helper download and extraction
class Bootstrap {
private:
  static std::unique_ptr<ModernDialog> s_modern_dialog;
  static std::atomic<bool> s_cancelled;
  static std::string s_current_status;
  static std::atomic<int> s_current_progress;
  static std::atomic<bool> s_download_completed;
  static std::atomic<bool> s_extract_completed;

  static bool DownloadFile(const std::string &url,
                           const std::filesystem::path &destination,
                           ProgressCallback callback);
  static bool DownloadFileSingle(const std::string &url,
                                 const std::filesystem::path &destination,
                                 ProgressCallback callback);
  static bool DownloadChunk(const std::string &url, DownloadChunk &chunk);
  static size_t GetRemoteFileSize(const std::string &url);
  static bool ExtractZip(const std::filesystem::path &zipPath,
                         const std::filesystem::path &extractPath,
                         ProgressCallback callback);
  static void UpdateProgress(int percentage, const std::string &status,
                             size_t bytesDownloaded = 0, size_t totalBytes = 0);
  static bool ShowModernDownloadDialog(HINSTANCE hInstance, HWND hParent);
  static bool DownloadUnzipBinary();
  static bool ExtractZipWithUnzip(const std::filesystem::path &zipPath,
                                  const std::filesystem::path &extractPath,
                                  ProgressCallback callback);

  static bool ValidateZipFile(const std::filesystem::path &zipPath);

public:
  static std::filesystem::path GetAppDirectory();
  static bool IsPathWithinAppDirectory(const std::filesystem::path &path);
  static bool ValidateAndCreateDirectory(const std::filesystem::path &path);
  static BootstrapResult CheckAndDownloadCEFHelper(HINSTANCE hInstance,
                                                   HWND hParent = nullptr);
  static std::string GetCEFHelperURL();
  static bool RelaunchApplication();
  static void InitializeGraphics();
};

// Helper functions
namespace BootstrapUtils {
std::string BytesToSize(size_t bytes);
std::string FormatSpeed(size_t bytesPerSecond);
bool CreateDirectoryRecursive(const std::filesystem::path &path);
std::filesystem::path GetTempFilePath(const std::string &filename);
bool IsValidExecutable(const std::filesystem::path &path);
size_t GetFileSize(const std::filesystem::path &path);
bool DeleteFileSafe(const std::filesystem::path &path);
bool DeleteTempPath();
} // namespace BootstrapUtils

#endif // BOOTSTRAP_HPP