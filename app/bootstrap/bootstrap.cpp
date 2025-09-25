#include "bootstrap.hpp"
#include "../utils/config.hpp"
#include "../utils/logger.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <comdef.h>
#include <d2d1.h>
#include <dwrite.h>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <sstream>
#include <thread>
#include <vector>
#include <wincodec.h>
#include <windowsx.h>
#include <wininet.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

// Static member definitions
std::unique_ptr<ModernDialog> Bootstrap::s_modern_dialog = nullptr;
std::atomic<bool> Bootstrap::s_cancelled{false};
std::string Bootstrap::s_current_status = "";
std::atomic<int> Bootstrap::s_current_progress{0};
std::atomic<bool> Bootstrap::s_download_completed{false};
std::atomic<bool> Bootstrap::s_extract_completed{false};

// Global variables for download tracking
static std::atomic<size_t> g_totalBytesDownloaded{0};
static std::atomic<size_t> g_totalFileSize{0};
static std::chrono::steady_clock::time_point g_downloadStartTime;
static std::mutex g_progressMutex;

bool Bootstrap::IsPathWithinAppDirectory(const std::filesystem::path &path) {
  try {
    const auto appDir = GetAppDirectory();
    std::error_code ec;

    // Get canonical app directory (this should always exist)
    const auto canonicalAppDir = std::filesystem::canonical(appDir, ec);
    if (ec) {
      Logger::LogMessage("Warning: Cannot get canonical app directory: " +
                         ec.message());
      return false;
    }

    // For the target path, we need to handle the case where it doesn't exist
    // yet
    std::filesystem::path canonicalPath;
    if (std::filesystem::exists(path)) {
      // Path exists, get its canonical form
      canonicalPath = std::filesystem::canonical(path, ec);
      if (ec) {
        Logger::LogMessage(
            "Warning: Cannot get canonical path for existing directory: " +
            path.string() + " - " + ec.message());
        return false;
      }
    } else {
      // Path doesn't exist, construct the canonical form manually
      // Start with the absolute path and resolve any .. or . components
      auto absolutePath = std::filesystem::absolute(path, ec);
      if (ec) {
        Logger::LogMessage("Warning: Cannot get absolute path: " +
                           path.string() + " - " + ec.message());
        return false;
      }
      canonicalPath = absolutePath.lexically_normal();
    }

    // Check if the path starts with the app directory
    auto appDirStr = canonicalAppDir.string();
    auto pathStr = canonicalPath.string();

    // Ensure both paths end with separator for proper comparison
    if (!appDirStr.empty() &&
        appDirStr.back() != std::filesystem::path::preferred_separator) {
      appDirStr += std::filesystem::path::preferred_separator;
    }

    return pathStr.find(appDirStr) == 0;
  } catch (const std::filesystem::filesystem_error &e) {
    // If we can't resolve paths, deny access for security
    Logger::LogMessage("Security: Path validation failed: " +
                       std::string(e.what()));
    return false;
  } catch (const std::exception &e) {
    // Catch any other exceptions
    Logger::LogMessage("Security: Unexpected error in path validation: " +
                       std::string(e.what()));
    return false;
  }
}

bool Bootstrap::ValidateAndCreateDirectory(const std::filesystem::path &path) {
  // First check if the path is within the app directory
  if (!IsPathWithinAppDirectory(path)) {
    Logger::LogMessage(
        "Security: Attempted to create directory outside app directory: " +
        path.string());
    return false;
  }

  // Create the directory if it doesn't exist
  std::error_code ec;
  if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
    return true;
  }

  bool result = std::filesystem::create_directories(path, ec);
  if (ec) {
    Logger::LogMessage("Failed to create directory: " + path.string() + " - " +
                       ec.message());
    return false;
  }

  return result;
}

