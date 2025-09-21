#include "windowed.hpp"
#include "../../app/terminal/renderer.hpp"
#include "../../app/terminal/processmanager.hpp"
#include "../../app/terminal/terminalbuffer.hpp"
#include <iostream>

TerminalWindow::TerminalWindow()
    : m_window(nullptr)
    , m_running(false)
    , m_windowWidth(1200)
    , m_windowHeight(720)
    , m_charWidth(8)
    , m_charHeight(16)
    , m_cols(80)
    , m_rows(25)
{
}

TerminalWindow::~TerminalWindow() {
    Shutdown();
}

bool TerminalWindow::Initialize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;

    // Initialize SDL
    std::cout << "Attempting to initialize SDL..." << std::endl;
    
    // Check available video drivers first
    int numDrivers = SDL_GetNumVideoDrivers();
    std::cout << "Available video drivers (" << numDrivers << "):" << std::endl;
    for (int i = 0; i < numDrivers; i++) {
        std::cout << "  " << i << ": " << SDL_GetVideoDriver(i) << std::endl;
    }
    
    // Try to initialize SDL with just video first
    int result = SDL_Init(SDL_INIT_VIDEO);
    if (result != 0) {
        const char* error = SDL_GetError();
        std::cerr << "SDL video initialization failed with code " << result << ": " << (error ? error : "Unknown error") << std::endl;
        
        // Try different video drivers explicitly using environment variable
        const char* drivers[] = {"windows", "software", nullptr};
        for (int i = 0; drivers[i] != nullptr; i++) {
            std::cout << "Trying to force video driver: " << drivers[i] << std::endl;
            SDL_SetHint("SDL_VIDEODRIVER", drivers[i]);
            SDL_Quit(); // Clean up previous attempt
            result = SDL_Init(SDL_INIT_VIDEO);
            if (result == 0) {
                std::cout << "Successfully initialized with driver: " << drivers[i] << std::endl;
                break;
            } else {
                std::cerr << "Failed with driver " << drivers[i] << ": " << SDL_GetError() << std::endl;
            }
        }
        
        if (result != 0) {
            return false;
        }
    }
    
    // Now initialize events
    if (SDL_InitSubSystem(SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL events initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    std::cout << "SDL initialized successfully with driver: " << SDL_GetCurrentVideoDriver() << std::endl;

    // Create window
    m_window = SDL_CreateWindow(
        "MikoTerminal",
        width, height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );

    if (!m_window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Enable text input
    SDL_StartTextInput(m_window);

    // Initialize components
    m_textRenderer = std::make_unique<DirectWriteRenderer>();
    if (!m_textRenderer->Initialize(m_window)) {
        std::cerr << "DirectWrite renderer initialization failed" << std::endl;
        return false;
    }

    m_terminalBuffer = std::make_unique<TerminalBuffer>();
    m_terminalBuffer->Initialize(m_cols, m_rows);

    m_processManager = std::make_unique<ProcessManager>();
    if (!m_processManager->Initialize("pwsh.exe")) {
        std::cerr << "Process manager initialization failed" << std::endl;
        return false;
    }

    // Set up process output callback
    m_processManager->SetOutputCallback([this](const std::string& output) {
        m_terminalBuffer->AppendOutput(output);
    });

    // Calculate character dimensions from font
    auto [charW, charH] = m_textRenderer->GetCharacterSize();
    m_charWidth = charW;
    m_charHeight = charH;

    // Recalculate terminal dimensions
    m_cols = m_windowWidth / m_charWidth;
    m_rows = m_windowHeight / m_charHeight;
    m_terminalBuffer->Resize(m_cols, m_rows);

    m_running = true;
    return true;
}

void TerminalWindow::Run() {
    while (m_running) {
        HandleEvents();
        UpdateTerminal();
        Render();
        
        // Reduce delay for more responsive rendering, especially for fast output
        SDL_Delay(8); // ~120 FPS for better responsiveness
    }
}

void TerminalWindow::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                m_running = false;
                break;
                
            case SDL_EVENT_KEY_DOWN:
                OnKeyDown(event.key);
                break;
                
            case SDL_EVENT_TEXT_INPUT:
                OnTextInput(event.text);
                break;
                
            case SDL_EVENT_WINDOW_RESIZED:
                OnResize(event.window.data1, event.window.data2);
                break;
        }
    }
}

