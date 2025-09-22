#ifndef BOOTSTRAP_HPP
#define BOOTSTRAP_HPP

#include <windows.h>
#include <wininet.h>
#include <commctrl.h>
#include <d2d1.h>
#include <dwrite.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wincodec.h>
#include <string>
#include <filesystem>
#include <functional>
#include <vector>
#include <thread>
#include <memory>
#include <atomic>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowscodecs.lib")

// Dialog resource IDs
#define IDD_DOWNLOAD_DIALOG 1001
#define IDC_DOWNLOAD_LABEL 1002
#define IDC_DOWNLOAD_PROGRESS 1003
#define IDC_DOWNLOAD_CANCEL 1004

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
using ProgressCallback = std::function<void(int percentage, const std::string& status, size_t bytesDownloaded, size_t totalBytes)>;

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
        : startByte(start), endByte(end),
          bytesDownloaded(0),
          completed(false), failed(false) {}
};


// Dark theme colors
struct DarkTheme {
    static constexpr D2D1_COLOR_F BACKGROUND = { 0.13f, 0.13f, 0.13f, 1.0f };      // #212121
    static constexpr D2D1_COLOR_F SURFACE = { 0.18f, 0.18f, 0.18f, 1.0f };         // #2D2D2D
    static constexpr D2D1_COLOR_F PRIMARY = { 0.26f, 0.59f, 0.98f, 1.0f };         // #4285F4
    static constexpr D2D1_COLOR_F TEXT_PRIMARY = { 0.87f, 0.87f, 0.87f, 1.0f };    // #DEDEDE
    static constexpr D2D1_COLOR_F TEXT_SECONDARY = { 0.61f, 0.61f, 0.61f, 1.0f };  // #9C9C9C
    static constexpr D2D1_COLOR_F BORDER = { 0.31f, 0.31f, 0.31f, 1.0f };          // #4F4F4F
    static constexpr D2D1_COLOR_F PROGRESS_BG = { 0.24f, 0.24f, 0.24f, 1.0f };     // #3D3D3D
};

// Modern UI Dialog class using D2D1 and DWrite
class ModernDialog {
private:
    HWND m_hwnd;
    ID2D1Factory* m_pD2DFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    IDWriteFactory* m_pDWriteFactory;
    IDWriteTextFormat* m_pTextFormat;
    IDWriteTextFormat* m_pTitleFormat;
    ID2D1SolidColorBrush* m_pBackgroundBrush;
    ID2D1SolidColorBrush* m_pSurfaceBrush;
    ID2D1SolidColorBrush* m_pPrimaryBrush;
    ID2D1SolidColorBrush* m_pTextBrush;
    ID2D1SolidColorBrush* m_pSecondaryTextBrush;
    ID2D1SolidColorBrush* m_pBorderBrush;
    ID2D1SolidColorBrush* m_pProgressBgBrush;

    int m_progress;
    std::wstring m_statusText;
    std::wstring m_titleText;
    std::wstring m_speedText;
    std::wstring m_sizeText;

    RECT m_progressRect;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();
    void OnPaint();
    void OnResize();
    void DrawProgressBar(const D2D1_RECT_F& rect);
    bool IsPointInRect(POINT pt, const RECT& rect);

public:
    ModernDialog();
    ~ModernDialog();

    bool Create(HINSTANCE hInstance, HWND hParent, const std::wstring& title);
    void Show();
    void Hide();
    void UpdateProgress(int percentage, const std::wstring& status, const std::wstring& speed = L"", const std::wstring& size = L"");
    HWND GetHWND() const { return m_hwnd; }
};

// Splash Screen class for application initialization
class SplashScreen {
private:
    HWND m_hwnd;
    ID2D1Factory* m_pD2DFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    IDWriteFactory* m_pDWriteFactory;
    IDWriteTextFormat* m_pTextFormat;
    IDWriteTextFormat* m_pTitleFormat;
    ID2D1SolidColorBrush* m_pBackgroundBrush;
    ID2D1SolidColorBrush* m_pTextBrush;
    ID2D1SolidColorBrush* m_pSecondaryTextBrush;
    ID2D1Bitmap* m_pSplashBitmap;
    IWICImagingFactory* m_pWICFactory;

    std::wstring m_statusText;
    std::wstring m_titleText;
    bool m_visible;
    
    // Preloading support
    static IWICBitmapSource* s_pPreloadedBitmapSource;
    static bool s_imagePreloaded;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();
    HRESULT LoadSplashImage();
    HRESULT LoadPreloadedImage();
    void OnPaint();
    void OnResize();

public:
    SplashScreen();
    ~SplashScreen();

    bool Create(HINSTANCE hInstance, const std::wstring& title = L"MikoIDE");
    void Show();
    void Hide();
    void UpdateStatus(const std::wstring& status);
    void SetTitle(const std::wstring& title);
    HWND GetHWND() const { return m_hwnd; }
    bool IsVisible() const { return m_visible; }
    
    // Static methods for preloading
    static HRESULT PreloadSplashImage();
    static void CleanupPreloadedImage();
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

    static bool DownloadFile(const std::string& url, const std::filesystem::path& destination, ProgressCallback callback);
    static bool DownloadFileSingle(const std::string& url, const std::filesystem::path& destination, ProgressCallback callback);
    static bool DownloadChunk(const std::string& url, DownloadChunk& chunk);
    static size_t GetRemoteFileSize(const std::string& url);
    static bool ExtractZip(const std::filesystem::path& zipPath, const std::filesystem::path& extractPath, ProgressCallback callback);
    static void UpdateProgress(int percentage, const std::string& status, size_t bytesDownloaded = 0, size_t totalBytes = 0);
    static bool ShowModernDownloadDialog(HINSTANCE hInstance, HWND hParent);
    static bool DownloadUnzipBinary();
        static bool ExtractZipWithUnzip(const std::filesystem::path& zipPath, const std::filesystem::path& extractPath, ProgressCallback callback);
        static bool ExtractZipWithMiniz(const std::filesystem::path& zipPath, const std::filesystem::path& extractPath, ProgressCallback callback);
        static bool ValidateZipFile(const std::filesystem::path& zipPath);

public:
    static BootstrapResult CheckAndDownloadCEFHelper(HINSTANCE hInstance, HWND hParent = nullptr);
    static std::string GetCEFHelperURL();
    static bool RelaunchApplication();
    static void InitializeGraphics();
};

// Helper functions
namespace BootstrapUtils {
    std::string BytesToSize(size_t bytes);
    std::string FormatSpeed(size_t bytesPerSecond);
    bool CreateDirectoryRecursive(const std::filesystem::path& path);
    std::filesystem::path GetTempFilePath(const std::string& filename);
    bool IsValidExecutable(const std::filesystem::path& path);
    size_t GetFileSize(const std::filesystem::path& path);
    bool DeleteFileSafe(const std::filesystem::path& path);
}

#endif // BOOTSTRAP_HPP