// Validate ZIP file integrity by checking the central directory
bool Bootstrap::ValidateZipFile(const std::filesystem::path &zipPath) {
  std::ifstream file(zipPath, std::ios::binary);
  if (!file) {
    return false;
  }

  // Get file size
  file.seekg(0, std::ios::end);
  size_t fileSize = static_cast<size_t>(file.tellg());

  if (fileSize <
      22) { // Minimum ZIP file size (end of central directory record)
    return false;
  }

  // Look for end of central directory signature (0x06054b50) in the last 65KB
  const size_t searchSize = std::min(fileSize, static_cast<size_t>(65536));
  file.seekg(static_cast<std::streamoff>(fileSize - searchSize), std::ios::beg);

  std::vector<char> buffer(searchSize);
  file.read(buffer.data(), static_cast<std::streamsize>(searchSize));

  // Search for the signature from the end
  const uint32_t signature = 0x06054b50;
  for (size_t i = searchSize; i >= 4; --i) {
    uint32_t value = *reinterpret_cast<const uint32_t *>(&buffer[i - 4]);
    if (value == signature) {
      return true; // Found valid end of central directory
    }
  }

  return false; // No valid signature found
}

std::filesystem::path Bootstrap::GetAppDirectory() {
  wchar_t exePath[MAX_PATH];
  DWORD result = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
  if (result == 0 || result == MAX_PATH) {
    // Fallback to current directory only as last resort
    Logger::LogMessage(
        "Warning: Failed to get executable path, using current directory");
    return std::filesystem::current_path();
  }

  std::filesystem::path path(exePath);
  return path.parent_path();
}

// Initialize D2D1 and DWrite
void Bootstrap::InitializeGraphics() {
  // Initialize COM for D2D1 and DWrite
  CoInitialize(nullptr);
}

// Get CEF helper download URL from config
std::string Bootstrap::GetCEFHelperURL() { return CEF_HELPER_URL; }

// Update progress dialog
void Bootstrap::UpdateProgress(int percentage, const std::string &status,
                               size_t bytesDownloaded, size_t totalBytes) {
  s_current_progress = percentage;
  s_current_status = status;

  if (s_modern_dialog) {
    // Calculate speed
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now - g_downloadStartTime)
                       .count();

    size_t speed = 0;
    if (elapsed > 0) {
      speed = (bytesDownloaded * 1000) / elapsed; // bytes per second
    }

    // Format speed and size
    std::string speedStr = BootstrapUtils::FormatSpeed(speed);
    std::string sizeStr = BootstrapUtils::BytesToSize(bytesDownloaded) + " / " +
                          BootstrapUtils::BytesToSize(totalBytes);

    // Convert to wide strings
    std::wstring statusWStr(status.begin(), status.end());
    std::wstring speedWStr(speedStr.begin(), speedStr.end());
    std::wstring sizeWStr(sizeStr.begin(), sizeStr.end());

    s_modern_dialog->UpdateProgress(percentage, statusWStr, speedWStr,
                                    sizeWStr);
  }
}

// Get file size from URL
size_t Bootstrap::GetRemoteFileSize(const std::string &url) {
  HINTERNET hInternet = InternetOpenA(
      "MikoIDE Bootstrap", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
  if (!hInternet) {
    return 0;
  }

  HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), nullptr, 0,
                                    INTERNET_FLAG_RELOAD, 0);
  if (!hUrl) {
    InternetCloseHandle(hInternet);
    return 0;
  }

  DWORD contentLength = 0;
  DWORD bufferSize = sizeof(contentLength);
  HttpQueryInfoA(hUrl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
                 &contentLength, &bufferSize, nullptr);

  InternetCloseHandle(hUrl);
  InternetCloseHandle(hInternet);

  return static_cast<size_t>(contentLength);
}

