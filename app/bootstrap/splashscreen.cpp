// SplashScreen implementation
#include "splashscreen.hpp"
#include "../resources/splash.hpp"

// Static members for preloading
IWICBitmapSource *SplashScreen::s_pPreloadedBitmapSource = nullptr;
bool SplashScreen::s_imagePreloaded = false;

SplashScreen::SplashScreen()
    : m_hwnd(nullptr), m_pD2DFactory(nullptr), m_pRenderTarget(nullptr),
      m_pDWriteFactory(nullptr), m_pTextFormat(nullptr),
      m_pTitleFormat(nullptr), m_pBackgroundBrush(nullptr),
      m_pTextBrush(nullptr), m_pSecondaryTextBrush(nullptr),
      m_pSplashBitmap(nullptr), m_pWICFactory(nullptr),
      m_statusText(L"Initializing..."), m_titleText(L"MikoIDE"),
      m_visible(false) {}

SplashScreen::~SplashScreen() {
  DiscardDeviceResources();

  if (m_pWICFactory) {
    m_pWICFactory->Release();
    m_pWICFactory = nullptr;
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
    m_hwnd = nullptr;
  }
}

LRESULT CALLBACK SplashScreen::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                          LPARAM lParam) {
  SplashScreen *pThis = nullptr;

  if (uMsg == WM_NCCREATE) {
    CREATESTRUCT *pCreate = (CREATESTRUCT *)lParam;
    pThis = (SplashScreen *)pCreate->lpCreateParams;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    pThis->m_hwnd = hwnd;
  } else {
    pThis = (SplashScreen *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  }

  if (pThis) {
    switch (uMsg) {
    case WM_PAINT:
      pThis->OnPaint();
      return 0;
    case WM_SIZE:
      pThis->OnResize();
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    }
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HRESULT SplashScreen::CreateDeviceIndependentResources() {
  HRESULT hr = S_OK;

  // Create D2D factory
  if (SUCCEEDED(hr)) {
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
  }

  // Create DWrite factory
  if (SUCCEEDED(hr)) {
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                             __uuidof(m_pDWriteFactory),
                             reinterpret_cast<IUnknown **>(&m_pDWriteFactory));
  }

  // Create WIC factory
  if (SUCCEEDED(hr)) {
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                          CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWICFactory));
  }

  // Create text formats
  if (SUCCEEDED(hr)) {
    hr = m_pDWriteFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 14.0f, L"",
        &m_pTextFormat);
  }

  if (SUCCEEDED(hr)) {
    hr = m_pDWriteFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"",
        &m_pTitleFormat);
  }

  return hr;
}

HRESULT SplashScreen::PreloadSplashImage() {
  if (s_imagePreloaded) {
    return S_OK; // Already preloaded
  }

  HRESULT hr = S_OK;

  // Initialize COM for this thread
  CoInitialize(nullptr);

  // Create WIC factory
  IWICImagingFactory *pWICFactory = nullptr;
  hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS(&pWICFactory));

  if (SUCCEEDED(hr)) {
    // Create a stream from the embedded PNG data
    IWICStream *pStream = nullptr;
    hr = pWICFactory->CreateStream(&pStream);

    if (SUCCEEDED(hr)) {
      hr = pStream->InitializeFromMemory(
          const_cast<BYTE *>(GetSplashScreenData()), GetSplashScreenSize());
    }

    // Create decoder
    IWICBitmapDecoder *pDecoder = nullptr;
    if (SUCCEEDED(hr)) {
      hr = pWICFactory->CreateDecoderFromStream(
          pStream, nullptr, WICDecodeMetadataCacheOnLoad, &pDecoder);
    }

    // Get the first frame
    IWICBitmapFrameDecode *pFrame = nullptr;
    if (SUCCEEDED(hr)) {
      hr = pDecoder->GetFrame(0, &pFrame);
    }

    // Create format converter
    IWICFormatConverter *pConverter = nullptr;
    if (SUCCEEDED(hr)) {
      hr = pWICFactory->CreateFormatConverter(&pConverter);
    }

    if (SUCCEEDED(hr)) {
      hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppPBGRA,
                                  WICBitmapDitherTypeNone, nullptr, 0.0,
                                  WICBitmapPaletteTypeMedianCut);
    }

    if (SUCCEEDED(hr)) {
      s_pPreloadedBitmapSource = pConverter;
      s_pPreloadedBitmapSource->AddRef();
      s_imagePreloaded = true;
    }

    // Cleanup temporary objects
    if (pConverter)
      pConverter->Release();
    if (pFrame)
      pFrame->Release();
    if (pDecoder)
      pDecoder->Release();
    if (pStream)
      pStream->Release();
    if (pWICFactory)
      pWICFactory->Release();
  }

  return hr;
}

