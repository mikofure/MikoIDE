#include "renderer.hpp"
#include "terminalbuffer.hpp"
#include <SDL3/SDL.h>

#ifdef _WIN32
#include <iostream>

DirectWriteRenderer::DirectWriteRenderer()
    : m_d2dFactory(nullptr), m_writeFactory(nullptr), m_renderTarget(nullptr),
      m_textFormat(nullptr), m_boldTextFormat(nullptr),
      m_italicTextFormat(nullptr), m_boldItalicTextFormat(nullptr),
      m_textBrush(nullptr), m_backgroundBrush(nullptr), m_cursorBrush(nullptr),
      m_hwnd(nullptr), m_charWidth(8.0f), m_charHeight(16.0f),
      m_fontName(L"JetBrains Mono"), m_fontSize(14.0f) {}

DirectWriteRenderer::~DirectWriteRenderer() { Shutdown(); }

void DirectWriteRenderer::Shutdown() {
  ReleaseDeviceResources();
  ClearBrushCache();

  if (m_textFormat) {
    m_textFormat->Release();
    m_textFormat = nullptr;
  }
  if (m_boldTextFormat) {
    m_boldTextFormat->Release();
    m_boldTextFormat = nullptr;
  }
  if (m_italicTextFormat) {
    m_italicTextFormat->Release();
    m_italicTextFormat = nullptr;
  }
  if (m_boldItalicTextFormat) {
    m_boldItalicTextFormat->Release();
    m_boldItalicTextFormat = nullptr;
  }
  if (m_writeFactory) {
    m_writeFactory->Release();
    m_writeFactory = nullptr;
  }
  if (m_d2dFactory) {
    m_d2dFactory->Release();
    m_d2dFactory = nullptr;
  }
}

bool DirectWriteRenderer::Initialize(SDL_Window *window) {
  // Get the window handle from SDL
  SDL_PropertiesID props = SDL_GetWindowProperties(window);
  m_hwnd = (HWND)SDL_GetPointerProperty(
      props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

  // Create Direct2D factory
  HRESULT hr =
      D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_d2dFactory);
  if (FAILED(hr)) {
    std::cerr << "Failed to create D2D factory" << std::endl;
    return false;
  }

  // Create DirectWrite factory
  hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                           reinterpret_cast<IUnknown **>(&m_writeFactory));
  if (FAILED(hr)) {
    std::cerr << "Failed to create DirectWrite factory" << std::endl;
    return false;
  }

  if (!CreateDeviceResources()) {
    return false;
  }

  if (!CreateTextFormat()) {
    return false;
  }

  return true;
}

bool DirectWriteRenderer::CreateDeviceResources() {
  if (m_renderTarget) {
    return true; // Already created
  }

  RECT rc;
  GetClientRect(m_hwnd, &rc);

  D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

  HRESULT hr = m_d2dFactory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(m_hwnd, size), &m_renderTarget);

  if (FAILED(hr)) {
    std::cerr << "Failed to create render target" << std::endl;
    return false;
  }

  // Create brushes using Hyperion theme colors
  // Text color: #FFFFFF (white foreground)
  hr = m_renderTarget->CreateSolidColorBrush(
      D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), // #FFFFFF
      &m_textBrush);
  if (FAILED(hr)) {
    std::cerr << "Failed to create text brush" << std::endl;
    return false;
  }

  // Background color: #1E1E1E (dark background)
  hr = m_renderTarget->CreateSolidColorBrush(
      D2D1::ColorF(0.118f, 0.118f, 0.118f, 1.0f), // #1E1E1E
      &m_backgroundBrush);
  if (FAILED(hr)) {
    std::cerr << "Failed to create background brush" << std::endl;
    return false;
  }

  // Cursor color: #FFFFFF (white cursor)
  hr = m_renderTarget->CreateSolidColorBrush(
      D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), // #FFFFFF
      &m_cursorBrush);
  if (FAILED(hr)) {
    std::cerr << "Failed to create cursor brush" << std::endl;
    return false;
  }

  return true;
}