// Download file chunk for multi-connection downloads
bool Bootstrap::DownloadChunk(const std::string &url, ::DownloadChunk &chunk) {
  HINTERNET hInternet = InternetOpenA(
      "MikoIDE Bootstrap", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
  if (!hInternet) {
    chunk.failed = true;
    return false;
  }

  // Parse URL to get components
  URL_COMPONENTSA urlComp = {0};
  urlComp.dwStructSize = sizeof(urlComp);
  char szHostName[256] = {0};
  char szUrlPath[1024] = {0};
  urlComp.lpszHostName = szHostName;
  urlComp.dwHostNameLength = sizeof(szHostName);
  urlComp.lpszUrlPath = szUrlPath;
  urlComp.dwUrlPathLength = sizeof(szUrlPath);

  if (!InternetCrackUrlA(url.c_str(), 0, 0, &urlComp)) {
    InternetCloseHandle(hInternet);
    chunk.failed = true;
    return false;
  }

  // Connect to server
  HINTERNET hConnect =
      InternetConnectA(hInternet, szHostName, urlComp.nPort, nullptr, nullptr,
                       INTERNET_SERVICE_HTTP, 0, 0);
  if (!hConnect) {
    InternetCloseHandle(hInternet);
    chunk.failed = true;
    return false;
  }

  // Create HTTP request with range header
  std::string rangeHeader = "Range: bytes=" + std::to_string(chunk.startByte) +
                            "-" + std::to_string(chunk.endByte);

  HINTERNET hRequest = HttpOpenRequestA(
      hConnect, "GET", szUrlPath, nullptr, nullptr, nullptr,
      INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE |
          (urlComp.nScheme == INTERNET_SCHEME_HTTPS ? INTERNET_FLAG_SECURE : 0),
      0);
  if (!hRequest) {
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    chunk.failed = true;
    return false;
  }

  // Send request with range header
  if (!HttpSendRequestA(hRequest, rangeHeader.c_str(),
                        static_cast<DWORD>(rangeHeader.length()), nullptr, 0)) {
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    chunk.failed = true;
    return false;
  }

  // Prepare buffer
  size_t chunkSize = chunk.endByte - chunk.startByte + 1;
  chunk.buffer.resize(chunkSize);

  // Download chunk
  const DWORD bufferSize = 8192;
  char tempBuffer[bufferSize];
  DWORD bytesRead = 0;
  size_t totalBytesRead = 0;

  while (InternetReadFile(hRequest, tempBuffer, bufferSize, &bytesRead) &&
         bytesRead > 0 && !s_cancelled) {
    if (totalBytesRead + bytesRead > chunkSize) {
      bytesRead = static_cast<DWORD>(chunkSize - totalBytesRead);
    }

    std::copy(tempBuffer, tempBuffer + bytesRead,
              chunk.buffer.begin() + totalBytesRead);
    totalBytesRead += bytesRead;
    chunk.bytesDownloaded = totalBytesRead;

    // Update global progress
    g_totalBytesDownloaded += bytesRead;

    if (totalBytesRead >= chunkSize) {
      break;
    }
  }

  InternetCloseHandle(hRequest);
  InternetCloseHandle(hConnect);
  InternetCloseHandle(hInternet);

  chunk.completed = (totalBytesRead == chunkSize) && !s_cancelled;
  chunk.failed = !chunk.completed;

  return chunk.completed;
}
bool Bootstrap::ShowModernDownloadDialog(HINSTANCE hInstance, HWND hParent) {
  s_modern_dialog = std::make_unique<ModernDialog>();

  if (!s_modern_dialog->Create(hInstance, hParent, L"Downloading CEF Helper")) {
    s_modern_dialog.reset();
    return false;
  }

  s_modern_dialog->Show();
  return true;
}

// Download file with progress callback (multi-connection support)
bool Bootstrap::DownloadFile(const std::string &url,
                             const std::filesystem::path &destination,
                             ProgressCallback callback) {
  Logger::LogMessage("Starting multi-connection download from: " + url);

  // Get file size first
  size_t fileSize = GetRemoteFileSize(url);
  if (fileSize == 0) {
    Logger::LogMessage(
        "Failed to get remote file size, falling back to single connection");
    return DownloadFileSingle(url, destination, callback);
  }

  g_totalFileSize = fileSize;
  g_totalBytesDownloaded = 0;
  g_downloadStartTime = std::chrono::steady_clock::now();

  // Determine number of connections (max 4 for reasonable server load)
  const size_t minChunkSize = 1024 * 1024; // 1MB minimum chunk size
  size_t numConnections =
      std::min(4ULL, std::max(1ULL, fileSize / minChunkSize));

  Logger::LogMessage("Using " + std::to_string(numConnections) +
                     " connections for " +
                     BootstrapUtils::BytesToSize(fileSize) + " file");

  // Create chunks
  std::vector<::DownloadChunk> chunks(numConnections);
  size_t chunkSize = fileSize / numConnections;

  for (size_t i = 0; i < numConnections; ++i) {
    chunks[i].startByte = i * chunkSize;
    chunks[i].endByte =
        (i == numConnections - 1) ? fileSize - 1 : (i + 1) * chunkSize - 1;
    chunks[i].bytesDownloaded = 0;
    chunks[i].completed = false;
    chunks[i].failed = false;
  }

  // Start download threads
  std::vector<std::thread> downloadThreads;
  for (size_t i = 0; i < numConnections; ++i) {
    downloadThreads.emplace_back(
        [&, i]() { Bootstrap::DownloadChunk(url, chunks[i]); });
  }

  // Progress monitoring thread
  std::thread progressThread([&]() {
    while (!s_cancelled) {
      size_t totalDownloaded = g_totalBytesDownloaded.load();
      int percentage =
          fileSize > 0 ? static_cast<int>((totalDownloaded * 100) / fileSize)
                       : 0;

      std::string status = "Downloading with " +
                           std::to_string(numConnections) + " connections...";
      callback(percentage, status, totalDownloaded, fileSize);

      // Check if all chunks are completed
      bool allCompleted = true;
      for (const auto &chunk : chunks) {
        if (!chunk.completed && !chunk.failed) {
          allCompleted = false;
          break;
        }
      }

      if (allCompleted) {
        break;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });

  // Wait for all downloads to complete
  for (auto &thread : downloadThreads) {
    thread.join();
  }

  progressThread.join();

  if (s_cancelled) {
    return false;
  }

  // Check if any chunk failed
  for (const auto &chunk : chunks) {
    if (chunk.failed) {
      Logger::LogMessage(
          "Chunk download failed, falling back to single connection");
      return DownloadFileSingle(url, destination, callback);
    }
  }

  // Combine chunks into final file
  callback(95, "Combining downloaded chunks...", fileSize, fileSize);

  std::ofstream outFile(destination, std::ios::binary);
  if (!outFile) {
    Logger::LogMessage("Failed to create destination file: " +
                       destination.string());
    return false;
  }

  for (const auto &chunk : chunks) {
    outFile.write(chunk.buffer.data(),
                  static_cast<std::streamsize>(chunk.buffer.size()));
  }

  outFile.close();

  // Validate ZIP file integrity
  if (!ValidateZipFile(destination)) {
    Logger::LogMessage(
        "Downloaded ZIP file is corrupted, falling back to single connection");
    BootstrapUtils::DeleteFileSafe(destination);
    return DownloadFileSingle(url, destination, callback);
  }

  callback(100, "Download completed", fileSize, fileSize);
  Logger::LogMessage("Multi-connection download completed successfully");
  return true;
}

// Fallback single connection download
bool Bootstrap::DownloadFileSingle(const std::string &url,
                                   const std::filesystem::path &destination,
                                   ProgressCallback callback) {
  HINTERNET hInternet = InternetOpenA(
      "MikoIDE Bootstrap", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
  if (!hInternet) {
    Logger::LogMessage("Failed to initialize WinINet");
    return false;
  }

  HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), nullptr, 0,
                                    INTERNET_FLAG_RELOAD, 0);
  if (!hUrl) {
    Logger::LogMessage("Failed to open URL: " + url);
    InternetCloseHandle(hInternet);
    return false;
  }

  // Get file size
  DWORD contentLength = 0;
  DWORD bufferSize = sizeof(contentLength);
  HttpQueryInfoA(hUrl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
                 &contentLength, &bufferSize, nullptr);

  g_totalFileSize = contentLength;
  g_totalBytesDownloaded = 0;
  g_downloadStartTime = std::chrono::steady_clock::now();

  // Create destination file
  std::ofstream outFile(destination, std::ios::binary);
  if (!outFile) {
    Logger::LogMessage("Failed to create destination file: " +
                       destination.string());
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    return false;
  }

  // Download with progress
  const DWORD bufferSize2 = 8192;
  char buffer[bufferSize2];
  DWORD bytesRead = 0;
  DWORD totalBytesRead = 0;

  callback(0, "Starting single connection download...", 0, contentLength);

  while (InternetReadFile(hUrl, buffer, bufferSize2, &bytesRead) &&
         bytesRead > 0) {
    if (s_cancelled) {
      outFile.close();
      std::filesystem::remove(destination);
      InternetCloseHandle(hUrl);
      InternetCloseHandle(hInternet);
      return false;
    }

    outFile.write(buffer, bytesRead);
    totalBytesRead += bytesRead;

    if (contentLength > 0) {
      int percentage = (int)((totalBytesRead * 100) / contentLength);
      std::string status = "Downloading...";
      callback(percentage, status, totalBytesRead, contentLength);
    } else {
      std::string status = "Downloading...";
      callback(0, status, totalBytesRead, 0);
    }
  }

  outFile.close();
  InternetCloseHandle(hUrl);
  InternetCloseHandle(hInternet);

  callback(100, "Download completed", totalBytesRead, contentLength);
  return true;
}

// Extract ZIP file with progress callback (using minizip-ng)
bool Bootstrap::ExtractZipWithUnzip(const std::filesystem::path &zipPath,
                                    const std::filesystem::path &extractPath,
                                    ProgressCallback callback) {
  Logger::LogMessage("Starting ZIP extraction using unzip.exe");

  try {
    // Validate ZIP file exists and has reasonable size
    if (!std::filesystem::exists(zipPath)) {
      Logger::LogMessage("ZIP file does not exist: " + zipPath.string());
      return false;
    }

    auto fileSize = std::filesystem::file_size(zipPath);
    if (fileSize == 0) {
      Logger::LogMessage("ZIP file is empty: " + zipPath.string());
      return false;
    }

    Logger::LogMessage("ZIP file validation passed: " + zipPath.string() +
                       " (" + std::to_string(fileSize) + " bytes)");

    // Ensure extraction directory exists
    std::filesystem::create_directories(extractPath);

    // Get unzip.exe path
    const auto exeDir = Bootstrap::GetAppDirectory();
    const std::filesystem::path unzipPath = exeDir / L"bin" / L"unzip.exe";

    if (!std::filesystem::exists(unzipPath)) {
      Logger::LogMessage("unzip.exe not found at: " + unzipPath.string());
      return false;
    }

    // Build command line: unzip.exe -o zipfile -d extractpath
    std::wstring cmdLine = L"\"" + unzipPath.wstring() + L"\" -o \"" +
                           zipPath.wstring() + L"\" -d \"" +
                           extractPath.wstring() + L"\"";
    Logger::LogMessage("Executing: " +
                       std::string(cmdLine.begin(), cmdLine.end()));

    // Create pipes for capturing output
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
      Logger::LogMessage("Failed to create pipe for unzip output");
      return false;
    }

    // Set up process startup info
    STARTUPINFOW si = {sizeof(STARTUPINFOW)};
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi = {};

    // Create process
    BOOL success = CreateProcessW(nullptr, const_cast<LPWSTR>(cmdLine.c_str()),
                                  nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
                                  nullptr, extractPath.c_str(), &si, &pi);

    CloseHandle(hWritePipe); // Close write end of pipe

    if (!success) {
      Logger::LogMessage("Failed to create unzip process");
      CloseHandle(hReadPipe);
      return false;
    }

    // Read output and parse progress
    std::string output;
    char buffer[4096];
    DWORD bytesRead;
    int totalFiles = 0;
    int extractedFiles = 0;

    while (
        ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) &&
        bytesRead > 0) {
      buffer[bytesRead] = '\0';
      output += buffer;

      // Parse output for progress
      std::string bufferStr(buffer);
      std::istringstream iss(bufferStr);
      std::string line;

      while (std::getline(iss, line)) {
        // Check for "inflating: filename" pattern
        if (line.find("inflating:") != std::string::npos) {
          extractedFiles++;
          size_t pos = line.find("inflating:");
          if (pos != std::string::npos) {
            std::string filename = line.substr(pos + 10);
            // Trim whitespace
            filename.erase(0, filename.find_first_not_of(" \t"));
            filename.erase(filename.find_last_not_of(" \t\r\n") + 1);

            if (callback) {
              int percentage =
                  totalFiles > 0 ? (extractedFiles * 100 / totalFiles) : 50;
              std::string status = "Extracting: " + filename;
              callback(percentage, status, extractedFiles, totalFiles);
            }

            Logger::LogMessage("Extracted: " + filename);
          }
        }
        // Check for "creating: foldername" pattern
        else if (line.find("creating:") != std::string::npos) {
          size_t pos = line.find("creating:");
          if (pos != std::string::npos) {
            std::string foldername = line.substr(pos + 9);
            // Trim whitespace
            foldername.erase(0, foldername.find_first_not_of(" \t"));
            foldername.erase(foldername.find_last_not_of(" \t\r\n") + 1);

            Logger::LogMessage("Created directory: " + foldername);
          }
        }
      }
    }

    // Wait for process to complete
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(hReadPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode == 0) {
      Logger::LogMessage(
          "ZIP extraction completed successfully using unzip.exe");
      if (callback) {
        callback(100, "ZIP extraction completed", 0, 0);
      }
      return true;
    } else {
      Logger::LogMessage("unzip.exe failed with exit code: " +
                         std::to_string(exitCode));
      Logger::LogMessage("unzip.exe output: " + output);
      return false;
    }

  } catch (const std::exception &e) {
    Logger::LogMessage("Exception during ZIP extraction with unzip.exe: " +
                       std::string(e.what()));
    return false;
  }
}

