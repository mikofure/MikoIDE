// Dark theme colors
#pragma once

#include <d2d1.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi.h>
#include <string>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#include "../../utils/logger.hpp"
// Dialog resource IDs
#define IDD_DOWNLOAD_DIALOG 1001
#define IDC_DOWNLOAD_LABEL 1002
#define IDC_DOWNLOAD_PROGRESS 1003
#define IDC_DOWNLOAD_CANCEL 1004

struct DarkTheme {
  static constexpr D2D1_COLOR_F BACKGROUND = {0.13f, 0.13f, 0.13f,
                                              1.0f}; // #212121
  static constexpr D2D1_COLOR_F SURFACE = {0.18f, 0.18f, 0.18f,
                                           1.0f}; // #2D2D2D
  static constexpr D2D1_COLOR_F PRIMARY = {0.26f, 0.59f, 0.98f,
                                           1.0f}; // #4285F4
  static constexpr D2D1_COLOR_F TEXT_PRIMARY = {0.87f, 0.87f, 0.87f,
                                                1.0f}; // #DEDEDE
  static constexpr D2D1_COLOR_F TEXT_SECONDARY = {0.61f, 0.61f, 0.61f,
                                                  1.0f};              // #9C9C9C
  static constexpr D2D1_COLOR_F BORDER = {0.31f, 0.31f, 0.31f, 1.0f}; // #4F4F4F
  static constexpr D2D1_COLOR_F PROGRESS_BG = {0.24f, 0.24f, 0.24f,
                                               1.0f}; // #3D3D3D
};

// Modern UI Dialog class using D2D1 and DWrite
class ModernDialog {
private:
  HWND m_hwnd;
  ID2D1Factory *m_pD2DFactory;
  ID2D1HwndRenderTarget *m_pRenderTarget;
  IDWriteFactory *m_pDWriteFactory;
  IDWriteTextFormat *m_pTextFormat;
  IDWriteTextFormat *m_pTitleFormat;
  ID2D1SolidColorBrush *m_pBackgroundBrush;
  ID2D1SolidColorBrush *m_pSurfaceBrush;
  ID2D1SolidColorBrush *m_pPrimaryBrush;
  ID2D1SolidColorBrush *m_pTextBrush;
  ID2D1SolidColorBrush *m_pSecondaryTextBrush;
  ID2D1SolidColorBrush *m_pBorderBrush;
  ID2D1SolidColorBrush *m_pProgressBgBrush;

  int m_progress;
  std::wstring m_statusText;
  std::wstring m_titleText;
  std::wstring m_speedText;
  std::wstring m_sizeText;

  RECT m_progressRect;

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);

  HRESULT CreateDeviceIndependentResources();
  HRESULT CreateDeviceResources();
  void DiscardDeviceResources();
  void OnPaint();
  void OnResize();
  void DrawProgressBar(const D2D1_RECT_F &rect);
  bool IsPointInRect(POINT pt, const RECT &rect);

public:
  ModernDialog();
  ~ModernDialog();

  bool Create(HINSTANCE hInstance, HWND hParent, const std::wstring &title);
  void Show();
  void Hide();
  void UpdateProgress(int percentage, const std::wstring &status,
                      const std::wstring &speed = L"",
                      const std::wstring &size = L"");
  HWND GetHWND() const { return m_hwnd; }
};