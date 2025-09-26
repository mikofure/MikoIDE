#ifdef _WIN32

#include "windows_dialog.hpp"

WindowsModernDialog::WindowsModernDialog()
    : m_hwnd(nullptr), m_pD2DFactory(nullptr), m_pRenderTarget(nullptr),
      m_pDWriteFactory(nullptr), m_pTextFormat(nullptr),
      m_pTitleFormat(nullptr), m_pBackgroundBrush(nullptr),
      m_pSurfaceBrush(nullptr), m_pPrimaryBrush(nullptr), m_pTextBrush(nullptr),
      m_pSecondaryTextBrush(nullptr), m_pBorderBrush(nullptr),
      m_pProgressBgBrush(nullptr), m_pProgressFillBrush(nullptr),
      m_progress(0), m_cancelled(false), m_bytesDownloaded(0), m_totalBytes(0), m_downloadSpeed(0) {}

WindowsModernDialog::~WindowsModernDialog() {
  DiscardDeviceResources();

  if (m_pTextFormat) {
    m_pTextFormat->Release();
    m_pTextFormat = nullptr;
  }
  if (m_pTitleFormat) {
    m_pTitleFormat->Release();
    m_pTitleFormat = nullptr;
  }
  if (m_pDWriteFactory) {
    m_pDWriteFactory->Release();
    m_pDWriteFactory = nullptr;
  }
  if (m_pD2DFactory) {
    m_pD2DFactory->Release();
    m_pD2DFactory = nullptr;
  }

  if (m_hwnd) {
    DestroyWindow(m_hwnd);
  }
}

// IModernDialog implementation
bool WindowsModernDialog::Create(PlatformInstance instance, PlatformWindow parent, const std::wstring& title) {
    return CreateNative(static_cast<HINSTANCE>(instance), static_cast<HWND>(parent), title);
}

void WindowsModernDialog::SetProgress(int percentage) {
    m_progress = percentage;
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void WindowsModernDialog::SetStatus(const std::string& status) {
    int len = MultiByteToWideChar(CP_UTF8, 0, status.c_str(), -1, nullptr, 0);
    m_statusText.resize(len);
    MultiByteToWideChar(CP_UTF8, 0, status.c_str(), -1, &m_statusText[0], len);
    m_statusText.pop_back(); // Remove null terminator
    
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void WindowsModernDialog::SetDownloadInfo(size_t bytesDownloaded, size_t totalBytes, size_t speed) {
    m_bytesDownloaded = bytesDownloaded;
    m_totalBytes = totalBytes;
    m_downloadSpeed = speed;
    
    // Format speed text
    if (speed > 0) {
        wchar_t speedBuffer[64];
        if (speed >= 1024 * 1024) {
            swprintf_s(speedBuffer, L"%.1f MB/s", speed / (1024.0 * 1024.0));
        } else if (speed >= 1024) {
            swprintf_s(speedBuffer, L"%.1f KB/s", speed / 1024.0);
        } else {
            swprintf_s(speedBuffer, L"%zu B/s", speed);
        }
        m_speedText = speedBuffer;
    }
    
    // Format size text
    if (totalBytes > 0) {
        wchar_t sizeBuffer[64];
        if (totalBytes >= 1024 * 1024) {
            swprintf_s(sizeBuffer, L"%.1f / %.1f MB", 
                      bytesDownloaded / (1024.0 * 1024.0),
                      totalBytes / (1024.0 * 1024.0));
        } else if (totalBytes >= 1024) {
            swprintf_s(sizeBuffer, L"%.1f / %.1f KB",
                      bytesDownloaded / 1024.0,
                      totalBytes / 1024.0);
        } else {
            swprintf_s(sizeBuffer, L"%zu / %zu B", bytesDownloaded, totalBytes);
        }
        m_sizeText = sizeBuffer;
    }
    
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void WindowsModernDialog::UpdateProgress(int percentage, const std::wstring& status, size_t bytesDownloaded, size_t totalBytes) {
    m_progress = percentage;
    m_statusText = status;
    SetDownloadInfo(bytesDownloaded, totalBytes, m_downloadSpeed);
}

// Native D2D methods
bool WindowsModernDialog::CreateNative(HINSTANCE hInstance, HWND hParent,
                          const std::wstring &title) {
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
      WS_EX_DLGMODALFRAME | WS_EX_TOPMOST, L"ModernDialogClass", title.c_str(),
      WS_POPUP | WS_CAPTION |
          WS_SYSMENU, // Removed WS_MINIMIZEBOX and WS_MAXIMIZEBOX
      (GetSystemMetrics(SM_CXSCREEN) - 500) / 2,
      (GetSystemMetrics(SM_CYSCREEN) - 250) / 2, 500, 250, hParent, nullptr,
      hInstance, this);

  return m_hwnd != nullptr;
}

void WindowsModernDialog::Show() {
  if (m_hwnd) {
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    SetForegroundWindow(m_hwnd);
  }
}

void WindowsModernDialog::Hide() {
  if (m_hwnd) {
    ShowWindow(m_hwnd, SW_HIDE);
  }
}

void WindowsModernDialog::UpdateProgress(int percentage, const std::wstring &status,
                                  const std::wstring &speed,
                                  const std::wstring &size) {
  m_progress = percentage;
  m_statusText = status;
  m_speedText = speed;
  m_sizeText = size;

  if (m_hwnd) {
    InvalidateRect(m_hwnd, nullptr, FALSE);
  }
}

LRESULT CALLBACK WindowsModernDialog::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                          LPARAM lParam) {
  WindowsModernDialog *pThis = nullptr;

  if (uMsg == WM_NCCREATE) {
    CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
    pThis = reinterpret_cast<WindowsModernDialog *>(pCreate->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    pThis->m_hwnd = hwnd;
  } else {
    pThis = reinterpret_cast<WindowsModernDialog *>(
        GetWindowLongPtr(hwnd, GWLP_USERDATA));
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
      pThis->Hide();
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    }
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HRESULT WindowsModernDialog::CreateDeviceIndependentResources() {
  HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                 &m_pD2DFactory);

  if (SUCCEEDED(hr)) {
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                             __uuidof(m_pDWriteFactory),
                             reinterpret_cast<IUnknown **>(&m_pDWriteFactory));
  }

  if (SUCCEEDED(hr)) {
    hr = m_pDWriteFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 14.0f, L"", &m_pTextFormat);
  }

  if (SUCCEEDED(hr)) {
    hr = m_pDWriteFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"", &m_pTitleFormat);
  }

  return hr;
}

