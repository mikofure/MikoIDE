#ifdef _WIN32

#include "windows_dialog.hpp"

WindowsModernDialog::WindowsModernDialog()
    : m_hwnd(nullptr), m_pD2DFactory(nullptr), m_pRenderTarget(nullptr),
      m_pDWriteFactory(nullptr), m_pTextFormat(nullptr),
      m_pTitleFormat(nullptr), m_pBackgroundBrush(nullptr),
      m_pSurfaceBrush(nullptr), m_pPrimaryBrush(nullptr), m_pTextBrush(nullptr),
      m_pSecondaryTextBrush(nullptr), m_pBorderBrush(nullptr),
      m_pProgressBgBrush(nullptr), m_progress(0) {}

WindowsModernDialog::~WindowsModernDialog() {
  DiscardDeviceResources();

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

bool WindowsModernDialog::Create(HINSTANCE hInstance, HWND hParent,
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

void WindowsModernDialog::Show() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
        SetForegroundWindow(m_hwnd);
        m_visible = true;
    }
}

void WindowsModernDialog::Hide() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_HIDE);
        m_visible = false;
        KillTimer(m_hwnd, 1);
    }
}

void WindowsModernDialog::SetTitle(const std::string& title) {
    m_titleText = title;
    if (m_hwnd) {
        int titleLen = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
        std::wstring wTitle(titleLen, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wTitle[0], titleLen);
        wTitle.pop_back();
        
        SetWindowTextW(m_hwnd, wTitle.c_str());
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void WindowsModernDialog::SetMessage(const std::string& message) {
    m_messageText = message;
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void WindowsModernDialog::AddButton(const std::string& text, DialogResult result) {
    DialogButton button;
    button.text = text;
    button.result = result;
    button.rect = D2D1::RectF(0, 0, 0, 0); // Will be calculated in layout
    m_buttons.push_back(button);
    
    if (m_hwnd) {
        CalculateLayout();
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void WindowsModernDialog::ClearButtons() {
    m_buttons.clear();
    m_hoveredButton = -1;
    m_pressedButton = -1;
    
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

DialogResult WindowsModernDialog::ShowModal() {
    // Use native Windows dialog if enabled
    if (m_useNativeDialog) {
        // Convert strings to wide strings for Windows API
        std::wstring wTitle(m_titleText.begin(), m_titleText.end());
        std::wstring wMessage(m_messageText.begin(), m_messageText.end());
        
        // Determine message box type based on buttons
        UINT uType = MB_OK;
        if (m_buttons.size() == 2) {
            // Check if we have Yes/No or OK/Cancel buttons
            bool hasYes = false, hasNo = false, hasOK = false, hasCancel = false;
            for (const auto& button : m_buttons) {
                if (button.result == DialogResult::Yes) hasYes = true;
                else if (button.result == DialogResult::No) hasNo = true;
                else if (button.result == DialogResult::OK) hasOK = true;
                else if (button.result == DialogResult::Cancel) hasCancel = true;
            }
            
            if (hasYes && hasNo) uType = MB_YESNO;
            else if (hasOK && hasCancel) uType = MB_OKCANCEL;
        }
        
        // Show native message box
        int result = MessageBoxW(m_parentHwnd, wMessage.c_str(), wTitle.c_str(), uType | MB_ICONINFORMATION);
        
        // Convert result back to DialogResult
        switch (result) {
            case IDOK: return DialogResult::OK;
            case IDCANCEL: return DialogResult::Cancel;
            case IDYES: return DialogResult::Yes;
            case IDNO: return DialogResult::No;
            default: return DialogResult::None;
        }
    }
    
    // Use custom D2D dialog
    if (!m_hwnd) return DialogResult::None;
    
    Show();
    
    m_result = DialogResult::None;
    
    // Message loop
    MSG msg;
    while (m_result == DialogResult::None && GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.hwnd == m_hwnd || IsChild(m_hwnd, msg.hwnd)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    Hide();
    return m_result;
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

void WindowsModernDialog::OnMouseMove(int x, int y) {
    int oldHovered = m_hoveredButton;
    m_hoveredButton = -1;
    
    for (size_t i = 0; i < m_buttons.size(); i++) {
        if (x >= m_buttons[i].rect.left && x <= m_buttons[i].rect.right &&
            y >= m_buttons[i].rect.top && y <= m_buttons[i].rect.bottom) {
            m_hoveredButton = static_cast<int>(i);
            break;
        }
    }
    
    if (oldHovered != m_hoveredButton) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
        
        // Update cursor
        SetCursor(LoadCursor(nullptr, m_hoveredButton >= 0 ? IDC_HAND : IDC_ARROW));
    }
}

void WindowsModernDialog::OnMouseDown(int x, int y) {
    m_pressedButton = -1;
    
    for (size_t i = 0; i < m_buttons.size(); i++) {
        if (x >= m_buttons[i].rect.left && x <= m_buttons[i].rect.right &&
            y >= m_buttons[i].rect.top && y <= m_buttons[i].rect.bottom) {
            m_pressedButton = static_cast<int>(i);
            InvalidateRect(m_hwnd, nullptr, FALSE);
            break;
        }
    }
}

void WindowsModernDialog::OnMouseUp(int x, int y) {
    if (m_pressedButton >= 0) {
        // Check if mouse is still over the pressed button
        if (x >= m_buttons[m_pressedButton].rect.left && x <= m_buttons[m_pressedButton].rect.right &&
            y >= m_buttons[m_pressedButton].rect.top && y <= m_buttons[m_pressedButton].rect.bottom) {
            m_result = m_buttons[m_pressedButton].result;
        }
        
        m_pressedButton = -1;
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void WindowsModernDialog::UpdateAnimation() {
    DWORD currentTime = GetTickCount();
    float deltaTime = (currentTime - m_lastAnimationTime) / 1000.0f;
    m_lastAnimationTime = currentTime;
    
    m_animationProgress += deltaTime * 3.0f;
    if (m_animationProgress > 2.0f * 3.14159f) {
        m_animationProgress -= 2.0f * 3.14159f;
    }
}

void WindowsModernDialog::CalculateLayout() {
    if (!m_pRenderTarget) return;
    
    D2D1_SIZE_F size = m_pRenderTarget->GetSize();
    
    // Calculate button layout
    if (!m_buttons.empty()) {
        float buttonWidth = 100.0f;
        float buttonHeight = 32.0f;
        float buttonSpacing = 12.0f;
        float totalWidth = m_buttons.size() * buttonWidth + (m_buttons.size() - 1) * buttonSpacing;
        float startX = (size.width - totalWidth) / 2;
        float buttonY = size.height - 60;
        
        for (size_t i = 0; i < m_buttons.size(); i++) {
            float x = startX + i * (buttonWidth + buttonSpacing);
            m_buttons[i].rect = D2D1::RectF(x, buttonY, x + buttonWidth, buttonY + buttonHeight);
        }
    }
}

void WindowsModernDialog::DrawBackground() {
    D2D1_SIZE_F size = m_pRenderTarget->GetSize();
    
    // Clear with transparent background
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    
    // Draw rounded rectangle background with subtle shadow effect
    D2D1_ROUNDED_RECT shadowRect = D2D1::RoundedRect(
        D2D1::RectF(12, 12, size.width - 8, size.height - 8),
        16, 16
    );
    
    ID2D1SolidColorBrush* shadowBrush;
    m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.3f), &shadowBrush);
    m_pRenderTarget->FillRoundedRectangle(shadowRect, shadowBrush);
    shadowBrush->Release();
    
    // Main background
    D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
        D2D1::RectF(8, 8, size.width - 8, size.height - 8),
        16, 16
    );
    
    m_pRenderTarget->FillRoundedRectangle(roundedRect, m_pBackgroundBrush);
    
    // Border
    m_pRenderTarget->DrawRoundedRectangle(roundedRect, m_pBorderBrush, 1.0f);
}

void WindowsModernDialog::DrawTitle() {
    if (m_titleText.empty()) return;
    
    D2D1_SIZE_F size = m_pRenderTarget->GetSize();
    
    int titleLen = MultiByteToWideChar(CP_UTF8, 0, m_titleText.c_str(), -1, nullptr, 0);
    std::wstring wTitle(titleLen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, m_titleText.c_str(), -1, &wTitle[0], titleLen);
    wTitle.pop_back();
    
    D2D1_RECT_F titleRect = D2D1::RectF(30, 30, size.width - 30, 70);
    m_pRenderTarget->DrawTextW(wTitle.c_str(), static_cast<UINT32>(wTitle.length()), m_pTitleFormat, titleRect, m_pTextBrush);
}

void WindowsModernDialog::DrawMessage() {
    if (m_messageText.empty()) return;
    
    D2D1_SIZE_F size = m_pRenderTarget->GetSize();
    
    int messageLen = MultiByteToWideChar(CP_UTF8, 0, m_messageText.c_str(), -1, nullptr, 0);
    std::wstring wMessage(messageLen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, m_messageText.c_str(), -1, &wMessage[0], messageLen);
    wMessage.pop_back();
    
    D2D1_RECT_F messageRect = D2D1::RectF(30, 80, size.width - 30, size.height - 100);
    m_pRenderTarget->DrawTextW(wMessage.c_str(), static_cast<UINT32>(wMessage.length()), m_pTextFormat, messageRect, m_pSecondaryTextBrush);
}

void WindowsModernDialog::DrawButtons() {
    for (size_t i = 0; i < m_buttons.size(); i++) {
        const DialogButton& button = m_buttons[i];
        
        // Choose brush based on state
        ID2D1SolidColorBrush* backgroundBrush = m_pButtonBrush;
        ID2D1SolidColorBrush* textBrush = m_pTextBrush;
        
        if (static_cast<int>(i) == m_pressedButton) {
            backgroundBrush = m_pAccentBrush;
        } else if (static_cast<int>(i) == m_hoveredButton) {
            backgroundBrush = m_pButtonHoverBrush;
        }
        
        // Draw button background
        D2D1_ROUNDED_RECT buttonRect = D2D1::RoundedRect(button.rect, 6, 6);
        m_pRenderTarget->FillRoundedRectangle(buttonRect, backgroundBrush);
        m_pRenderTarget->DrawRoundedRectangle(buttonRect, m_pBorderBrush, 1.0f);
        
        // Draw button text
        int textLen = MultiByteToWideChar(CP_UTF8, 0, button.text.c_str(), -1, nullptr, 0);
        std::wstring wText(textLen, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, button.text.c_str(), -1, &wText[0], textLen);
        wText.pop_back();
        
        m_pRenderTarget->DrawTextW(wText.c_str(), static_cast<UINT32>(wText.length()), m_pButtonFormat, button.rect, textBrush);
    }
}

void WindowsModernDialog::SetProgress(int percentage) {
    m_progress = std::clamp(percentage, 0, 100);
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void WindowsModernDialog::SetStatus(const std::string& status) {
    m_statusText = status;
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void WindowsModernDialog::SetDownloadInfo(size_t bytesDownloaded, size_t totalBytes, size_t speed) {
    m_bytesDownloaded = bytesDownloaded;
    m_totalBytes = totalBytes;
    m_downloadSpeed = speed;
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void WindowsModernDialog::UpdateProgress(int percentage, const std::wstring& status, size_t bytesDownloaded, size_t totalBytes) {
    m_progress = std::clamp(percentage, 0, 100);
    
    // Convert wide string status to string
    int statusLen = WideCharToMultiByte(CP_UTF8, 0, status.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string statusStr(statusLen, '\0');
    WideCharToMultiByte(CP_UTF8, 0, status.c_str(), -1, &statusStr[0], statusLen, nullptr, nullptr);
    statusStr.pop_back();
    
    m_statusText = statusStr;
    m_bytesDownloaded = bytesDownloaded;
    m_totalBytes = totalBytes;
    
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

#endif // _WIN32