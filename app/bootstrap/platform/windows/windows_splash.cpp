#ifdef _WIN32

#include "windows_splash.hpp"
#include "../../../utils/logger.hpp"
#include <windowsx.h>
#include <dwmapi.h>
#include <cmath>

#pragma comment(lib, "dwmapi.lib")

// Static member definitions
IWICBitmapSource* WindowsSplashScreen::s_pPreloadedBitmapSource = nullptr;
bool WindowsSplashScreen::s_imagePreloaded = false;

// Window class name
static const wchar_t* SPLASH_WINDOW_CLASS = L"MikoIDESplashWindow";

WindowsSplashScreen::WindowsSplashScreen()
    : m_hwnd(nullptr)
    , m_pD2DFactory(nullptr)
    , m_pRenderTarget(nullptr)
    , m_pDWriteFactory(nullptr)
    , m_pTextFormat(nullptr)
    , m_pTitleFormat(nullptr)
    , m_pBackgroundBrush(nullptr)
    , m_pTextBrush(nullptr)
    , m_pSecondaryTextBrush(nullptr)
    , m_pAccentBrush(nullptr)
    , m_pSplashBitmap(nullptr)
    , m_pWICFactory(nullptr)
    , m_visible(false)
    , m_animationProgress(0.0f)
    , m_lastAnimationTime(0)
{
}

WindowsSplashScreen::~WindowsSplashScreen() {
    DiscardDeviceResources();
    
    if (m_pD2DFactory) {
        m_pD2DFactory->Release();
        m_pD2DFactory = nullptr;
    }
    
    if (m_pDWriteFactory) {
        m_pDWriteFactory->Release();
        m_pDWriteFactory = nullptr;
    }
    
    if (m_pWICFactory) {
        m_pWICFactory->Release();
        m_pWICFactory = nullptr;
    }
    
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

bool WindowsSplashScreen::Create(NativeInstanceHandle instance, const std::string& title) {
    HINSTANCE hInstance = static_cast<HINSTANCE>(instance);
    
    // Convert title to wide string
    int titleLen = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
    std::wstring wTitle(titleLen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wTitle[0], titleLen);
    wTitle.pop_back(); // Remove null terminator
    
    m_titleText = title;
    
    // Register window class
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr; // We'll handle painting
    wcex.lpszClassName = SPLASH_WINDOW_CLASS;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(101));
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(101));
    
    RegisterClassExW(&wcex);
    
    // Create device-independent resources
    if (FAILED(CreateDeviceIndependentResources())) {
        Logger::LogMessage("Failed to create device-independent resources for splash screen");
        return false;
    }
    
    // Calculate window size and position
    const int width = 480;
    const int height = 320;
    
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - width) / 2;
    int y = (screenHeight - height) / 2;
    
    // Create window
    m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        SPLASH_WINDOW_CLASS,
        wTitle.c_str(),
        WS_POPUP,
        x, y, width, height,
        nullptr, nullptr, hInstance, this
    );
    
    if (!m_hwnd) {
        Logger::LogMessage("Failed to create splash screen window");
        return false;
    }
    
    // Enable modern window effects
    BOOL enable = TRUE;
    DwmSetWindowAttribute(m_hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &enable, sizeof(enable));
    
    // Set window opacity
    SetLayeredWindowAttributes(m_hwnd, 0, 240, LWA_ALPHA);
    
    // Create device-dependent resources
    if (FAILED(CreateDeviceResources())) {
        Logger::LogMessage("Failed to create device resources for splash screen");
        return false;
    }
    
    // Load splash image
    LoadSplashImage();
    
    // Start animation timer
    m_lastAnimationTime = GetTickCount();
    SetTimer(m_hwnd, 1, 16, nullptr); // ~60 FPS
    
    return true;
}

void WindowsSplashScreen::Show() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
        m_visible = true;
    }
}

void WindowsSplashScreen::Hide() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_HIDE);
        m_visible = false;
        KillTimer(m_hwnd, 1);
    }
}