void DirectWriteRenderer::ReleaseDeviceResources() {
  if (m_renderTarget) {
    m_renderTarget->Release();
    m_renderTarget = nullptr;
  }
  if (m_textBrush) {
    m_textBrush->Release();
    m_textBrush = nullptr;
  }
  if (m_backgroundBrush) {
    m_backgroundBrush->Release();
    m_backgroundBrush = nullptr;
  }
  if (m_cursorBrush) {
    m_cursorBrush->Release();
    m_cursorBrush = nullptr;
  }
  ClearBrushCache();
}

bool DirectWriteRenderer::CreateTextFormat() {
  HRESULT hr = m_writeFactory->CreateTextFormat(
      m_fontName.c_str(), nullptr, DWRITE_FONT_WEIGHT_REGULAR,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, m_fontSize,
      L"en-us", &m_textFormat);

  if (FAILED(hr)) {
    std::cerr << "Failed to create text format" << std::endl;
    return false;
  }

  // Create additional formats for attributes
  hr = m_writeFactory->CreateTextFormat(
      m_fontName.c_str(), nullptr, DWRITE_FONT_WEIGHT_BOLD,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, m_fontSize,
      L"en-us", &m_boldTextFormat);

  if (FAILED(hr)) {
    std::cerr << "Failed to create bold text format" << std::endl;
    return false;
  }

  hr = m_writeFactory->CreateTextFormat(
      m_fontName.c_str(), nullptr, DWRITE_FONT_WEIGHT_REGULAR,
      DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL, m_fontSize,
      L"en-us", &m_italicTextFormat);

  if (FAILED(hr)) {
    std::cerr << "Failed to create italic text format" << std::endl;
    return false;
  }

  hr = m_writeFactory->CreateTextFormat(
      m_fontName.c_str(), nullptr, DWRITE_FONT_WEIGHT_BOLD,
      DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL, m_fontSize,
      L"en-us", &m_boldItalicTextFormat);

  if (FAILED(hr)) {
    std::cerr << "Failed to create bold italic text format" << std::endl;
    return false;
  }

  return true;
}

void DirectWriteRenderer::RenderTerminal(const TerminalBuffer &buffer) {
  if (!m_renderTarget || !m_textFormat) {
    return;
  }

  m_renderTarget->BeginDraw();

  // Clear with background color (dark theme)
  m_renderTarget->Clear(D2D1::ColorF(0.07f, 0.07f, 0.07f, 1.0f));

  const auto &rows = buffer.GetBuffer();

  for (size_t row = 0; row < rows.size(); ++row) {
    const auto &cells = rows[row];
    float y = static_cast<float>(row) * m_charHeight;

    for (size_t col = 0; col < cells.size(); ++col) {
      const auto &cell = cells[col];
      float x = static_cast<float>(col) * m_charWidth;

      wchar_t wideChar = static_cast<wchar_t>(
          static_cast<unsigned char>(cell.character)); // basic ASCII support
      std::wstring text(1, wideChar);

      const bool isBold = cell.fontWeight == FontWeight::Bold ||
                          cell.fontWeight == FontWeight::SemiBold ||
                          cell.fontWeight == FontWeight::ExtraBold ||
                          cell.fontWeight == FontWeight::Black;

      RenderText(text, x, y, cell.foregroundColor, cell.backgroundColor,
                 isBold, cell.italic, cell.underline, cell.strikethrough);
    }
  }

  HRESULT hr = m_renderTarget->EndDraw();

  if (hr == D2DERR_RECREATE_TARGET) {
    ReleaseDeviceResources();
  }

  if (m_hwnd) {
    InvalidateRect(m_hwnd, nullptr, FALSE);
  }
}

