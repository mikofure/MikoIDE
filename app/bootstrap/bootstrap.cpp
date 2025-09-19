#include "bootstrap.hpp"
#include "../utils/config.hpp"
#include "../utils/logger.hpp"
#include <shlwapi.h>
#include <shellapi.h>
#include <windowsx.h>
#include <sstream>
#include <thread>
#include <chrono>
#include <fstream>
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <iomanip>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "wininet.lib")

// Static member definitions
std::unique_ptr<ModernDialog> Bootstrap::s_modern_dialog = nullptr;
bool Bootstrap::s_cancelled = false;
std::string Bootstrap::s_current_status = "";
int Bootstrap::s_current_progress = 0;

// Global variables for download tracking
static std::atomic<size_t> g_totalBytesDownloaded{0};
static std::atomic<size_t> g_totalFileSize{0};
static std::chrono::steady_clock::time_point g_downloadStartTime;
static std::mutex g_progressMutex;

// ModernDialog implementation
ModernDialog::ModernDialog() 
    : m_hwnd(nullptr)
    , m_pD2DFactory(nullptr)
    , m_pRenderTarget(nullptr)
    , m_pDWriteFactory(nullptr)
    , m_pTextFormat(nullptr)
    , m_pTitleFormat(nullptr)
    , m_pBackgroundBrush(nullptr)
    , m_pSurfaceBrush(nullptr)
    , m_pPrimaryBrush(nullptr)
    , m_pTextBrush(nullptr)
    , m_pSecondaryTextBrush(nullptr)
    , m_pBorderBrush(nullptr)
    , m_pProgressBgBrush(nullptr)
    , m_progress(0)
{
}

ModernDialog::~ModernDialog() {
    DiscardDeviceResources();
    
    if (m_pD2DFactory) {
        m_pD2DFactory->Release();
        m_pD2DFactory = nullptr;
    }
    
    if (m_pDWriteFactory) {
        m_pDWriteFactory->Release();
        m_pDWriteFactory = nullptr;
    }
    
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

LRESULT CALLBACK ModernDialog::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ModernDialog* pThis = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (ModernDialog*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hwnd = hwnd;
    } else {
        pThis = (ModernDialog*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    if (pThis) {
        switch (uMsg) {
            case WM_PAINT:
                pThis->OnPaint();
                return 0;
                
            case WM_SIZE:
                pThis->OnResize();
                return 0;
                
            case WM_CLOSE:
                // Set cancellation flag to stop background processes
                Bootstrap::s_cancelled = true;
                pThis->Hide();
                return 0;
                
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HRESULT ModernDialog::CreateDeviceIndependentResources() {
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    
    if (SUCCEEDED(hr)) {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(m_pDWriteFactory),
            reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
        );
    }
    
    if (SUCCEEDED(hr)) {
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            14.0f,
            L"",
            &m_pTextFormat
        );
    }
    
    if (SUCCEEDED(hr)) {
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_SEMI_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            16.0f,
            L"",
            &m_pTitleFormat
        );
    }
    
    return hr;
}

HRESULT ModernDialog::CreateDeviceResources() {
    HRESULT hr = S_OK;
    
    if (!m_pRenderTarget) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        
        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
        );
        
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget
        );
        
        if (SUCCEEDED(hr)) {
            hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::BACKGROUND, &m_pBackgroundBrush);
        }
        
        if (SUCCEEDED(hr)) {
            hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::SURFACE, &m_pSurfaceBrush);
        }
        
        if (SUCCEEDED(hr)) {
            hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::PRIMARY, &m_pPrimaryBrush);
        }
        
        if (SUCCEEDED(hr)) {
            hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::TEXT_PRIMARY, &m_pTextBrush);
        }
        
        if (SUCCEEDED(hr)) {
            hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::TEXT_SECONDARY, &m_pSecondaryTextBrush);
        }
        
        if (SUCCEEDED(hr)) {
            hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::BORDER, &m_pBorderBrush);
        }
        
        if (SUCCEEDED(hr)) {
            hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::PROGRESS_BG, &m_pProgressBgBrush);
        }
    }
    
    return hr;
}