bool Bootstrap::ExtractZip(const std::filesystem::path &zipPath,
                           const std::filesystem::path &extractPath,
                           ProgressCallback callback) {
  // Use unzip.exe for ZIP extraction
  return ExtractZipWithUnzip(zipPath, extractPath, callback);
}

// Check if CEF helper exists and download if necessary
bool Bootstrap::DownloadUnzipBinary() {
  Logger::LogMessage("Bootstrap: Downloading unzip.exe...");

  // Get paths
  const auto exeDir = Bootstrap::GetAppDirectory();
  const std::filesystem::path unzipPath = exeDir / L"bin" / L"unzip.exe";

  // Check if unzip.exe already exists
  if (std::filesystem::exists(unzipPath) &&
      BootstrapUtils::IsValidExecutable(unzipPath)) {
    Logger::LogMessage("Bootstrap: unzip.exe already exists");
    return true;
  }

  // Create bin directory if it doesn't exist
  const std::filesystem::path binDir = exeDir / L"bin";
  if (!Bootstrap::ValidateAndCreateDirectory(binDir)) {
    Logger::LogMessage("Bootstrap: Failed to create or validate bin directory");
    return false;
  }

  // Download unzip.exe
  const std::string unzipUrl = "https://stahlworks.com/dev/unzip.exe";
  bool downloadSuccess = DownloadFileSingle(
      unzipUrl, unzipPath,
      [](int percentage, const std::string &status, size_t bytesDownloaded,
         size_t totalBytes) {
        UpdateProgress(percentage, "Downloading unzip.exe: " + status,
                       bytesDownloaded, totalBytes);
      });

  if (!downloadSuccess) {
    Logger::LogMessage("Bootstrap: Failed to download unzip.exe");
    return false;
  }

  // Verify the downloaded file
  if (!std::filesystem::exists(unzipPath) ||
      !BootstrapUtils::IsValidExecutable(unzipPath)) {
    Logger::LogMessage("Bootstrap: Downloaded unzip.exe is not valid");
    return false;
  }

  Logger::LogMessage("Bootstrap: unzip.exe downloaded successfully");
  return true;
}