void DirectWriteRenderer::RenderText(const std::wstring &text, float x, float y,
                                     const RGBColor &fgColor,
                                     const RGBColor &bgColor, bool bold,
                                     bool italic, bool underline,
                                     bool strikethrough) {
  if (!m_renderTarget || !m_textFormat || text.empty()) {
    return;
  }

  ID2D1Brush *fgBrush = GetOrCreateBrush(fgColor);
  ID2D1Brush *bgBrush = GetOrCreateBrush(bgColor);

  if (!fgBrush || !bgBrush) {
    return;
  }

  IDWriteTextFormat *textFormat = m_textFormat;
  if (bold && italic) {
    textFormat = m_boldItalicTextFormat;
  } else if (bold) {
    textFormat = m_boldTextFormat;
  } else if (italic) {
    textFormat = m_italicTextFormat;
  }

  if (bgColor.r != 0 || bgColor.g != 0 || bgColor.b != 0) {
    D2D1_RECT_F bgRect =
        D2D1::RectF(x, y, x + static_cast<float>(text.length()) * m_charWidth,
                    y + m_charHeight);
    m_renderTarget->FillRectangle(bgRect, bgBrush);
  }

  IDWriteTextLayout *textLayout = nullptr;
  HRESULT hr = m_writeFactory->CreateTextLayout(
      text.c_str(), static_cast<UINT32>(text.length()), textFormat,
      m_charWidth * text.length() + 1.0f, m_charHeight + 1.0f, &textLayout);

  if (SUCCEEDED(hr)) {
    if (underline) {
      textLayout->SetUnderline(
          TRUE, DWRITE_TEXT_RANGE{0, static_cast<UINT32>(text.length())});
    }
    if (strikethrough) {
      textLayout->SetStrikethrough(
          TRUE, DWRITE_TEXT_RANGE{0, static_cast<UINT32>(text.length())});
    }

    D2D1_POINT_2F origin = D2D1::Point2F(x, y);
    m_renderTarget->DrawTextLayout(origin, textLayout, fgBrush);

    textLayout->Release();
  } else {
    D2D1_RECT_F layoutRect = D2D1::RectF(
        x, y, x + static_cast<float>(text.length()) * m_charWidth + 1.0f,
        y + m_charHeight + 1.0f);

    m_renderTarget->DrawText(text.c_str(), static_cast<UINT32>(text.length()),
                             textFormat, layoutRect, fgBrush);
  }
}

void DirectWriteRenderer::OnResize(int width, int height) {
  if (m_renderTarget) {
    D2D1_SIZE_U size = D2D1::SizeU(width, height);
    m_renderTarget->Resize(size);
  }
}

std::pair<int, int> DirectWriteRenderer::GetCharacterSize() const {
  return {static_cast<int>(m_charWidth), static_cast<int>(m_charHeight)};
}

ID2D1Brush *DirectWriteRenderer::GetOrCreateBrush(const RGBColor &color) {
  if (!m_renderTarget) {
    return nullptr;
  }

  uint32_t colorKey = (static_cast<uint32_t>(color.r) << 16) |
                      (static_cast<uint32_t>(color.g) << 8) |
                      static_cast<uint32_t>(color.b);

  auto it = m_brushCache.find(colorKey);
  if (it != m_brushCache.end()) {
    return it->second;
  }

  ID2D1SolidColorBrush *brush = nullptr;
  D2D1_COLOR_F d2dColor =
      D2D1::ColorF(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, 1.0f);

  HRESULT hr = m_renderTarget->CreateSolidColorBrush(d2dColor, &brush);
  if (SUCCEEDED(hr)) {
    m_brushCache[colorKey] = brush;
    return brush;
  }

  return nullptr;
}

void DirectWriteRenderer::ClearBrushCache() {
  for (auto &pair : m_brushCache) {
    if (pair.second) {
      pair.second->Release();
    }
  }
  m_brushCache.clear();
}

#else

DirectWriteRenderer::DirectWriteRenderer() : char_size_{8, 16} {}
DirectWriteRenderer::~DirectWriteRenderer() = default;

bool DirectWriteRenderer::Initialize(SDL_Window *) { return false; }
void DirectWriteRenderer::Shutdown() {}

void DirectWriteRenderer::RenderTerminal(const TerminalBuffer &) {}
void DirectWriteRenderer::OnResize(int, int) {}

std::pair<int, int> DirectWriteRenderer::GetCharacterSize() const {
  return char_size_;
}

#endif
