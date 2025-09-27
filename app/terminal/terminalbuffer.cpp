#include "terminalbuffer.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

// ANSI color conversion constructor
RGBColor::RGBColor(int ansiColor) {
  // Standard ANSI colors (0-15)
  static const uint8_t ansiColorTable[16][3] = {
      {0, 0, 0},       // 0: Black
      {128, 0, 0},     // 1: Dark Red
      {0, 128, 0},     // 2: Dark Green
      {128, 128, 0},   // 3: Dark Yellow
      {0, 0, 128},     // 4: Dark Blue
      {128, 0, 128},   // 5: Dark Magenta
      {0, 128, 128},   // 6: Dark Cyan
      {192, 192, 192}, // 7: Light Gray
      {128, 128, 128}, // 8: Dark Gray
      {255, 0, 0},     // 9: Red
      {0, 255, 0},     // 10: Green
      {255, 255, 0},   // 11: Yellow
      {0, 0, 255},     // 12: Blue
      {255, 0, 255},   // 13: Magenta
      {0, 255, 255},   // 14: Cyan
      {255, 255, 255}  // 15: White
  };

  if (ansiColor >= 0 && ansiColor <= 15) {
    r = ansiColorTable[ansiColor][0];
    g = ansiColorTable[ansiColor][1];
    b = ansiColorTable[ansiColor][2];
  } else {
    // Default to white for invalid colors
    r = g = b = 255;
  }
}

TerminalBuffer::TerminalBuffer()
    : m_cols(80), m_rows(25), m_cursorX(0), m_cursorY(0),
      m_currentForeground(192, 192, 192), m_currentBackground(0, 0, 0),
      m_currentFontWeight(FontWeight::Normal), m_currentUnderline(false),
      m_currentItalic(false), m_currentStrikethrough(false),
      m_inEscapeSequence(false), m_scrollTop(0), m_scrollBottom(24),
      m_promptEndX(-1), m_promptEndY(-1) {}

TerminalBuffer::~TerminalBuffer() {}

void TerminalBuffer::Initialize(int cols, int rows) {
  m_cols = cols;
  m_rows = rows;
  m_scrollBottom = rows - 1;

  m_buffer.clear();
  m_buffer.resize(rows);

  for (auto &row : m_buffer) {
    row.resize(cols);
  }

  m_cursorX = 0;
  m_cursorY = 0;
}

void TerminalBuffer::Resize(int cols, int rows) {
  // Save current content
  auto oldBuffer = m_buffer;
  int oldCols = m_cols;
  int oldRows = m_rows;

  // Resize buffer
  m_cols = cols;
  m_rows = rows;
  m_scrollBottom = rows - 1;

  m_buffer.clear();
  m_buffer.resize(rows);

  for (auto &row : m_buffer) {
    row.resize(cols);
  }

  // Copy old content
  int copyRows = std::min(oldRows, rows);
  int copyCols = std::min(oldCols, cols);

  for (int row = 0; row < copyRows; ++row) {
    for (int col = 0; col < copyCols; ++col) {
      m_buffer[row][col] = oldBuffer[row][col];
    }
  }

  // Adjust cursor position
  m_cursorX = std::min(m_cursorX, cols - 1);
  m_cursorY = std::min(m_cursorY, rows - 1);
}

void TerminalBuffer::AppendOutput(const std::string &output) {
  for (char c : output) {
    ProcessCharacter(c);
  }
}