BootstrapResult Bootstrap::CheckAndDownloadCEFHelper(HINSTANCE hInstance,
                                                     HWND hParent) {
  Logger::LogMessage("Bootstrap: Checking CEF helper...");

  // Reset completion flags
  s_download_completed = false;
  s_extract_completed = false;
  s_cancelled = false;

  // Get paths
  const auto exeDir = Bootstrap::GetAppDirectory();
  const bool is64 = sizeof(void *) == 8;
  const std::wstring platform = is64 ? L"windows64" : L"windows32";
  const std::filesystem::path cefDir = exeDir / L"bin" / L"cef" / platform;
  const std::filesystem::path helperPath = cefDir / L"mikowebhelper.exe";

  // Check if helper already exists
  if (std::filesystem::exists(helperPath) &&
      BootstrapUtils::IsValidExecutable(helperPath)) {
    Logger::LogMessage("Bootstrap: CEF helper already exists");
    return BootstrapResult::ALREADY_EXISTS;
  }

  Logger::LogMessage("Bootstrap: CEF helper not found, starting download...");

  // Initialize COM for shell operations
  CoInitialize(nullptr);

  // Initialize graphics for D2D1 and DWrite
  InitializeGraphics();

  // Show modern download dialog
  if (!ShowModernDownloadDialog(hInstance, hParent)) {
    Logger::LogMessage("Bootstrap: Failed to create download dialog");
    CoUninitialize();
    return BootstrapResult::DOWNLOAD_FAILED;
  }

  // First, download unzip.exe binary
  if (!DownloadUnzipBinary()) {
    if (s_modern_dialog) {
      s_modern_dialog->Hide();
      s_modern_dialog.reset();
    }
    CoUninitialize();
    return BootstrapResult::DOWNLOAD_FAILED;
  }

  // Get download URL
  std::string downloadUrl = GetCEFHelperURL();

  // Create temp file for download
  std::filesystem::path tempZip =
      BootstrapUtils::GetTempFilePath("mikowebhelper.zip");

  // Download file
  bool downloadSuccess = false;
  std::thread downloadThread([&]() {
    downloadSuccess = DownloadFile(
        downloadUrl, tempZip,
        [](int percentage, const std::string &status, size_t bytesDownloaded,
           size_t totalBytes) {
          UpdateProgress(percentage, status, bytesDownloaded, totalBytes);
        });
    s_download_completed = true;
  });

  // Message loop for dialog - wait for download completion
  MSG msg;
  while (!s_cancelled && !s_download_completed) {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        s_cancelled = true;
        break;
      }
      if (s_modern_dialog && s_modern_dialog->GetHWND()) {
        if (!IsDialogMessage(s_modern_dialog->GetHWND(), &msg)) {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      } else {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    // Small sleep to prevent 100% CPU usage
    Sleep(10);
  }

  // Wait for download thread to complete
  if (downloadThread.joinable()) {
    downloadThread.join();
  }

  if (s_cancelled) {
    if (s_modern_dialog) {
      s_modern_dialog->Hide();
      s_modern_dialog.reset();
    }
    BootstrapUtils::DeleteFileSafe(tempZip);
    CoUninitialize();
    return BootstrapResult::USER_CANCELLED;
  }

  if (!downloadSuccess) {
    if (s_modern_dialog) {
      s_modern_dialog->Hide();
      s_modern_dialog.reset();
    }
    BootstrapUtils::DeleteFileSafe(tempZip);
    CoUninitialize();
    return BootstrapResult::DOWNLOAD_FAILED;
  }

  // Extract ZIP file
  bool extractSuccess = false;
  std::thread extractThread([&]() {
    extractSuccess = ExtractZip(tempZip, cefDir,
                                [](int percentage, const std::string &status,
                                   size_t bytesDownloaded, size_t totalBytes) {
                                  UpdateProgress(percentage, status,
                                                 bytesDownloaded, totalBytes);
                                });
    s_extract_completed = true;
  });

  // Wait for extraction - use completion flag instead of joinable
  while (!s_cancelled && !s_extract_completed) {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        s_cancelled = true;
        break;
      }
      if (s_modern_dialog && s_modern_dialog->GetHWND()) {
        if (!IsDialogMessage(s_modern_dialog->GetHWND(), &msg)) {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      } else {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    // Small sleep to prevent 100% CPU usage
    Sleep(10);
  }

  // Wait for extraction thread to complete
  if (extractThread.joinable()) {
    extractThread.join();
  }

  // Cleanup
  if (s_modern_dialog) {
    s_modern_dialog->Hide();
    s_modern_dialog.reset();
  }
  BootstrapUtils::DeleteFileSafe(tempZip);
  CoUninitialize();

  if (s_cancelled) {
    return BootstrapResult::USER_CANCELLED;
  }

  if (!extractSuccess) {
    return BootstrapResult::EXTRACT_FAILED;
  }

  // Debug: List extracted files
  Logger::LogMessage("Bootstrap: Checking extraction directory: " +
                     cefDir.string());
  try {
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(cefDir)) {
      if (entry.is_regular_file()) {
        Logger::LogMessage("Bootstrap: Found file: " + entry.path().string());
      }
    }
  } catch (const std::exception &e) {
    Logger::LogMessage("Bootstrap: Error listing files: " +
                       std::string(e.what()));
  }

  Logger::LogMessage("Bootstrap: Looking for helper at: " +
                     helperPath.string());

  // Verify extraction
  if (!std::filesystem::exists(helperPath) ||
      !BootstrapUtils::IsValidExecutable(helperPath)) {
    Logger::LogMessage("Bootstrap: Extraction verification failed");
    return BootstrapResult::EXTRACT_FAILED;
  }

  Logger::LogMessage(
      "Bootstrap: CEF helper downloaded and extracted successfully");
  return BootstrapResult::SUCCESS;
}

// Relaunch application
bool Bootstrap::RelaunchApplication() {
  wchar_t exePath[MAX_PATH];
  DWORD result = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
  if (result == 0 || result == MAX_PATH) {
    return false;
  }

  STARTUPINFOW si = {};
  PROCESS_INFORMATION pi = {};
  si.cb = sizeof(si);

  if (CreateProcessW(exePath, nullptr, nullptr, nullptr, FALSE, 0, nullptr,
                     nullptr, &si, &pi)) {
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
  }

  return false;
}

// Helper functions implementation
namespace BootstrapUtils {
std::string BytesToSize(size_t bytes) {
  const char *units[] = {"B", "KB", "MB", "GB", "TB"};
  int unitIndex = 0;
  double size = static_cast<double>(bytes);

  while (size >= 1024.0 && unitIndex < 4) {
    size /= 1024.0;
    unitIndex++;
  }

  std::ostringstream oss;
  if (unitIndex == 0) {
    oss << static_cast<int>(size) << " " << units[unitIndex];
  } else {
    oss << std::fixed << std::setprecision(1) << size << " "
        << units[unitIndex];
  }

  return oss.str();
}

std::string FormatSpeed(size_t bytesPerSecond) {
  if (bytesPerSecond == 0) {
    return "0 B/s";
  }

  const char *units[] = {"B/s", "KB/s", "MB/s", "GB/s"};
  int unitIndex = 0;
  double speed = static_cast<double>(bytesPerSecond);

  while (speed >= 1024.0 && unitIndex < 3) {
    speed /= 1024.0;
    unitIndex++;
  }

  std::ostringstream oss;
  if (unitIndex == 0) {
    oss << static_cast<int>(speed) << " " << units[unitIndex];
  } else {
    oss << std::fixed << std::setprecision(1) << speed << " "
        << units[unitIndex];
  }

  return oss.str();
}

bool CreateDirectoryRecursive(const std::filesystem::path &path) {
  std::error_code ec;
  // If directory already exists, return true
  if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
    return true;
  }
  // Try to create the directory
  bool result = std::filesystem::create_directories(path, ec);
  // Return true if creation succeeded or if directory already exists
  return result || (!ec && std::filesystem::exists(path));
}

std::filesystem::path GetTempFilePath(const std::string &filename) {
  // Use app directory for temp files instead of system temp
  const auto appDir = Bootstrap::GetAppDirectory();
  const auto tempDir = appDir / "temp";

  // Create temp directory if it doesn't exist
  std::error_code ec;
  if (!Bootstrap::ValidateAndCreateDirectory(tempDir)) {
    Logger::LogMessage("Warning: Failed to create or validate temp directory, "
                       "using app directory");
    return appDir / filename;
  }

  return tempDir / filename;
}

bool IsValidExecutable(const std::filesystem::path &path) {
  if (!std::filesystem::exists(path)) {
    return false;
  }

  // Check if file has .exe extension and is not empty
  if (path.extension() != L".exe") {
    return false;
  }

  std::error_code ec;
  auto size = std::filesystem::file_size(path, ec);
  return !ec && size > 0;
}

size_t GetFileSize(const std::filesystem::path &path) {
  std::error_code ec;
  auto size = std::filesystem::file_size(path, ec);
  return ec ? 0 : size;
}

bool DeleteFileSafe(const std::filesystem::path &path) {
  if (!std::filesystem::exists(path)) {
    return true;
  }

  std::error_code ec;
  return std::filesystem::remove(path, ec);
}

bool DeleteTempPath() {
  const auto appDir = Bootstrap::GetAppDirectory();
  const auto tempDir = appDir / "temp";

  if (!std::filesystem::exists(tempDir)) {
    return true; // Nothing to delete
  }

  std::error_code ec;
  std::filesystem::remove_all(tempDir, ec);

  if (ec) {
    Logger::LogMessage("Warning: Failed to delete temp directory: " +
                       ec.message());
    return false;
  }

  Logger::LogMessage("Temp directory cleaned up successfully");
  return true;
}
} // namespace BootstrapUtils