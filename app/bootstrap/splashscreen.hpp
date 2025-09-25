// Splash Screen class for application initialization
#pragma once
#include <d2d1.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi.h>
#include <string>
#include <wincodec.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowscodecs.lib")
#include "../utils/logger.hpp"

class SplashScreen {
private:
  HWND m_hwnd;
  ID2D1Factory *m_pD2DFactory;
  ID2D1HwndRenderTarget *m_pRenderTarget;
  IDWriteFactory *m_pDWriteFactory;
  IDWriteTextFormat *m_pTextFormat;
  IDWriteTextFormat *m_pTitleFormat;
  ID2D1SolidColorBrush *m_pBackgroundBrush;
  ID2D1SolidColorBrush *m_pTextBrush;
  ID2D1SolidColorBrush *m_pSecondaryTextBrush;
  ID2D1Bitmap *m_pSplashBitmap;
  IWICImagingFactory *m_pWICFactory;

  std::wstring m_statusText;
  std::wstring m_titleText;
  bool m_visible;

  // Preloading support
  static IWICBitmapSource *s_pPreloadedBitmapSource;
  static bool s_imagePreloaded;

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);

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

  bool Create(HINSTANCE hInstance, const std::wstring &title = L"MikoIDE");
  void Show();
  void Hide();
  void UpdateStatus(const std::wstring &status);
  void SetTitle(const std::wstring &title);
  HWND GetHWND() const { return m_hwnd; }
  bool IsVisible() const { return m_visible; }

  // Static methods for preloading
  static HRESULT PreloadSplashImage();
  static void CleanupPreloadedImage();
};