void TerminalBuffer::ProcessCharacter(char c) {
  // Handle escape sequences
  if (m_inEscapeSequence) {
    m_escapeBuffer += c;

    // Check if sequence is complete
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
      ProcessEscapeSequence(m_escapeBuffer);
      m_escapeBuffer.clear();
      m_inEscapeSequence = false;
    }
    return;
  }

  // Start of escape sequence
  if (c == '\x1b') {
    m_inEscapeSequence = true;
    m_escapeBuffer = c;
    return;
  }

  // Handle special characters
  switch (c) {
  case '\r':
    CarriageReturn();
    break;

  case '\n':
    NewLine();
    break;

  case '\t':
    Tab();
    break;

  case '\b': // backspace
    Backspace();
    break;

  case '\a': // Bell - ignore for now
    break;

  default:
    // Regular character
    if (c >= 32 && c <= 126) { // Printable ASCII
      // Check if we need to wrap to next line
      if (m_cursorX >= m_cols) {
        NewLine();
        CarriageReturn();
      }

      // Place character at current position
      if (m_cursorY < m_rows && m_cursorX < m_cols) {
        m_buffer[m_cursorY][m_cursorX].character = c;
        m_buffer[m_cursorY][m_cursorX].foregroundColor = m_currentForeground;
        m_buffer[m_cursorY][m_cursorX].backgroundColor = m_currentBackground;
        m_buffer[m_cursorY][m_cursorX].fontWeight = m_currentFontWeight;
        m_buffer[m_cursorY][m_cursorX].underline = m_currentUnderline;
        m_buffer[m_cursorY][m_cursorX].italic = m_currentItalic;
        m_buffer[m_cursorY][m_cursorX].strikethrough = m_currentStrikethrough;
        m_cursorX++;

        // If we've reached the end of the line, prepare for next line
        if (m_cursorX >= m_cols) {
          // Don't move to next line yet, wait for next character
          // This allows proper handling of exact line-width text
        }
      }
    }
    break;
  }

  EnsureCursorInBounds();
}

void TerminalBuffer::ProcessEscapeSequence(const std::string &sequence) {
  if (sequence.empty())
    return;

  // Handle CSI sequences (Control Sequence Introducer)
  if (sequence[0] == '[') {
    std::string params = sequence.substr(1, sequence.length() - 2);
    char command = sequence.back();

    switch (command) {
    case 'm': // SGR (Select Graphic Rendition) - colors and text attributes
      ProcessSGRSequence(params);
      break;

    case 'H': // Cursor Position
    case 'f': {
      std::istringstream iss(params);
      std::string token;
      int row = 1, col = 1;

      if (std::getline(iss, token, ';')) {
        if (!token.empty())
          row = std::stoi(token);
      }
      if (std::getline(iss, token, ';')) {
        if (!token.empty())
          col = std::stoi(token);
      }

      MoveCursor(col - 1, row - 1);
      break;
    }

    case 'A': // Cursor Up
      if (!params.empty()) {
        int n = std::stoi(params);
        MoveCursorRelative(0, -n);
      } else {
        MoveCursorRelative(0, -1);
      }
      break;

    case 'B': // Cursor Down
      if (!params.empty()) {
        int n = std::stoi(params);
        MoveCursorRelative(0, n);
      } else {
        MoveCursorRelative(0, 1);
      }
      break;

    case 'C': // Cursor Forward
      if (!params.empty()) {
        int n = std::stoi(params);
        MoveCursorRelative(n, 0);
      } else {
        MoveCursorRelative(1, 0);
      }
      break;

    case 'D': // Cursor Backward
      if (!params.empty()) {
        int n = std::stoi(params);
        MoveCursorRelative(-n, 0);
      } else {
        MoveCursorRelative(-1, 0);
      }
      break;

    case 'J': // Erase Display
      if (params.empty() || params == "0") {
        // Clear from cursor to end of screen
        for (int y = m_cursorY; y < m_rows; y++) {
          int startX = (y == m_cursorY) ? m_cursorX : 0;
          for (int x = startX; x < m_cols; x++) {
            m_buffer[y][x] = TerminalCell();
          }
        }
      } else if (params == "1") {
        // Clear from beginning of screen to cursor
        for (int y = 0; y <= m_cursorY; y++) {
          int endX = (y == m_cursorY) ? m_cursorX : m_cols - 1;
          for (int x = 0; x <= endX; x++) {
            m_buffer[y][x] = TerminalCell();
          }
        }
      } else if (params == "2") {
        // Clear entire screen
        Clear();
      }
      break;

    case 'K': // Erase Line
      if (params.empty() || params == "0") {
        // Clear from cursor to end of line
        for (int x = m_cursorX; x < m_cols; x++) {
          m_buffer[m_cursorY][x] = TerminalCell();
        }
      } else if (params == "1") {
        // Clear from beginning of line to cursor
        for (int x = 0; x <= m_cursorX; x++) {
          m_buffer[m_cursorY][x] = TerminalCell();
        }
      } else if (params == "2") {
        // Clear entire line
        ClearLine(m_cursorY);
      }
      break;
    }
  }
}