void SplashScreen::CleanupPreloadedImage() {
  if (s_pPreloadedBitmapSource) {
    s_pPreloadedBitmapSource->Release();
    s_pPreloadedBitmapSource = nullptr;
  }
  s_imagePreloaded = false;
}

HRESULT SplashScreen::LoadPreloadedImage() {
  HRESULT hr = S_OK;

  if (s_imagePreloaded && s_pPreloadedBitmapSource && m_pRenderTarget) {
    // Create D2D bitmap from preloaded WIC bitmap source
    hr = m_pRenderTarget->CreateBitmapFromWicBitmap(s_pPreloadedBitmapSource,
                                                    nullptr, &m_pSplashBitmap);
  } else {
    // Fallback to regular loading if preloading failed
    hr = LoadSplashImage();
  }

  return hr;
}

HRESULT SplashScreen::CreateDeviceResources() {
  HRESULT hr = S_OK;

  if (!m_pRenderTarget) {
    RECT rc;
    GetClientRect(m_hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    hr = m_pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hwnd, size), &m_pRenderTarget);

    if (SUCCEEDED(hr)) {
      // Create brushes
      hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x1e1e1e, 1.0f),
                                                  &m_pBackgroundBrush);
    }

    if (SUCCEEDED(hr)) {
      hr = m_pRenderTarget->CreateSolidColorBrush(
          D2D1::ColorF(D2D1::ColorF::White), &m_pTextBrush);
    }

    if (SUCCEEDED(hr)) {
      hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xcccccc, 1.0f),
                                                  &m_pSecondaryTextBrush);
    }

    if (SUCCEEDED(hr)) {
      hr = LoadPreloadedImage();
    }
  }

  return hr;
}

void SplashScreen::DiscardDeviceResources() {
  if (m_pSplashBitmap) {
    m_pSplashBitmap->Release();
    m_pSplashBitmap = nullptr;
  }

  if (m_pBackgroundBrush) {
    m_pBackgroundBrush->Release();
    m_pBackgroundBrush = nullptr;
  }

  if (m_pTextBrush) {
    m_pTextBrush->Release();
    m_pTextBrush = nullptr;
  }

  if (m_pSecondaryTextBrush) {
    m_pSecondaryTextBrush->Release();
    m_pSecondaryTextBrush = nullptr;
  }

  if (m_pRenderTarget) {
    m_pRenderTarget->Release();
    m_pRenderTarget = nullptr;
  }
}