void WindowsSplashScreen::UpdateStatus(const std::string& status) {
    m_statusText = status;
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void WindowsSplashScreen::SetTitle(const std::string& title) {
    m_titleText = title;
    if (m_hwnd) {
        // Convert to wide string and set window title
        int titleLen = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
        std::wstring wTitle(titleLen, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wTitle[0], titleLen);
        wTitle.pop_back();
        
        SetWindowTextW(m_hwnd, wTitle.c_str());
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

LRESULT CALLBACK WindowsSplashScreen::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WindowsSplashScreen* pThis = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<WindowsSplashScreen*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<WindowsSplashScreen*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    
    if (pThis) {
        switch (uMsg) {
        case WM_PAINT:
            pThis->OnPaint();
            return 0;
            
        case WM_SIZE:
            pThis->OnResize();
            return 0;
            
        case WM_TIMER:
            if (wParam == 1) {
                pThis->UpdateAnimation();
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
            
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            return 0;
        }
    }
    
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

HRESULT WindowsSplashScreen::CreateDeviceIndependentResources() {
    HRESULT hr = S_OK;
    
    // Create D2D factory
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    if (FAILED(hr)) return hr;
    
    // Create DWrite factory
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory),
                            reinterpret_cast<IUnknown**>(&m_pDWriteFactory));
    if (FAILED(hr)) return hr;
    
    // Create WIC factory
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&m_pWICFactory));
    if (FAILED(hr)) return hr;
    
    // Create text formats
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
    if (FAILED(hr)) return hr;
    
    hr = m_pDWriteFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_SEMI_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        24.0f,
        L"",
        &m_pTitleFormat
    );
    if (FAILED(hr)) return hr;
    
    m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    m_pTitleFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_pTitleFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    
    return hr;
}

HRESULT WindowsSplashScreen::CreateDeviceResources() {
    HRESULT hr = S_OK;
    
    if (!m_pRenderTarget) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        
        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget
        );
        
        if (SUCCEEDED(hr)) {
            // Create brushes
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(0.1f, 0.1f, 0.1f, 0.95f), &m_pBackgroundBrush);
            
            if (SUCCEEDED(hr)) {
                hr = m_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::White), &m_pTextBrush);
            }
            
            if (SUCCEEDED(hr)) {
                hr = m_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(0.7f, 0.7f, 0.7f), &m_pSecondaryTextBrush);
            }
            
            if (SUCCEEDED(hr)) {
                hr = m_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(0.0f, 0.48f, 1.0f), &m_pAccentBrush);
            }
        }
    }
    
    return hr;
}

void WindowsSplashScreen::DiscardDeviceResources() {
    if (m_pRenderTarget) { m_pRenderTarget->Release(); m_pRenderTarget = nullptr; }
    if (m_pBackgroundBrush) { m_pBackgroundBrush->Release(); m_pBackgroundBrush = nullptr; }
    if (m_pTextBrush) { m_pTextBrush->Release(); m_pTextBrush = nullptr; }
    if (m_pSecondaryTextBrush) { m_pSecondaryTextBrush->Release(); m_pSecondaryTextBrush = nullptr; }
    if (m_pAccentBrush) { m_pAccentBrush->Release(); m_pAccentBrush = nullptr; }
    if (m_pSplashBitmap) { m_pSplashBitmap->Release(); m_pSplashBitmap = nullptr; }
}

void WindowsSplashScreen::OnPaint() {
    HRESULT hr = CreateDeviceResources();
    
    if (SUCCEEDED(hr)) {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        
        m_pRenderTarget->BeginDraw();
        
        DrawModernBackground();
        DrawLogo();
        DrawText();
        DrawProgressIndicator();
        
        hr = m_pRenderTarget->EndDraw();
        
        if (hr == D2DERR_RECREATE_TARGET) {
            DiscardDeviceResources();
        }
        
        EndPaint(m_hwnd, &ps);
    }
}

void WindowsSplashScreen::OnResize() {
    if (m_pRenderTarget) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        
        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        m_pRenderTarget->Resize(size);
    }
}

void WindowsSplashScreen::UpdateAnimation() {
    DWORD currentTime = GetTickCount();
    float deltaTime = (currentTime - m_lastAnimationTime) / 1000.0f;
    m_lastAnimationTime = currentTime;
    
    m_animationProgress += deltaTime * 2.0f; // 2 cycles per second
    if (m_animationProgress > 2.0f * 3.14159f) {
        m_animationProgress -= 2.0f * 3.14159f;
    }
}