void TerminalBuffer::ProcessSGRSequence(const std::string &params) {
  if (params.empty()) {
    // Reset all attributes
    m_currentForeground = RGBColor(192, 192, 192); // Light gray
    m_currentBackground = RGBColor(0, 0, 0);       // Black
    m_currentFontWeight = FontWeight::Normal;
    m_currentUnderline = false;
    m_currentItalic = false;
    m_currentStrikethrough = false;
    return;
  }

  std::istringstream iss(params);
  std::string token;

  while (std::getline(iss, token, ';')) {
    if (token.empty())
      continue;

    int code = std::stoi(token);

    switch (code) {
    case 0: // Reset
      m_currentForeground = RGBColor(192, 192, 192);
      m_currentBackground = RGBColor(0, 0, 0);
      m_currentFontWeight = FontWeight::Normal;
      m_currentUnderline = false;
      m_currentItalic = false;
      m_currentStrikethrough = false;
      break;

    case 1: // Bold
      m_currentFontWeight = FontWeight::Bold;
      break;

    case 2: // Dim/Faint
      m_currentFontWeight = FontWeight::Light;
      break;

    case 3: // Italic
      m_currentItalic = true;
      break;

    case 4: // Underline
      m_currentUnderline = true;
      break;

    case 9: // Strikethrough
      m_currentStrikethrough = true;
      break;

    case 22: // Normal intensity (not bold/dim)
      m_currentFontWeight = FontWeight::Normal;
      break;

    case 23: // Not italic
      m_currentItalic = false;
      break;

    case 24: // Not underlined
      m_currentUnderline = false;
      break;

    case 29: // Not strikethrough
      m_currentStrikethrough = false;
      break;

    // Standard foreground colors (30-37)
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
      m_currentForeground = RGBColor(code - 30);
      break;

    // Standard background colors (40-47)
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
      m_currentBackground = RGBColor(code - 40);
      break;

    // Bright foreground colors (90-97)
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
      m_currentForeground = RGBColor(code - 90 + 8);
      break;

    // Bright background colors (100-107)
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
      m_currentBackground = RGBColor(code - 100 + 8);
      break;

    // 24-bit RGB foreground color (38;2;r;g;b)
    case 38: {
      std::string mode, r, g, b;
      if (std::getline(iss, mode, ';') && mode == "2" &&
          std::getline(iss, r, ';') && std::getline(iss, g, ';') &&
          std::getline(iss, b, ';')) {
        m_currentForeground =
            RGBColor(std::stoi(r), std::stoi(g), std::stoi(b));
      }
      break;
    }

    // 24-bit RGB background color (48;2;r;g;b)
    case 48: {
      std::string mode, r, g, b;
      if (std::getline(iss, mode, ';') && mode == "2" &&
          std::getline(iss, r, ';') && std::getline(iss, g, ';') &&
          std::getline(iss, b, ';')) {
        m_currentBackground =
            RGBColor(std::stoi(r), std::stoi(g), std::stoi(b));
      }
      break;
    }
    }
  }
}
void TerminalBuffer::EnsureCursorInBounds() {
  m_cursorX = std::max(0, std::min(m_cols - 1, m_cursorX));
  m_cursorY = std::max(0, std::min(m_rows - 1, m_cursorY));
}