HRESULT SplashScreen::LoadSplashImage() {
  HRESULT hr = S_OK;

  // Create a stream from the embedded PNG data
  IWICStream *pStream = nullptr;
  hr = m_pWICFactory->CreateStream(&pStream);

  if (SUCCEEDED(hr)) {
    hr = pStream->InitializeFromMemory(
        const_cast<BYTE *>(GetSplashScreenData()), GetSplashScreenSize());
  }

  // Create decoder
  IWICBitmapDecoder *pDecoder = nullptr;
  if (SUCCEEDED(hr)) {
    hr = m_pWICFactory->CreateDecoderFromStream(
        pStream, nullptr, WICDecodeMetadataCacheOnLoad, &pDecoder);
  }

  // Get the first frame
  IWICBitmapFrameDecode *pFrame = nullptr;
  if (SUCCEEDED(hr)) {
    hr = pDecoder->GetFrame(0, &pFrame);
  }

  // Create format converter
  IWICFormatConverter *pConverter = nullptr;
  if (SUCCEEDED(hr)) {
    hr = m_pWICFactory->CreateFormatConverter(&pConverter);
  }

  if (SUCCEEDED(hr)) {
    hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppPBGRA,
                                WICBitmapDitherTypeNone, nullptr, 0.0,
                                WICBitmapPaletteTypeMedianCut);
  }

  // Create D2D bitmap
  if (SUCCEEDED(hr)) {
    hr = m_pRenderTarget->CreateBitmapFromWicBitmap(pConverter, nullptr,
                                                    &m_pSplashBitmap);
  }

  // Clean up
  if (pConverter)
    pConverter->Release();
  if (pFrame)
    pFrame->Release();
  if (pDecoder)
    pDecoder->Release();
  if (pStream)
    pStream->Release();

  return hr;
}

void SplashScreen::OnPaint() {
  HRESULT hr = CreateDeviceResources();

  if (SUCCEEDED(hr)) {
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd, &ps);

    m_pRenderTarget->BeginDraw();

    // Clear background to transparent
    m_pRenderTarget->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));

    // Get render target size
    D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

    // Draw splash image if available
    if (m_pSplashBitmap) {
      // Stretch image to fit the entire window
      D2D1_RECT_F destRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);

      m_pRenderTarget->DrawBitmap(m_pSplashBitmap, destRect, 1.0f,
                                  D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    }

    hr = m_pRenderTarget->EndDraw();

    if (hr == D2DERR_RECREATE_TARGET) {
      DiscardDeviceResources();
    }

    EndPaint(m_hwnd, &ps);
  }
}

void SplashScreen::OnResize() {
  if (m_pRenderTarget) {
    RECT rc;
    GetClientRect(m_hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    m_pRenderTarget->Resize(size);
  }
}

bool SplashScreen::Create(HINSTANCE hInstance, const std::wstring &title) {
  // Initialize COM
  CoInitialize(nullptr);

  // Create device independent resources
  HRESULT hr = CreateDeviceIndependentResources();
  if (FAILED(hr)) {
    return false;
  }

  // Register window class
  const wchar_t CLASS_NAME[] = L"SplashScreenWindow";

  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.hbrBackground = nullptr; // No background brush for transparency
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

  RegisterClass(&wc);

  // Calculate window size and position
  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);
  int windowWidth = 810;
  int windowHeight = 199;
  int x = (screenWidth - windowWidth) / 2;
  int y = (screenHeight - windowHeight) / 2;

  // Create window with layered window support for transparency
  m_hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
                          CLASS_NAME, title.c_str(), WS_POPUP | WS_VISIBLE, x,
                          y, windowWidth, windowHeight, nullptr, nullptr,
                          hInstance, this);

  if (!m_hwnd) {
    return false;
  }

  // Set window transparency - make background fully transparent
  SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 255,
                             LWA_COLORKEY | LWA_ALPHA);

  m_titleText = title;
  return true;
}

void SplashScreen::Show() {
  if (m_hwnd) {
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    m_visible = true;
  }
}

void SplashScreen::Hide() {
  if (m_hwnd) {
    ShowWindow(m_hwnd, SW_HIDE);
    m_visible = false;
  }
}

void SplashScreen::UpdateStatus(const std::wstring &status) {
  m_statusText = status;
  if (m_hwnd && m_visible) {
    InvalidateRect(m_hwnd, nullptr, FALSE);
    UpdateWindow(m_hwnd);
  }
}

void SplashScreen::SetTitle(const std::wstring &title) {
  m_titleText = title;
  if (m_hwnd) {
    SetWindowText(m_hwnd, title.c_str());
    if (m_visible) {
      InvalidateRect(m_hwnd, nullptr, FALSE);
      UpdateWindow(m_hwnd);
    }
  }
}