void ModernDialog::DiscardDeviceResources() {
    if (m_pBackgroundBrush) { m_pBackgroundBrush->Release(); m_pBackgroundBrush = nullptr; }
    if (m_pSurfaceBrush) { m_pSurfaceBrush->Release(); m_pSurfaceBrush = nullptr; }
    if (m_pPrimaryBrush) { m_pPrimaryBrush->Release(); m_pPrimaryBrush = nullptr; }
    if (m_pTextBrush) { m_pTextBrush->Release(); m_pTextBrush = nullptr; }
    if (m_pSecondaryTextBrush) { m_pSecondaryTextBrush->Release(); m_pSecondaryTextBrush = nullptr; }
    if (m_pBorderBrush) { m_pBorderBrush->Release(); m_pBorderBrush = nullptr; }
    if (m_pProgressBgBrush) { m_pProgressBgBrush->Release(); m_pProgressBgBrush = nullptr; }
    if (m_pTextFormat) { m_pTextFormat->Release(); m_pTextFormat = nullptr; }
    if (m_pTitleFormat) { m_pTitleFormat->Release(); m_pTitleFormat = nullptr; }
    if (m_pRenderTarget) { m_pRenderTarget->Release(); m_pRenderTarget = nullptr; }
}

void ModernDialog::OnPaint() {
    HRESULT hr = CreateDeviceResources();
    
    if (SUCCEEDED(hr)) {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        
        m_pRenderTarget->BeginDraw();
        
        // Clear background with root background color
        m_pRenderTarget->Clear(DarkTheme::BACKGROUND);
        
        RECT clientRect;
        GetClientRect(m_hwnd, &clientRect);
        D2D1_RECT_F rect = D2D1::RectF(
            static_cast<float>(clientRect.left),
            static_cast<float>(clientRect.top),
            static_cast<float>(clientRect.right),
            static_cast<float>(clientRect.bottom)
        );
        
        // Draw title directly on background
        D2D1_RECT_F titleRect = D2D1::RectF(32, 32, rect.right - 32, 60);
        m_pRenderTarget->DrawText(
            m_titleText.c_str(),
            static_cast<UINT32>(m_titleText.length()),
            m_pTitleFormat,
            titleRect,
            m_pTextBrush
        );
        
        // Draw status text directly on background
        D2D1_RECT_F statusRect = D2D1::RectF(32, 70, rect.right - 32, 95);
        m_pRenderTarget->DrawText(
            m_statusText.c_str(),
            static_cast<UINT32>(m_statusText.length()),
            m_pTextFormat,
            statusRect,
            m_pSecondaryTextBrush
        );
        
        // Draw speed and size info
        if (!m_speedText.empty()) {
            D2D1_RECT_F speedRect = D2D1::RectF(32, 95, rect.right - 32, 115);
            m_pRenderTarget->DrawText(
                m_speedText.c_str(),
                static_cast<UINT32>(m_speedText.length()),
                m_pTextFormat,
                speedRect,
                m_pSecondaryTextBrush
            );
        }
        
        if (!m_sizeText.empty()) {
            D2D1_RECT_F sizeRect = D2D1::RectF(32, 115, rect.right - 32, 135);
            m_pRenderTarget->DrawText(
                m_sizeText.c_str(),
                static_cast<UINT32>(m_sizeText.length()),
                m_pTextFormat,
                sizeRect,
                m_pSecondaryTextBrush
            );
        }
        
        // Draw progress bar directly on background
        D2D1_RECT_F progressRect = D2D1::RectF(32, 145, rect.right - 32, 165);
        DrawProgressBar(progressRect);
        
        hr = m_pRenderTarget->EndDraw();
        
        if (hr == D2DERR_RECREATE_TARGET) {
            DiscardDeviceResources();
        }
        
        EndPaint(m_hwnd, &ps);
    }
}