void WindowsSplashScreen::DrawModernBackground() {
    D2D1_SIZE_F size = m_pRenderTarget->GetSize();
    
    // Clear with background
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    
    // Draw rounded rectangle background
    D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
        D2D1::RectF(10, 10, size.width - 10, size.height - 10),
        12, 12
    );
    
    m_pRenderTarget->FillRoundedRectangle(roundedRect, m_pBackgroundBrush);
}

void WindowsSplashScreen::DrawLogo() {
    if (m_pSplashBitmap) {
        D2D1_SIZE_F size = m_pRenderTarget->GetSize();
        D2D1_SIZE_F bitmapSize = m_pSplashBitmap->GetSize();
        
        float logoSize = 64.0f;
        D2D1_RECT_F destRect = D2D1::RectF(
            (size.width - logoSize) / 2,
            60,
            (size.width + logoSize) / 2,
            60 + logoSize
        );
        
        m_pRenderTarget->DrawBitmap(m_pSplashBitmap, destRect);
    }
}

void WindowsSplashScreen::DrawText() {
    D2D1_SIZE_F size = m_pRenderTarget->GetSize();
    
    // Draw title
    if (!m_titleText.empty()) {
        int titleLen = MultiByteToWideChar(CP_UTF8, 0, m_titleText.c_str(), -1, nullptr, 0);
        std::wstring wTitle(titleLen, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, m_titleText.c_str(), -1, &wTitle[0], titleLen);
        wTitle.pop_back();
        
        D2D1_RECT_F titleRect = D2D1::RectF(20, 140, size.width - 20, 180);
        m_pRenderTarget->DrawTextW(wTitle.c_str(), static_cast<UINT32>(wTitle.length()), m_pTitleFormat, titleRect, m_pTextBrush);
    }
    
    // Draw status
    if (!m_statusText.empty()) {
        int statusLen = MultiByteToWideChar(CP_UTF8, 0, m_statusText.c_str(), -1, nullptr, 0);
        std::wstring wStatus(statusLen, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, m_statusText.c_str(), -1, &wStatus[0], statusLen);
        wStatus.pop_back();
        
        D2D1_RECT_F statusRect = D2D1::RectF(20, 200, size.width - 20, 240);
        m_pRenderTarget->DrawTextW(wStatus.c_str(), static_cast<UINT32>(wStatus.length()), m_pTextFormat, statusRect, m_pSecondaryTextBrush);
    }
}

void WindowsSplashScreen::DrawProgressIndicator() {
    D2D1_SIZE_F size = m_pRenderTarget->GetSize();
    
    // Draw animated progress dots
    float centerX = size.width / 2;
    float centerY = 260;
    float radius = 4.0f;
    float spacing = 16.0f;
    
    for (int i = 0; i < 3; i++) {
        float x = centerX + (i - 1) * spacing;
        float alpha = (sin(m_animationProgress + i * 0.5f) + 1.0f) / 2.0f;
        
        ID2D1SolidColorBrush* brush;
        m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(0.0f, 0.48f, 1.0f, alpha), &brush);
        
        D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(x, centerY), radius, radius);
        m_pRenderTarget->FillEllipse(ellipse, brush);
        
        brush->Release();
    }
}

HRESULT WindowsSplashScreen::LoadSplashImage() {
    // Try to load preloaded image first
    if (s_imagePreloaded && s_pPreloadedBitmapSource) {
        return LoadPreloadedImage();
    }
    
    // Load from resources or file
    // This is a placeholder - implement actual image loading
    return S_OK;
}

HRESULT WindowsSplashScreen::LoadPreloadedImage() {
    if (!s_pPreloadedBitmapSource || !m_pRenderTarget) {
        return E_FAIL;
    }
    
    return m_pRenderTarget->CreateBitmapFromWicBitmap(s_pPreloadedBitmapSource, nullptr, &m_pSplashBitmap);
}

HRESULT WindowsSplashScreen::PreloadSplashImage() {
    // Implement image preloading
    s_imagePreloaded = true;
    return S_OK;
}

void WindowsSplashScreen::CleanupPreloadedImage() {
    if (s_pPreloadedBitmapSource) {
        s_pPreloadedBitmapSource->Release();
        s_pPreloadedBitmapSource = nullptr;
    }
    s_imagePreloaded = false;
}

#endif // _WIN32