HRESULT WindowsModernDialog::CreateDeviceResources() {
  HRESULT hr = S_OK;

  if (!m_pRenderTarget) {
    RECT rc;
    GetClientRect(m_hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    hr = m_pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hwnd, size), &m_pRenderTarget);

    if (SUCCEEDED(hr)) {
      hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::BACKGROUND,
                                                  &m_pBackgroundBrush);
    }

    if (SUCCEEDED(hr)) {
      hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::SURFACE,
                                                  &m_pSurfaceBrush);
    }

    if (SUCCEEDED(hr)) {
      hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::PRIMARY,
                                                  &m_pPrimaryBrush);
    }

    if (SUCCEEDED(hr)) {
      hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::TEXT_PRIMARY,
                                                  &m_pTextBrush);
    }

    if (SUCCEEDED(hr)) {
      hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::TEXT_SECONDARY,
                                                  &m_pSecondaryTextBrush);
    }

    if (SUCCEEDED(hr)) {
      hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::BORDER,
                                                  &m_pBorderBrush);
    }

    if (SUCCEEDED(hr)) {
      hr = m_pRenderTarget->CreateSolidColorBrush(DarkTheme::PROGRESS_BG,
                                                  &m_pProgressBgBrush);
    }
  }

  return hr;
}

void WindowsModernDialog::DiscardDeviceResources() {
  if (m_pBackgroundBrush) {
    m_pBackgroundBrush->Release();
    m_pBackgroundBrush = nullptr;
  }
  if (m_pSurfaceBrush) {
    m_pSurfaceBrush->Release();
    m_pSurfaceBrush = nullptr;
  }
  if (m_pPrimaryBrush) {
    m_pPrimaryBrush->Release();
    m_pPrimaryBrush = nullptr;
  }
  if (m_pTextBrush) {
    m_pTextBrush->Release();
    m_pTextBrush = nullptr;
  }
  if (m_pSecondaryTextBrush) {
    m_pSecondaryTextBrush->Release();
    m_pSecondaryTextBrush = nullptr;
  }
  if (m_pBorderBrush) {
    m_pBorderBrush->Release();
    m_pBorderBrush = nullptr;
  }
  if (m_pProgressBgBrush) {
    m_pProgressBgBrush->Release();
    m_pProgressBgBrush = nullptr;
  }
  if (m_pTextFormat) {
    m_pTextFormat->Release();
    m_pTextFormat = nullptr;
  }
  if (m_pTitleFormat) {
    m_pTitleFormat->Release();
    m_pTitleFormat = nullptr;
  }
  if (m_pRenderTarget) {
    m_pRenderTarget->Release();
    m_pRenderTarget = nullptr;
  }
}

