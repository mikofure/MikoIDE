#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

class TerminalBuffer;
struct RGBColor;

#ifdef _WIN32
#include <d2d1.h>
#include <dwrite.h>
#include <windows.h>

class DirectWriteRenderer {
public:
  DirectWriteRenderer();
  ~DirectWriteRenderer();

  bool Initialize(SDL_Window *window);
  void Shutdown();

  void RenderTerminal(const TerminalBuffer &buffer);
  void OnResize(int width, int height);

  std::pair<int, int> GetCharacterSize() const;

private:
  bool CreateDeviceResources();
  void ReleaseDeviceResources();
  bool CreateTextFormat();
  void RenderText(const std::wstring &text, float x, float y,
                  const RGBColor &foreground, const RGBColor &background,
                  bool bold = false, bool italic = false,
                  bool underline = false, bool strikethrough = false);

  ID2D1Brush *GetOrCreateBrush(const RGBColor &color);
  void ClearBrushCache();

  ID2D1Factory *m_d2dFactory;
  IDWriteFactory *m_writeFactory;
  ID2D1HwndRenderTarget *m_renderTarget;
  IDWriteTextFormat *m_textFormat;
  IDWriteTextFormat *m_boldTextFormat;
  IDWriteTextFormat *m_italicTextFormat;
  IDWriteTextFormat *m_boldItalicTextFormat;

  ID2D1SolidColorBrush *m_textBrush;
  ID2D1SolidColorBrush *m_backgroundBrush;
  ID2D1SolidColorBrush *m_cursorBrush;

  std::unordered_map<uint32_t, ID2D1SolidColorBrush *> m_brushCache;

  HWND m_hwnd;

  float m_charWidth;
  float m_charHeight;

  std::wstring m_fontName;
  float m_fontSize;
};
#else

class DirectWriteRenderer {
public:
  DirectWriteRenderer();
  ~DirectWriteRenderer();

  bool Initialize(SDL_Window *window);
  void Shutdown();

  void RenderTerminal(const TerminalBuffer &buffer);
  void OnResize(int width, int height);

  std::pair<int, int> GetCharacterSize() const;

private:
  std::pair<int, int> char_size_;
};
#endif