void ModernDialog::OnResize() {
    if (m_pRenderTarget) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        
        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
        );
        
        m_pRenderTarget->Resize(size);
    }
}

void ModernDialog::DrawProgressBar(const D2D1_RECT_F& rect) {
    // Background
    D2D1_ROUNDED_RECT bgRect = D2D1::RoundedRect(rect, 4, 4);
    m_pRenderTarget->FillRoundedRectangle(bgRect, m_pProgressBgBrush);
    m_pRenderTarget->DrawRoundedRectangle(bgRect, m_pBorderBrush, 1.0f);
    
    // Progress fill
    if (m_progress > 0) {
        float progressWidth = (rect.right - rect.left) * (m_progress / 100.0f);
        D2D1_RECT_F fillRect = D2D1::RectF(rect.left, rect.top, rect.left + progressWidth, rect.bottom);
        D2D1_ROUNDED_RECT fillRoundedRect = D2D1::RoundedRect(fillRect, 4, 4);
        m_pRenderTarget->FillRoundedRectangle(fillRoundedRect, m_pPrimaryBrush);
    }
}

bool ModernDialog::IsPointInRect(POINT pt, const RECT& rect) {
    return pt.x >= rect.left && pt.x <= rect.right && pt.y >= rect.top && pt.y <= rect.bottom;
}

bool ModernDialog::Create(HINSTANCE hInstance, HWND hParent, const std::wstring& title) {
    m_titleText = title;
    
    HRESULT hr = CreateDeviceIndependentResources();
    if (FAILED(hr)) {
        return false;
    }
    
    // Register window class
    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASSEX wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WindowProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = nullptr; // We'll handle painting
        wcex.lpszClassName = L"ModernDialogClass";
        
        if (!RegisterClassEx(&wcex)) {
            return false;
        }
        classRegistered = true;
    }
    
    // Create window with modern style (disable minimize/maximize)
    m_hwnd = CreateWindowEx(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"ModernDialogClass",
        title.c_str(),
        WS_POPUP | WS_CAPTION | WS_SYSMENU,  // Removed WS_MINIMIZEBOX and WS_MAXIMIZEBOX
        (GetSystemMetrics(SM_CXSCREEN) - 500) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - 250) / 2,
        500, 250,
        hParent,
        nullptr,
        hInstance,
        this
    );
    
    if (m_hwnd) {
        // Enable dark theme for title bar using DWM API
        BOOL darkMode = TRUE;
        DwmSetWindowAttribute(m_hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
        
        // Alternative attribute for older Windows versions
        DwmSetWindowAttribute(m_hwnd, 20, &darkMode, sizeof(darkMode));
    }
    
    return m_hwnd != nullptr;
}

void ModernDialog::Show() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
        SetForegroundWindow(m_hwnd);
    }
}

void ModernDialog::Hide() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_HIDE);
    }
}

void ModernDialog::UpdateProgress(int percentage, const std::wstring& status, const std::wstring& speed, const std::wstring& size) {
    m_progress = percentage;
    m_statusText = status;
    m_speedText = speed;
    m_sizeText = size;
    
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

// Initialize D2D1 and DWrite
void Bootstrap::InitializeGraphics() {
    // Initialize COM for D2D1 and DWrite
    CoInitialize(nullptr);
}

// Get CEF helper download URL from config
std::string Bootstrap::GetCEFHelperURL() {
    return CEF_HELPER_URL;
}

// Update progress dialog
void Bootstrap::UpdateProgress(int percentage, const std::string& status, size_t bytesDownloaded, size_t totalBytes) {
    s_current_progress = percentage;
    s_current_status = status;
    
    if (s_modern_dialog) {
        // Calculate speed
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_downloadStartTime).count();
        
        size_t speed = 0;
        if (elapsed > 0) {
            speed = (bytesDownloaded * 1000) / elapsed; // bytes per second
        }
        
        // Format speed and size
        std::string speedStr = BootstrapUtils::FormatSpeed(speed);
        std::string sizeStr = BootstrapUtils::BytesToSize(bytesDownloaded) + " / " + BootstrapUtils::BytesToSize(totalBytes);
        
        // Convert to wide strings
        std::wstring statusWStr(status.begin(), status.end());
        std::wstring speedWStr(speedStr.begin(), speedStr.end());
        std::wstring sizeWStr(sizeStr.begin(), sizeStr.end());
        
        s_modern_dialog->UpdateProgress(percentage, statusWStr, speedWStr, sizeWStr);
    }
}

