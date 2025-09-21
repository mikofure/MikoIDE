#pragma once

#include <vector>
#include <string>
#include <memory>

// RGB color structure for 24-bit colors
struct RGBColor {
    uint8_t r, g, b;
    
    RGBColor() : r(255), g(255), b(255) {} // Default white
    RGBColor(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
    RGBColor(int ansiColor); // Constructor for ANSI color conversion
    
    // Convert to D2D1_COLOR_F for DirectWrite
    void ToD2DColor(float& red, float& green, float& blue, float alpha = 1.0f) const {
        red = r / 255.0f;
        green = g / 255.0f;
        blue = b / 255.0f;
    }
};

// Font weight enumeration
enum class FontWeight {
    Normal = 400,
    Bold = 700,
    Light = 300,
    SemiBold = 600,
    ExtraBold = 800,
    Black = 900
};

struct TerminalCell {
    char character;
    RGBColor foregroundColor;  // 24-bit RGB foreground color
    RGBColor backgroundColor;  // 24-bit RGB background color
    FontWeight fontWeight;     // Font weight (normal, bold, etc.)
    bool underline;           // Underline attribute
    bool italic;              // Italic attribute
    bool strikethrough;       // Strikethrough attribute
    
    TerminalCell() 
        : character(' ')
        , foregroundColor(192, 192, 192)  // Light gray
        , backgroundColor(0, 0, 0)        // Black
        , fontWeight(FontWeight::Normal)
        , underline(false)
        , italic(false)
        , strikethrough(false) {}
        
    TerminalCell(char c) 
        : character(c)
        , foregroundColor(192, 192, 192)  // Light gray
        , backgroundColor(0, 0, 0)        // Black
        , fontWeight(FontWeight::Normal)
        , underline(false)
        , italic(false)
        , strikethrough(false) {}
};

class TerminalBuffer {
public:
    TerminalBuffer();
    ~TerminalBuffer();

    void Initialize(int cols, int rows);
    void Resize(int cols, int rows);
    
    void AppendOutput(const std::string& output);
    void ProcessCharacter(char c);
    
    void MoveCursor(int x, int y);
    void MoveCursorRelative(int dx, int dy);
    
    std::vector<std::string> GetLines() const;
    std::vector<std::vector<TerminalCell>> GetBuffer() const;
    std::pair<int, int> GetCursorPosition() const;
    
    void Clear();
    void ClearLine(int line);
    
    void ScrollUp(int lines = 1);
    
    // Prompt protection methods
    void SetPromptEnd(int x, int y);  // Set where the prompt ends (user input starts)
    void ResetPromptProtection();     // Reset prompt protection

private:
    void ProcessEscapeSequence(const std::string& sequence);
    void ProcessSGRSequence(const std::string& params);
    void EnsureCursorInBounds();
    void NewLine();
    void CarriageReturn();
    void Tab();
    void Backspace();
    
    std::vector<std::vector<TerminalCell>> m_buffer;
    int m_cols;
    int m_rows;
    int m_cursorX;
    int m_cursorY;
    
    // Current text attributes (enhanced for 24-bit colors)
    RGBColor m_currentForeground;
    RGBColor m_currentBackground;
    FontWeight m_currentFontWeight;
    bool m_currentUnderline;
    bool m_currentItalic;
    bool m_currentStrikethrough;
    
    // For escape sequence processing
    std::string m_escapeBuffer;
    bool m_inEscapeSequence;
    
    // Scrolling
    int m_scrollTop;
    int m_scrollBottom;
    
    // Prompt protection
    int m_promptEndX;  // X position where prompt ends (where user input starts)
    int m_promptEndY;  // Y position of the prompt line
};