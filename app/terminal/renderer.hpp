#pragma once

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <SDL3/SDL.h>
#include <string>
#include <memory>
#include <unordered_map>

class TerminalBuffer;
struct RGBColor;

class DirectWriteRenderer {
public:
    DirectWriteRenderer();
    ~DirectWriteRenderer();

    bool Initialize(SDL_Window* window);
    void Shutdown();
    
    void RenderTerminal(const TerminalBuffer& buffer);
    void OnResize(int width, int height);
    
    std::pair<int, int> GetCharacterSize() const;

private:
    bool CreateDeviceResources();
    void ReleaseDeviceResources();
    bool CreateTextFormat();
    void RenderText(const std::wstring& text, float x, float y, const RGBColor& foreground, 
                   const RGBColor& background, bool bold = false, bool italic = false, 
                   bool underline = false, bool strikethrough = false);
    
    // Color management for 24-bit colors
    ID2D1Brush* GetOrCreateBrush(const RGBColor& color);
    void ClearBrushCache();

    // DirectWrite/Direct2D interfaces
    ID2D1Factory* m_d2dFactory;
    IDWriteFactory* m_writeFactory;
    ID2D1HwndRenderTarget* m_renderTarget;
    IDWriteTextFormat* m_textFormat;
    IDWriteTextFormat* m_boldTextFormat;
    IDWriteTextFormat* m_italicTextFormat;
    IDWriteTextFormat* m_boldItalicTextFormat;
    
    // Brushes
    ID2D1SolidColorBrush* m_textBrush;
    ID2D1SolidColorBrush* m_backgroundBrush;
    ID2D1SolidColorBrush* m_cursorBrush;
    
    // Dynamic brush cache for 24-bit colors
    std::unordered_map<uint32_t, ID2D1SolidColorBrush*> m_brushCache;
    
    // Window handle
    HWND m_hwnd;
    
    // Font metrics
    float m_charWidth;
    float m_charHeight;
    
    // Settings
    std::wstring m_fontName;
    float m_fontSize;
};