void TerminalBuffer::NewLine() {
  // Move cursor down
  m_cursorY++;
  m_cursorX = 0; // <<<<< สำคัญ! reset column เวลาเจอ newline

  if (m_cursorY >= m_rows) {
    ScrollUp();
    m_cursorY = m_rows - 1;
  }
}

void TerminalBuffer::CarriageReturn() { m_cursorX = 0; }

void TerminalBuffer::Tab() {
  int nextTabStop = ((m_cursorX / 8) + 1) * 8;
  m_cursorX = std::min(nextTabStop, m_cols - 1);
}

void TerminalBuffer::Backspace() {
  // Check if prompt protection is active and we're at the prompt boundary
  if (m_promptEndX >= 0 && m_promptEndY >= 0) {
    // If we're on the prompt line and at or before the prompt end position,
    // ignore backspace
    if (m_cursorY == m_promptEndY && m_cursorX <= m_promptEndX) {
      return; // Protect the prompt from being deleted
    }
  }

  if (m_cursorX > 0) {
    // Move cursor back and clear the character
    m_cursorX--;
    m_buffer[m_cursorY][m_cursorX].character = ' ';
    m_buffer[m_cursorY][m_cursorX].foregroundColor = m_currentForeground;
    m_buffer[m_cursorY][m_cursorX].backgroundColor = m_currentBackground;
    m_buffer[m_cursorY][m_cursorX].fontWeight = m_currentFontWeight;
    m_buffer[m_cursorY][m_cursorX].underline = m_currentUnderline;
    m_buffer[m_cursorY][m_cursorX].italic = m_currentItalic;
    m_buffer[m_cursorY][m_cursorX].strikethrough = m_currentStrikethrough;
  }
  // Note: Backspace at beginning of line (m_cursorX == 0) is ignored
  // This prevents cursor from moving to previous line, providing protection
}

void TerminalBuffer::MoveCursor(int x, int y) {
  m_cursorX = x;
  m_cursorY = y;
  EnsureCursorInBounds();
}

void TerminalBuffer::MoveCursorRelative(int dx, int dy) {
  m_cursorX += dx;
  m_cursorY += dy;
  EnsureCursorInBounds();
}

std::vector<std::string> TerminalBuffer::GetLines() const {
  std::vector<std::string> lines;
  lines.reserve(m_rows);

  for (const auto &row : m_buffer) {
    std::string line;
    line.reserve(m_cols);

    for (const auto &cell : row) {
      line += cell.character;
    }

    // Remove trailing spaces
    while (!line.empty() && line.back() == ' ') {
      line.pop_back();
    }

    lines.push_back(line);
  }

  return lines;
}

const std::vector<std::vector<TerminalCell>> &
TerminalBuffer::GetBuffer() const {
  return m_buffer;
}

std::pair<int, int> TerminalBuffer::GetCursorPosition() const {
  return {m_cursorX, m_cursorY};
}

void TerminalBuffer::Clear() {
  for (auto &row : m_buffer) {
    for (auto &cell : row) {
      cell.character = ' ';
    }
  }
}

void TerminalBuffer::ClearLine(int line) {
  if (line >= 0 && line < m_rows) {
    for (auto &cell : m_buffer[line]) {
      cell.character = ' ';
    }
  }
}

void TerminalBuffer::ScrollUp(int lines) {
  if (lines <= 0)
    return;

  // Move lines up
  for (int row = 0; row < m_rows - lines; ++row) {
    m_buffer[row] = m_buffer[row + lines];
  }

  // Clear bottom lines
  for (int row = m_rows - lines; row < m_rows; ++row) {
    for (auto &cell : m_buffer[row]) {
      cell.character = ' ';
    }
  }
}

void TerminalBuffer::SetPromptEnd(int x, int y) {
  m_promptEndX = x;
  m_promptEndY = y;
}

void TerminalBuffer::ResetPromptProtection() {
  m_promptEndX = -1;
  m_promptEndY = -1;
}