void TerminalWindow::OnKeyDown(const SDL_KeyboardEvent& event) {
    std::string input;
    
    switch (event.scancode) {
        case SDL_SCANCODE_RETURN:
            input = "\r\n";
            // Echo newline for immediate feedback
            m_terminalBuffer->AppendOutput("\r\n");
            break;
            
        case SDL_SCANCODE_BACKSPACE:
            input = "\b";
            // Handle backspace visually - remove last character if possible
            {
                auto [cursorX, cursorY] = m_terminalBuffer->GetCursorPosition();
                if (cursorX > 0) {
                    m_terminalBuffer->MoveCursor(cursorX - 1, cursorY);
                    // Clear the character at the cursor position
                    auto buffer = m_terminalBuffer->GetBuffer();
                    if (cursorY < buffer.size() && cursorX - 1 < buffer[cursorY].size()) {
                        // This is a simple approach - in a real terminal we'd need more sophisticated handling
                        m_terminalBuffer->AppendOutput(" \b");
                    }
                }
            }
            break;
            
        case SDL_SCANCODE_TAB:
            input = "\t";
            m_terminalBuffer->AppendOutput("\t");
            break;
            
        case SDL_SCANCODE_ESCAPE:
            input = "\x1b";
            break;
            
        case SDL_SCANCODE_UP:
            input = "\x1b[A";
            break;
            
        case SDL_SCANCODE_DOWN:
            input = "\x1b[B";
            break;
            
        case SDL_SCANCODE_RIGHT:
            input = "\x1b[C";
            break;
            
        case SDL_SCANCODE_LEFT:
            input = "\x1b[D";
            break;
            
        case SDL_SCANCODE_HOME:
            input = "\x1b[H";
            break;
            
        case SDL_SCANCODE_END:
            input = "\x1b[F";
            break;
            
        case SDL_SCANCODE_PAGEUP:
            input = "\x1b[5~";
            break;
            
        case SDL_SCANCODE_PAGEDOWN:
            input = "\x1b[6~";
            break;
            
        case SDL_SCANCODE_DELETE:
            input = "\x1b[3~";
            break;
            
        default:
            // Handle Ctrl+C
            if (event.mod & SDL_KMOD_CTRL) {
                if (event.scancode == SDL_SCANCODE_C) {
                    input = "\x03"; // Ctrl+C
                }
            }
            break;
    }
    
    if (!input.empty()) {
        m_processManager->SendInput(input);
    }
}

void TerminalWindow::OnTextInput(const SDL_TextInputEvent& event) {
    // Send input to the process
    m_processManager->SendInput(event.text);
    
    // Local echo - display the typed character immediately
    // This provides immediate visual feedback while typing
    std::string inputText = event.text;
    
    // Only echo printable characters (not control characters)
    bool shouldEcho = true;
    for (char c : inputText) {
        if (c < 32 && c != '\t') { // Skip control characters except tab
            shouldEcho = false;
            break;
        }
    }
    
    if (shouldEcho) {
        m_terminalBuffer->AppendOutput(inputText);
    }
}

void TerminalWindow::OnResize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
    
    // Recalculate terminal dimensions
    m_cols = width / m_charWidth;
    m_rows = height / m_charHeight;
    
    m_terminalBuffer->Resize(m_cols, m_rows);
    m_textRenderer->OnResize(width, height);
}

void TerminalWindow::UpdateTerminal() {
    // Read any pending output from the process
    m_processManager->Update();
}

void TerminalWindow::Render() {
    // Render terminal text using DirectWrite
    m_textRenderer->RenderTerminal(*m_terminalBuffer);
}

void TerminalWindow::Shutdown() {
    if (m_processManager) {
        m_processManager.reset();
    }
    
    if (m_textRenderer) {
        m_textRenderer.reset();
    }
    
    if (m_terminalBuffer) {
        m_terminalBuffer.reset();
    }
    
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    
    SDL_Quit();
}