void WindowsModernDialog::OnPaint() {
  HRESULT hr = CreateDeviceResources();

  if (SUCCEEDED(hr)) {
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd, &ps);

    m_pRenderTarget->BeginDraw();

    // Clear background with root background color
    m_pRenderTarget->Clear(DarkTheme::BACKGROUND);

    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    D2D1_RECT_F rect = D2D1::RectF(static_cast<float>(clientRect.left),
                                   static_cast<float>(clientRect.top),
                                   static_cast<float>(clientRect.right),
                                   static_cast<float>(clientRect.bottom));

    // Draw title directly on background
    D2D1_RECT_F titleRect = D2D1::RectF(32, 32, rect.right - 32, 60);
    m_pRenderTarget->DrawText(m_titleText.c_str(),
                              static_cast<UINT32>(m_titleText.length()),
                              m_pTitleFormat, titleRect, m_pTextBrush);

    // Draw status text directly on background
    D2D1_RECT_F statusRect = D2D1::RectF(32, 70, rect.right - 32, 95);
    m_pRenderTarget->DrawText(m_statusText.c_str(),
                              static_cast<UINT32>(m_statusText.length()),
                              m_pTextFormat, statusRect, m_pSecondaryTextBrush);

    // Draw speed and size info
    if (!m_speedText.empty()) {
      D2D1_RECT_F speedRect = D2D1::RectF(32, 95, rect.right - 32, 115);
      m_pRenderTarget->DrawText(
          m_speedText.c_str(), static_cast<UINT32>(m_speedText.length()),
          m_pTextFormat, speedRect, m_pSecondaryTextBrush);
    }

    if (!m_sizeText.empty()) {
      D2D1_RECT_F sizeRect = D2D1::RectF(32, 115, rect.right - 32, 135);
      m_pRenderTarget->DrawText(m_sizeText.c_str(),
                                static_cast<UINT32>(m_sizeText.length()),
                                m_pTextFormat, sizeRect, m_pSecondaryTextBrush);
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

void WindowsModernDialog::OnResize() {
  if (m_pRenderTarget) {
    RECT rc;
    GetClientRect(m_hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    m_pRenderTarget->Resize(size);
  }
}

void WindowsModernDialog::DrawProgressBar(const D2D1_RECT_F &rect) {
  // Background
  D2D1_ROUNDED_RECT bgRect = D2D1::RoundedRect(rect, 4, 4);
  m_pRenderTarget->FillRoundedRectangle(bgRect, m_pProgressBgBrush);
  m_pRenderTarget->DrawRoundedRectangle(bgRect, m_pBorderBrush, 1.0f);

  // Progress fill
  if (m_progress > 0) {
    float progressWidth = (rect.right - rect.left) * (m_progress / 100.0f);
    D2D1_RECT_F fillRect = D2D1::RectF(rect.left, rect.top,
                                       rect.left + progressWidth, rect.bottom);
    D2D1_ROUNDED_RECT fillRoundedRect = D2D1::RoundedRect(fillRect, 4, 4);
    m_pRenderTarget->FillRoundedRectangle(fillRoundedRect, m_pPrimaryBrush);
  }
}

bool WindowsModernDialog::IsPointInRect(POINT pt, const RECT &rect) {
  return pt.x >= rect.left && pt.x <= rect.right && pt.y >= rect.top &&
         pt.y <= rect.bottom;
}



#endif // _WIN32