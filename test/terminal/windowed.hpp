#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <functional>

class DirectWriteRenderer;
class ProcessManager;
class TerminalBuffer;

class TerminalWindow {
public:
    TerminalWindow();
    ~TerminalWindow();

    bool Initialize(int width = 1200, int height = 800);
    void Run();
    void Shutdown();

    // Event callbacks
    void OnKeyDown(const SDL_KeyboardEvent& event);
    void OnTextInput(const SDL_TextInputEvent& event);
    void OnResize(int width, int height);

private:
    void HandleEvents();
    void Render();
    void UpdateTerminal();

    SDL_Window* m_window;
    
    std::unique_ptr<DirectWriteRenderer> m_textRenderer;
    std::unique_ptr<ProcessManager> m_processManager;
    std::unique_ptr<TerminalBuffer> m_terminalBuffer;
    
    bool m_running;
    int m_windowWidth;
    int m_windowHeight;
    
    // Terminal settings
    int m_charWidth;
    int m_charHeight;
    int m_cols;
    int m_rows;
};