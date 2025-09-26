#pragma once

#ifdef _WIN32

#include "../../ui_interface.hpp"
#include <d2d1.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi.h>
#include <wincodec.h>
#include <string>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowscodecs.lib")

class WindowsSplashScreen : public ISplashScreen {
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
    ID2D1SolidColorBrush* m_pAccentBrush;
    ID2D1Bitmap* m_pSplashBitmap;
    IWICImagingFactory* m_pWICFactory;
    
    std::string m_statusText;
    std::string m_titleText;
    bool m_visible;
    
    // Animation support
    float m_animationProgress;
    DWORD m_lastAnimationTime;
    
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
    void UpdateAnimation();
    
    // Modern UI drawing methods
    void DrawModernBackground();
    void DrawLogo();
    void DrawText();
    void DrawProgressIndicator();
    
public:
    WindowsSplashScreen();
    ~WindowsSplashScreen() override;
    
    // ISplashScreen implementation
    bool Create(NativeInstanceHandle instance, const std::string& title = "MikoIDE") override;
    void Show() override;
    void Hide() override;
    void UpdateStatus(const std::string& status) override;
    void SetTitle(const std::string& title) override;
    NativeWindowHandle GetNativeHandle() const override { return m_hwnd; }
    bool IsVisible() const override { return m_visible; }
    
    // Static methods for preloading
    static HRESULT PreloadSplashImage();
    static void CleanupPreloadedImage();
};

#endif // _WIN32