// Get file size from URL
size_t Bootstrap::GetRemoteFileSize(const std::string& url) {
    HINTERNET hInternet = InternetOpenA("MikoIDE Bootstrap", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!hInternet) {
        return 0;
    }
    
    HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), nullptr, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        return 0;
    }
    
    DWORD contentLength = 0;
    DWORD bufferSize = sizeof(contentLength);
    HttpQueryInfoA(hUrl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLength, &bufferSize, nullptr);
    
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    
    return static_cast<size_t>(contentLength);
}

// Download file chunk for multi-connection downloads
bool Bootstrap::DownloadChunk(const std::string& url, ::DownloadChunk& chunk) {
    HINTERNET hInternet = InternetOpenA("MikoIDE Bootstrap", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!hInternet) {
        chunk.failed = true;
        return false;
    }
    
    HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), nullptr, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        chunk.failed = true;
        return false;
    }
    
    // Set range header for partial content
    std::string rangeHeader = "Range: bytes=" + std::to_string(chunk.startByte) + "-" + std::to_string(chunk.endByte);
    HttpAddRequestHeadersA(hUrl, rangeHeader.c_str(), static_cast<DWORD>(rangeHeader.length()), HTTP_ADDREQ_FLAG_ADD);
    
    // Prepare buffer
    size_t chunkSize = chunk.endByte - chunk.startByte + 1;
    chunk.buffer.resize(chunkSize);
    
    // Download chunk
    const DWORD bufferSize = 8192;
    char tempBuffer[bufferSize];
    DWORD bytesRead = 0;
    size_t totalBytesRead = 0;
    
    while (InternetReadFile(hUrl, tempBuffer, bufferSize, &bytesRead) && bytesRead > 0 && !s_cancelled) {
        if (totalBytesRead + bytesRead > chunkSize) {
            bytesRead = static_cast<DWORD>(chunkSize - totalBytesRead);
        }
        
        std::copy(tempBuffer, tempBuffer + bytesRead, chunk.buffer.begin() + totalBytesRead);
        totalBytesRead += bytesRead;
        chunk.bytesDownloaded = totalBytesRead;
        
        // Update global progress
        g_totalBytesDownloaded += bytesRead;
        
        if (totalBytesRead >= chunkSize) {
            break;
        }
    }
    
    InternetCloseHandle(hUrl);
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
bool Bootstrap::DownloadFile(const std::string& url, const std::filesystem::path& destination, ProgressCallback callback) {
    Logger::LogMessage("Starting multi-connection download from: " + url);
    
    // Get file size first
    size_t fileSize = GetRemoteFileSize(url);
    if (fileSize == 0) {
        Logger::LogMessage("Failed to get remote file size, falling back to single connection");
        return DownloadFileSingle(url, destination, callback);
    }
    
    g_totalFileSize = fileSize;
    g_totalBytesDownloaded = 0;
    g_downloadStartTime = std::chrono::steady_clock::now();
    
    // Determine number of connections (max 4 for reasonable server load)
    const size_t minChunkSize = 1024 * 1024; // 1MB minimum chunk size
    size_t numConnections = std::min(4ULL, std::max(1ULL, fileSize / minChunkSize));
    
    Logger::LogMessage("Using " + std::to_string(numConnections) + " connections for " + BootstrapUtils::BytesToSize(fileSize) + " file");
    
    // Create chunks
    std::vector<::DownloadChunk> chunks(numConnections);
    size_t chunkSize = fileSize / numConnections;
    
    for (size_t i = 0; i < numConnections; ++i) {
        chunks[i].startByte = i * chunkSize;
        chunks[i].endByte = (i == numConnections - 1) ? fileSize - 1 : (i + 1) * chunkSize - 1;
        chunks[i].bytesDownloaded = 0;
        chunks[i].completed = false;
        chunks[i].failed = false;
    }
    
    // Start download threads
    std::vector<std::thread> downloadThreads;
    for (size_t i = 0; i < numConnections; ++i) {
        downloadThreads.emplace_back([&, i]() {
            Bootstrap::DownloadChunk(url, chunks[i]);
        });
    }
    
    // Progress monitoring thread
    std::thread progressThread([&]() {
        while (!s_cancelled) {
            size_t totalDownloaded = g_totalBytesDownloaded.load();
            int percentage = fileSize > 0 ? static_cast<int>((totalDownloaded * 100) / fileSize) : 0;
            
            std::string status = "Downloading with " + std::to_string(numConnections) + " connections...";
            callback(percentage, status, totalDownloaded, fileSize);
            
            // Check if all chunks are completed
            bool allCompleted = true;
            for (const auto& chunk : chunks) {
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
    for (auto& thread : downloadThreads) {
        thread.join();
    }
    
    progressThread.join();
    
    if (s_cancelled) {
        return false;
    }
    
    // Check if any chunk failed
    for (const auto& chunk : chunks) {
        if (chunk.failed) {
            Logger::LogMessage("Chunk download failed, falling back to single connection");
            return DownloadFileSingle(url, destination, callback);
        }
    }
    
    // Combine chunks into final file
    callback(95, "Combining downloaded chunks...", fileSize, fileSize);
    
    std::ofstream outFile(destination, std::ios::binary);
    if (!outFile) {
        Logger::LogMessage("Failed to create destination file: " + destination.string());
        return false;
    }
    
    for (const auto& chunk : chunks) {
        outFile.write(chunk.buffer.data(), static_cast<std::streamsize>(chunk.buffer.size()));
    }
    
    outFile.close();
    
    callback(100, "Download completed", fileSize, fileSize);
    Logger::LogMessage("Multi-connection download completed successfully");
    return true;
}

// Fallback single connection download
bool Bootstrap::DownloadFileSingle(const std::string& url, const std::filesystem::path& destination, ProgressCallback callback) {
    HINTERNET hInternet = InternetOpenA("MikoIDE Bootstrap", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!hInternet) {
        Logger::LogMessage("Failed to initialize WinINet");
        return false;
    }
    
    HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), nullptr, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        Logger::LogMessage("Failed to open URL: " + url);
        InternetCloseHandle(hInternet);
        return false;
    }
    
    // Get file size
    DWORD contentLength = 0;
    DWORD bufferSize = sizeof(contentLength);
    HttpQueryInfoA(hUrl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLength, &bufferSize, nullptr);
    
    g_totalFileSize = contentLength;
    g_totalBytesDownloaded = 0;
    g_downloadStartTime = std::chrono::steady_clock::now();
    
    // Create destination file
    std::ofstream outFile(destination, std::ios::binary);
    if (!outFile) {
        Logger::LogMessage("Failed to create destination file: " + destination.string());
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
    
    while (InternetReadFile(hUrl, buffer, bufferSize2, &bytesRead) && bytesRead > 0) {
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

// Extract ZIP file with improved error handling and progress
bool Bootstrap::ExtractZip(const std::filesystem::path& zipPath, const std::filesystem::path& extractPath, ProgressCallback callback) {
    Logger::LogMessage("Starting ZIP extraction from: " + zipPath.string() + " to: " + extractPath.string());
    Logger::LogMessage("Extracting filename: " + zipPath.filename().string());
    
    callback(0, "Initializing extraction...", 0, 0);
    
    // Ensure extract directory exists
    std::error_code ec;
    std::filesystem::create_directories(extractPath, ec);
    if (ec) {
        Logger::LogMessage("Failed to create extraction directory: " + ec.message());
        return false;
    }
    
    // Verify the ZIP file exists and is readable
    if (!std::filesystem::exists(zipPath, ec) || ec) {
        Logger::LogMessage("ZIP file does not exist: " + zipPath.string());
        return false;
    }
    
    callback(25, "Opening ZIP file for " + zipPath.filename().string() + "...", 0, 0);
    
    // Use Windows Shell API to extract ZIP file
    std::wstring zipPathW = zipPath.wstring();
    std::wstring extractPathW = extractPath.wstring();
    
    // Create shell folder objects
    IShellDispatch* pShellDispatch = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void**)&pShellDispatch);
    if (FAILED(hr) || !pShellDispatch) {
        Logger::LogMessage("Failed to create Shell dispatch object");
        return false;
    }
    
    callback(50, "Extracting files...", 0, 0);
    
    // Get the ZIP folder
    VARIANT zipVar;
    VariantInit(&zipVar);
    zipVar.vt = VT_BSTR;
    zipVar.bstrVal = SysAllocString(zipPathW.c_str());
    
    Folder* pZipFolder = nullptr;
    hr = pShellDispatch->NameSpace(zipVar, &pZipFolder);
    VariantClear(&zipVar);
    
    if (FAILED(hr) || !pZipFolder) {
        Logger::LogMessage("Failed to open ZIP folder");
        pShellDispatch->Release();
        return false;
    }
    
    // Get the destination folder
    VARIANT destVar;
    VariantInit(&destVar);
    destVar.vt = VT_BSTR;
    destVar.bstrVal = SysAllocString(extractPathW.c_str());
    
    Folder* pDestFolder = nullptr;
    hr = pShellDispatch->NameSpace(destVar, &pDestFolder);
    VariantClear(&destVar);
    
    if (FAILED(hr) || !pDestFolder) {
        Logger::LogMessage("Failed to open destination folder");
        pZipFolder->Release();
        pShellDispatch->Release();
        return false;
    }
    
    callback(75, "Copying files...", 0, 0);
    
    // Get items from ZIP
    FolderItems* pZipItems = nullptr;
    hr = pZipFolder->Items(&pZipItems);
    if (FAILED(hr) || !pZipItems) {
        Logger::LogMessage("Failed to get ZIP items");
        pDestFolder->Release();
        pZipFolder->Release();
        pShellDispatch->Release();
        return false;
    }
    
    // Copy all items with no UI and overwrite existing files
    VARIANT vOptions;
    VariantInit(&vOptions);
    vOptions.vt = VT_I4;
    vOptions.lVal = FOF_NO_UI | FOF_NOCONFIRMATION | FOF_NOERRORUI;
    
    VARIANT vItems;
    VariantInit(&vItems);
    vItems.vt = VT_DISPATCH;
    vItems.pdispVal = pZipItems;
    
    hr = pDestFolder->CopyHere(vItems, vOptions);
    
    VariantClear(&vItems);
    VariantClear(&vOptions);
    
    // Cleanup
    pZipItems->Release();
    pDestFolder->Release();
    pZipFolder->Release();
    pShellDispatch->Release();
    
    if (FAILED(hr)) {
        Logger::LogMessage("Failed to extract ZIP file using Shell API");
        return false;
    }
    
    callback(100, "Extraction completed for " + zipPath.filename().string(), 0, 0);
    Logger::LogMessage("ZIP extraction completed successfully using Windows Shell API");
    
    return true;
}

// Check if CEF helper exists and download if necessary
BootstrapResult Bootstrap::CheckAndDownloadCEFHelper(HINSTANCE hInstance, HWND hParent) {
    Logger::LogMessage("Bootstrap: Checking CEF helper...");
    
    // Get paths
    const auto exeDir = std::filesystem::current_path();
    const bool is64 = sizeof(void*) == 8;
    const std::wstring platform = is64 ? L"windows64" : L"windows32";
    const std::filesystem::path cefDir = exeDir / L"bin" / L"cef" / platform;
    const std::filesystem::path helperPath = cefDir / L"mikowebhelper.exe";
    
    // Check if helper already exists
    if (std::filesystem::exists(helperPath) && BootstrapUtils::IsValidExecutable(helperPath)) {
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
    
    // Get download URL
    std::string downloadUrl = GetCEFHelperURL();
    
    // Create temp file for download
    std::filesystem::path tempZip = BootstrapUtils::GetTempFilePath("mikowebhelper.zip");
    
    // Download file
    bool downloadSuccess = false;
    std::thread downloadThread([&]() {
        downloadSuccess = DownloadFile(downloadUrl, tempZip, [](int percentage, const std::string& status, size_t bytesDownloaded, size_t totalBytes) {
            UpdateProgress(percentage, status, bytesDownloaded, totalBytes);
        });
    });
    
    // Message loop for dialog
    MSG msg;
    while (!s_cancelled && downloadThread.joinable()) {
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
    }
    
    downloadThread.join();
    
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
        extractSuccess = ExtractZip(tempZip, cefDir, [](int percentage, const std::string& status, size_t bytesDownloaded, size_t totalBytes) {
            UpdateProgress(percentage, status, bytesDownloaded, totalBytes);
        });
    });
    
    // Wait for extraction
    while (!s_cancelled && extractThread.joinable()) {
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
        
        // Check if extraction thread has finished
        if (extractThread.joinable()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    extractThread.join();
    
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
    
    // Verify extraction
    if (!std::filesystem::exists(helperPath) || !BootstrapUtils::IsValidExecutable(helperPath)) {
        Logger::LogMessage("Bootstrap: Extraction verification failed");
        return BootstrapResult::EXTRACT_FAILED;
    }
    
    Logger::LogMessage("Bootstrap: CEF helper downloaded and extracted successfully");
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
    
    if (CreateProcessW(exePath, nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    
    return false;
}

// Helper functions implementation
namespace BootstrapUtils {
    std::string BytesToSize(size_t bytes) {
        const char* units[] = { "B", "KB", "MB", "GB", "TB" };
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
            oss << std::fixed << std::setprecision(1) << size << " " << units[unitIndex];
        }
        
        return oss.str();
    }
    
    std::string FormatSpeed(size_t bytesPerSecond) {
        if (bytesPerSecond == 0) {
            return "0 B/s";
        }
        
        const char* units[] = { "B/s", "KB/s", "MB/s", "GB/s" };
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
            oss << std::fixed << std::setprecision(1) << speed << " " << units[unitIndex];
        }
        
        return oss.str();
    }
    
    bool CreateDirectoryRecursive(const std::filesystem::path& path) {
        std::error_code ec;
        return std::filesystem::create_directories(path, ec);
    }
    
    std::filesystem::path GetTempFilePath(const std::string& filename) {
        wchar_t tempPath[MAX_PATH];
        DWORD result = GetTempPathW(MAX_PATH, tempPath);
        if (result == 0) {
            return std::filesystem::current_path() / filename;
        }
        
        std::filesystem::path temp(tempPath);
        return temp / filename;
    }
    
    bool IsValidExecutable(const std::filesystem::path& path) {
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
    
    size_t GetFileSize(const std::filesystem::path& path) {
        std::error_code ec;
        auto size = std::filesystem::file_size(path, ec);
        return ec ? 0 : size;
    }
    
    bool DeleteFileSafe(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            return true;
        }
        
        std::error_code ec;
        return std::filesystem::remove(path, ec);
    }
}