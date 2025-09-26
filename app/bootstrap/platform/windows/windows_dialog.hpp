#pragma once

#ifdef _WIN32

#include "../../ui_interface.hpp"
#include <d2d1.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi.h>
#include <atomic>
#include <string>
#include <vector>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// Dark theme colors
struct DarkTheme {
    static constexpr D2D1_COLOR_F BACKGROUND = {0.13f, 0.13f, 0.13f, 1.0f}; // #212121
    static constexpr D2D1_COLOR_F SURFACE = {0.18f, 0.18f, 0.18f, 1.0f}; // #2D2D2D
    static constexpr D2D1_COLOR_F PRIMARY = {0.26f, 0.59f, 0.98f, 1.0f}; // #4285F4
    static constexpr D2D1_COLOR_F TEXT_PRIMARY = {0.87f, 0.87f, 0.87f, 1.0f}; // #DEDEDE
    static constexpr D2D1_COLOR_F TEXT_SECONDARY = {0.61f, 0.61f, 0.61f, 1.0f}; // #9C9C9C
    static constexpr D2D1_COLOR_F BORDER = {0.31f, 0.31f, 0.31f, 1.0f}; // #4F4F4F
    static constexpr D2D1_COLOR_F PROGRESS_BG = {0.24f, 0.24f, 0.24f, 1.0f}; // #3D3D3D
};

enum class DialogResult {
    None,
    OK,
    Cancel,
    Yes,
    No
};

struct DialogButton {
    std::string text;
    DialogResult result;
    D2D1_RECT_F rect;
};

class WindowsModernDialog : public IModernDialog {
private:
    HWND m_hwnd;
    HWND m_parentHwnd;
    ID2D1Factory* m_pD2DFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    IDWriteFactory* m_pDWriteFactory;
    IDWriteTextFormat* m_pTextFormat;
    IDWriteTextFormat* m_pTitleFormat;
    
    // Brushes for native D2D UI
    ID2D1SolidColorBrush* m_pBackgroundBrush;
    ID2D1SolidColorBrush* m_pSurfaceBrush;
    ID2D1SolidColorBrush* m_pPrimaryBrush;
    ID2D1SolidColorBrush* m_pTextBrush;
    ID2D1SolidColorBrush* m_pSecondaryTextBrush;
    ID2D1SolidColorBrush* m_pBorderBrush;
    ID2D1SolidColorBrush* m_pProgressBgBrush;
    ID2D1SolidColorBrush* m_pProgressFillBrush;
    
    // State
    std::wstring m_titleText;
    std::wstring m_statusText;
    std::wstring m_speedText;
    std::wstring m_sizeText;
    int m_progress;
    bool m_cancelled;
    size_t m_bytesDownloaded;
    size_t m_totalBytes;
    size_t m_downloadSpeed;
    
    // Dialog state
    DialogResult m_result;
    int m_hoveredButton;
    int m_pressedButton;
    std::vector<DialogButton> m_buttons;
    
    // Animation
    float m_animationProgress;
    DWORD m_lastAnimationTime;
    
    // Layout
    D2D1_RECT_F m_progressRect;
    D2D1_RECT_F m_cancelButtonRect;
    bool m_buttonHovered;
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();
    void OnPaint();
    void OnResize();
    void UpdateAnimation();
    void CalculateLayout();
    
    // Drawing methods
    void DrawBackground();
    void DrawTitle();
    void DrawStatus();
    void DrawProgressBar(const D2D1_RECT_F& rect);
    void DrawDownloadInfo();
    void DrawMessage();
    void DrawButtons();
    
    // Helper methods
    void OnMouseMove(int x, int y);
    void OnMouseClick(int x, int y);
    void OnMouseDown(int x, int y);
    void OnMouseUp(int x, int y);
    bool IsPointInRect(POINT pt, const RECT& rect);
    
public:
    WindowsModernDialog();
    ~WindowsModernDialog() override;
    
    // IModernDialog implementation
    bool Create(PlatformInstance instance, PlatformWindow parent, const std::wstring& title) override;
    void Show() override;
    void Hide() override;
    void SetProgress(int percentage) override;
    void SetStatus(const std::string& status) override;
    void SetDownloadInfo(size_t bytesDownloaded, size_t totalBytes, size_t speed) override;
    void UpdateProgress(int percentage, const std::wstring& status, size_t bytesDownloaded, size_t totalBytes) override;
    bool IsCancelled() const override { return m_cancelled; }
    NativeWindowHandle GetNativeHandle() const override { return m_hwnd; }
    
    // Native D2D methods
    void UpdateProgress(int percentage, const std::wstring& status, 
                       const std::wstring& speed = L"", const std::wstring& size = L"");
};

#endif // _WIN32