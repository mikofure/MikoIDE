#include "windowed.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>

#ifdef _WIN32
#include <SDL3/SDL_system.h>
#endif

namespace Hyperion {
namespace Toolchain {

// UIButton Implementation
UIButton::UIButton(const std::string& text, std::function<void()> onClick)
    : m_text(text), m_onClick(onClick) {
}

void UIButton::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat) {
    if (!IsVisible() || !renderTarget || !textFormat) return;

    ID2D1SolidColorBrush* brush = nullptr;
    
    // Create brush for button background
    D2D1_COLOR_F bgColor;
    if (m_pressed) {
        bgColor = D2D1::ColorF(0.1f, 0.4f, 0.8f, 1.0f); // Pressed state
    } else if (m_hovered) {
        bgColor = D2D1::ColorF(0.25f, 0.65f, 1.0f, 1.0f); // Hovered state
    } else {
        bgColor = D2D1::ColorF(0.2f, 0.6f, 1.0f, 1.0f); // Normal state
    }
    
    renderTarget->CreateSolidColorBrush(bgColor, &brush);
    
    if (brush) {
        // Draw button background
        D2D1_RECT_F rect = D2D1::RectF(m_bounds.x, m_bounds.y, 
                                       m_bounds.x + m_bounds.width, 
                                       m_bounds.y + m_bounds.height);
        renderTarget->FillRectangle(rect, brush);
        
        // Draw button text
        brush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f));
        
        std::wstring wtext(m_text.begin(), m_text.end());
        renderTarget->DrawText(wtext.c_str(), static_cast<UINT32>(wtext.length()),
                              textFormat, rect, brush);
        
        brush->Release();
    }
}

bool UIButton::HandleEvent(const SDL_Event& event) {
    if (!IsVisible()) return false;
    
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        float mouseX = static_cast<float>(event.motion.x);
        float mouseY = static_cast<float>(event.motion.y);
        m_hovered = m_bounds.Contains(mouseX, mouseY);
    }
    else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            float mouseX = static_cast<float>(event.button.x);
            float mouseY = static_cast<float>(event.button.y);
            if (m_bounds.Contains(mouseX, mouseY)) {
                m_pressed = true;
                return true;
            }
        }
    }
    else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (event.button.button == SDL_BUTTON_LEFT && m_pressed) {
            float mouseX = static_cast<float>(event.button.x);
            float mouseY = static_cast<float>(event.button.y);
            if (m_bounds.Contains(mouseX, mouseY) && m_onClick) {
                m_onClick();
            }
            m_pressed = false;
            return true;
        }
    }
    
    return false;
}

// UILabel Implementation
UILabel::UILabel(const std::string& text) : m_text(text) {
}

void UILabel::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat) {
    if (!IsVisible() || !renderTarget || !textFormat) return;

    ID2D1SolidColorBrush* brush = nullptr;
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.9f, 0.9f, 1.0f), &brush);
    
    if (brush) {
        D2D1_RECT_F rect = D2D1::RectF(m_bounds.x, m_bounds.y, 
                                       m_bounds.x + m_bounds.width, 
                                       m_bounds.y + m_bounds.height);
        
        std::wstring wtext(m_text.begin(), m_text.end());
        renderTarget->DrawText(wtext.c_str(), static_cast<UINT32>(wtext.length()),
                              textFormat, rect, brush);
        
        brush->Release();
    }
}

bool UILabel::HandleEvent(const SDL_Event& event) {
    return false; // Labels don't handle events
}

// UIList Implementation
UIList::UIList() {
}

void UIList::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat) {
    if (!IsVisible() || !renderTarget || !textFormat) return;

    ID2D1SolidColorBrush* brush = nullptr;
    ID2D1SolidColorBrush* selectionBrush = nullptr;
    
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.15f, 0.15f, 0.15f, 1.0f), &brush);
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.3f, 0.7f, 1.0f, 0.3f), &selectionBrush);
    
    if (brush && selectionBrush) {
        // Draw background
        D2D1_RECT_F bgRect = D2D1::RectF(m_bounds.x, m_bounds.y, 
                                         m_bounds.x + m_bounds.width, 
                                         m_bounds.y + m_bounds.height);
        renderTarget->FillRectangle(bgRect, brush);
        
        // Draw items
        float itemHeight = 25.0f;
        float currentY = m_bounds.y + 5.0f;
        
        for (size_t i = m_scrollOffset; i < m_items.size() && currentY < m_bounds.y + m_bounds.height; ++i) {
            D2D1_RECT_F itemRect = D2D1::RectF(m_bounds.x + 5.0f, currentY, 
                                               m_bounds.x + m_bounds.width - 5.0f, 
                                               currentY + itemHeight);
            
            // Draw selection background
            if (static_cast<int>(i) == m_selectedIndex) {
                renderTarget->FillRectangle(itemRect, selectionBrush);
            }
            
            // Draw item text
            brush->SetColor(D2D1::ColorF(0.9f, 0.9f, 0.9f, 1.0f));
            std::wstring wtext(m_items[i].begin(), m_items[i].end());
            renderTarget->DrawText(wtext.c_str(), static_cast<UINT32>(wtext.length()),
                                  textFormat, itemRect, brush);
            
            currentY += itemHeight;
        }
        
        brush->Release();
        selectionBrush->Release();
    }
}

bool UIList::HandleEvent(const SDL_Event& event) {
    if (!IsVisible()) return false;
    
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            float mouseX = static_cast<float>(event.button.x);
            float mouseY = static_cast<float>(event.button.y);
            
            if (m_bounds.Contains(mouseX, mouseY)) {
                float itemHeight = 25.0f;
                int clickedIndex = static_cast<int>((mouseY - m_bounds.y - 5.0f) / itemHeight) + m_scrollOffset;
                
                if (clickedIndex >= 0 && clickedIndex < static_cast<int>(m_items.size())) {
                    m_selectedIndex = clickedIndex;
                    if (m_onSelectionChanged) {
                        m_onSelectionChanged(m_selectedIndex, m_items[m_selectedIndex]);
                    }
                    return true;
                }
            }
        }
    }
    
    return false;
}

void UIList::AddItem(const std::string& item) {
    m_items.push_back(item);
}

void UIList::RemoveItem(const std::string& item) {
    auto it = std::find(m_items.begin(), m_items.end(), item);
    if (it != m_items.end()) {
        m_items.erase(it);
        if (m_selectedIndex >= static_cast<int>(m_items.size())) {
            m_selectedIndex = static_cast<int>(m_items.size()) - 1;
        }
    }
}

void UIList::Clear() {
    m_items.clear();
    m_selectedIndex = -1;
    m_scrollOffset = 0;
}

std::string UIList::GetSelectedItem() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
        return m_items[m_selectedIndex];
    }
    return "";
}

// UITextInput Implementation
UITextInput::UITextInput(const std::string& placeholder) : m_placeholder(placeholder) {
}

void UITextInput::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat) {
    if (!IsVisible() || !renderTarget || !textFormat) return;

    ID2D1SolidColorBrush* brush = nullptr;
    ID2D1SolidColorBrush* borderBrush = nullptr;
    
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.2f, 0.2f, 0.2f, 1.0f), &brush);
    renderTarget->CreateSolidColorBrush(m_focused ? D2D1::ColorF(0.2f, 0.6f, 1.0f, 1.0f) : D2D1::ColorF(0.4f, 0.4f, 0.4f, 1.0f), &borderBrush);
    
    if (brush && borderBrush) {
        D2D1_RECT_F rect = D2D1::RectF(m_bounds.x, m_bounds.y, 
                                       m_bounds.x + m_bounds.width, 
                                       m_bounds.y + m_bounds.height);
        
        // Draw background
        renderTarget->FillRectangle(rect, brush);
        
        // Draw border
        renderTarget->DrawRectangle(rect, borderBrush, 2.0f);
        
        // Draw text or placeholder
        brush->SetColor(D2D1::ColorF(0.9f, 0.9f, 0.9f, 1.0f));
        std::string displayText = m_text.empty() ? m_placeholder : m_text;
        if (m_text.empty()) {
            brush->SetColor(D2D1::ColorF(0.6f, 0.6f, 0.6f, 1.0f));
        }
        
        D2D1_RECT_F textRect = D2D1::RectF(m_bounds.x + 5.0f, m_bounds.y + 5.0f, 
                                           m_bounds.x + m_bounds.width - 5.0f, 
                                           m_bounds.y + m_bounds.height - 5.0f);
        
        std::wstring wtext(displayText.begin(), displayText.end());
        renderTarget->DrawText(wtext.c_str(), static_cast<UINT32>(wtext.length()),
                              textFormat, textRect, brush);
        
        brush->Release();
        borderBrush->Release();
    }
}

bool UITextInput::HandleEvent(const SDL_Event& event) {
    if (!IsVisible()) return false;
    
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            float mouseX = static_cast<float>(event.button.x);
            float mouseY = static_cast<float>(event.button.y);
            m_focused = m_bounds.Contains(mouseX, mouseY);
            return m_focused;
        }
    }
    else if (event.type == SDL_EVENT_TEXT_INPUT && m_focused) {
        m_text += event.text.text;
        m_cursorPosition = m_text.length();
        return true;
    }
    else if (event.type == SDL_EVENT_KEY_DOWN && m_focused) {
        if (event.key.key == SDLK_BACKSPACE && !m_text.empty()) {
            m_text.pop_back();
            m_cursorPosition = m_text.length();
            return true;
        }
    }
    
    return false;
}

// UIPanel Implementation
UIPanel::UIPanel() {
}

void UIPanel::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat) {
    if (!IsVisible()) return;
    
    for (auto& widget : m_widgets) {
        if (widget && widget->IsVisible()) {
            widget->Render(renderTarget, textFormat);
        }
    }
}

bool UIPanel::HandleEvent(const SDL_Event& event) {
    if (!IsVisible()) return false;
    
    // Handle events in reverse order (top widgets first)
    for (auto it = m_widgets.rbegin(); it != m_widgets.rend(); ++it) {
        if (*it && (*it)->IsVisible() && (*it)->HandleEvent(event)) {
            return true;
        }
    }
    
    return false;
}

void UIPanel::AddWidget(std::shared_ptr<UIWidget> widget) {
    if (widget) {
        m_widgets.push_back(widget);
    }
}

void UIPanel::RemoveWidget(std::shared_ptr<UIWidget> widget) {
    auto it = std::find(m_widgets.begin(), m_widgets.end(), widget);
    if (it != m_widgets.end()) {
        m_widgets.erase(it);
    }
}

void UIPanel::Clear() {
    m_widgets.clear();
}

// WindowedUI Implementation
WindowedUI::WindowedUI()
    : m_window(nullptr)
    , m_renderer(nullptr)
    , m_hwnd(nullptr)
    , m_d2dFactory(nullptr)
    , m_renderTarget(nullptr)
    , m_brush(nullptr)
    , m_writeFactory(nullptr)
    , m_textFormat(nullptr)
    , m_focusedWidget(nullptr)
    , m_width(1200)
    , m_height(800)
    , m_initialized(false)
    , m_minimized(false) {
}

WindowedUI::~WindowedUI() {
    Shutdown();
}

bool WindowedUI::Initialize(const std::string& title, int width, int height) {
    m_width = width;
    m_height = height;
    
    if (!InitializeSDL(title, width, height)) {
        std::cerr << "Failed to initialize SDL" << std::endl;
        return false;
    }
    
    if (!InitializeDirect2D()) {
        std::cerr << "Failed to initialize Direct2D" << std::endl;
        return false;
    }
    
    if (!InitializeDirectWrite()) {
        std::cerr << "Failed to initialize DirectWrite" << std::endl;
        return false;
    }
    
    CreateUIElements();
    SetupMainLayout();
    
    m_initialized = true;
    return true;
}

void WindowedUI::Shutdown() {
    if (!m_initialized) return;
    
    // Release DirectWrite resources
    if (m_textFormat) {
        m_textFormat->Release();
        m_textFormat = nullptr;
    }
    
    if (m_writeFactory) {
        m_writeFactory->Release();
        m_writeFactory = nullptr;
    }
    
    // Release Direct2D resources
    if (m_brush) {
        m_brush->Release();
        m_brush = nullptr;
    }
    
    if (m_renderTarget) {
        m_renderTarget->Release();
        m_renderTarget = nullptr;
    }
    
    if (m_d2dFactory) {
        m_d2dFactory->Release();
        m_d2dFactory = nullptr;
    }
    
    // Release SDL resources
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    
    SDL_Quit();
    m_initialized = false;
}

bool WindowedUI::InitializeSDL(const std::string& title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    m_window = SDL_CreateWindow(title.c_str(),
                               width, height,
                               SDL_WINDOW_RESIZABLE);
    
    if (!m_window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
#ifdef _WIN32
    // Get the Windows handle for Direct2D using SDL3 properties API
    SDL_PropertiesID props = SDL_GetWindowProperties(m_window);
    m_hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#endif
    
    return true;
}

bool WindowedUI::InitializeDirect2D() {
#ifdef _WIN32
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_d2dFactory);
    if (FAILED(hr)) {
        return false;
    }
    
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
    
    hr = m_d2dFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hwnd, size),
        &m_renderTarget);
    
    if (FAILED(hr)) {
        return false;
    }
    
    hr = m_renderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        &m_brush);
    
    return SUCCEEDED(hr);
#else
    return false; // Direct2D is Windows-only
#endif
}

bool WindowedUI::InitializeDirectWrite() {
#ifdef _WIN32
    HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(m_writeFactory),
        reinterpret_cast<IUnknown**>(&m_writeFactory));
    
    if (FAILED(hr)) {
        return false;
    }
    
    hr = m_writeFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        14.0f,
        L"",
        &m_textFormat);
    
    if (FAILED(hr)) {
        return false;
    }
    
    m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    
    return true;
#else
    return false; // DirectWrite is Windows-only
#endif
}

void WindowedUI::CreateUIElements() {
    // Create main panels
    m_mainPanel = std::make_shared<UIPanel>();
    m_toolchainPanel = std::make_shared<UIPanel>();
    m_projectPanel = std::make_shared<UIPanel>();
    m_consolePanel = std::make_shared<UIPanel>();
    
    // Create UI elements
    m_toolchainList = std::make_shared<UIList>();
    m_projectList = std::make_shared<UIList>();
    m_consoleOutput = std::make_shared<UILabel>("Console output will appear here...");
    m_statusLabel = std::make_shared<UILabel>("Ready");
    
    // Create buttons with callbacks
    m_buildButton = std::make_shared<UIButton>("Build", [this]() {
        if (m_onBuildRequested) {
            m_onBuildRequested();
        }
    });
    
    m_runButton = std::make_shared<UIButton>("Run", [this]() {
        if (m_onRunRequested) {
            m_onRunRequested();
        }
    });
    
    m_newProjectButton = std::make_shared<UIButton>("New Project", [this]() {
        if (m_onNewProject) {
            m_onNewProject();
        }
    });
    
    m_openProjectButton = std::make_shared<UIButton>("Open Project", [this]() {
        if (m_onOpenProject) {
            m_onOpenProject();
        }
    });
    
    // Add widgets to panels
    m_toolchainPanel->AddWidget(m_toolchainList);
    m_projectPanel->AddWidget(m_projectList);
    m_projectPanel->AddWidget(m_newProjectButton);
    m_projectPanel->AddWidget(m_openProjectButton);
    m_consolePanel->AddWidget(m_consoleOutput);
    
    // Add action buttons to main panel
    m_mainPanel->AddWidget(m_buildButton);
    m_mainPanel->AddWidget(m_runButton);
    
    // Add all panels to main widget list
    AddWidget(m_toolchainPanel);
    AddWidget(m_projectPanel);
    AddWidget(m_consolePanel);
    AddWidget(m_mainPanel);
    AddWidget(m_statusLabel);
}

void WindowedUI::SetupMainLayout() {
    UpdateLayout();
}

void WindowedUI::UpdateLayout() {
    float panelWidth = static_cast<float>(m_width) / 3.0f;
    float panelHeight = static_cast<float>(m_height) - 100.0f; // Leave space for status bar and buttons
    
    // Layout toolchain panel (left)
    m_toolchainPanel->SetBounds(Rect(10.0f, 10.0f, panelWidth - 20.0f, panelHeight));
    m_toolchainList->SetBounds(Rect(10.0f, 10.0f, panelWidth - 20.0f, panelHeight - 20.0f));
    
    // Layout project panel (middle)
    m_projectPanel->SetBounds(Rect(panelWidth + 10.0f, 10.0f, panelWidth - 20.0f, panelHeight));
    m_projectList->SetBounds(Rect(panelWidth + 10.0f, 10.0f, panelWidth - 20.0f, panelHeight - 80.0f));
    m_newProjectButton->SetBounds(Rect(panelWidth + 10.0f, panelHeight - 60.0f, 100.0f, 30.0f));
    m_openProjectButton->SetBounds(Rect(panelWidth + 120.0f, panelHeight - 60.0f, 100.0f, 30.0f));
    
    // Layout console panel (right)
    m_consolePanel->SetBounds(Rect(2.0f * panelWidth + 10.0f, 10.0f, panelWidth - 20.0f, panelHeight));
    m_consoleOutput->SetBounds(Rect(2.0f * panelWidth + 10.0f, 10.0f, panelWidth - 20.0f, panelHeight - 20.0f));
    
    // Layout action buttons
    m_buildButton->SetBounds(Rect(10.0f, panelHeight + 20.0f, 100.0f, 30.0f));
    m_runButton->SetBounds(Rect(120.0f, panelHeight + 20.0f, 100.0f, 30.0f));
    
    // Layout status bar
    m_statusLabel->SetBounds(Rect(10.0f, static_cast<float>(m_height) - 30.0f, static_cast<float>(m_width) - 20.0f, 20.0f));
}

bool WindowedUI::HandleEvent(const SDL_Event& event) {
    if (!m_initialized) return false;
    
    if (event.type == SDL_EVENT_WINDOW_RESIZED) {
        m_width = event.window.data1;
        m_height = event.window.data2;
        UpdateLayout();
        
#ifdef _WIN32
        if (m_renderTarget) {
            D2D1_SIZE_U size = D2D1::SizeU(m_width, m_height);
            m_renderTarget->Resize(size);
        }
#endif
        return true;
    }
    
    // Handle widget events
    for (auto it = m_widgets.rbegin(); it != m_widgets.rend(); ++it) {
        if (*it && (*it)->IsVisible() && (*it)->HandleEvent(event)) {
            return true;
        }
    }
    
    return false;
}

void WindowedUI::Render() {
    if (!m_initialized || !m_renderTarget) return;
    
    BeginFrame();
    RenderBackground();
    RenderWidgets();
    RenderStatusBar();
    EndFrame();
}

void WindowedUI::BeginFrame() {
#ifdef _WIN32
    if (m_renderTarget) {
        m_renderTarget->BeginDraw();
        m_renderTarget->Clear(ColorFromTheme(m_theme.background.r, m_theme.background.g, m_theme.background.b, m_theme.background.a));
    }
#endif
}

void WindowedUI::EndFrame() {
#ifdef _WIN32
    if (m_renderTarget) {
        HRESULT hr = m_renderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET) {
            // Recreate render target if needed
            m_renderTarget->Release();
            m_renderTarget = nullptr;
            InitializeDirect2D();
        }
    }
#endif
}

void WindowedUI::RenderBackground() {
    // Background is cleared in BeginFrame()
}

void WindowedUI::RenderWidgets() {
    for (auto& widget : m_widgets) {
        if (widget && widget->IsVisible()) {
            widget->Render(m_renderTarget, m_textFormat);
        }
    }
}

void WindowedUI::RenderStatusBar() {
    // Status bar is rendered as part of the status label widget
}

D2D1_COLOR_F WindowedUI::ColorFromTheme(float r, float g, float b, float a) const {
    return D2D1::ColorF(r, g, b, a);
}

void WindowedUI::AddWidget(std::shared_ptr<UIWidget> widget) {
    if (widget) {
        m_widgets.push_back(widget);
    }
}

void WindowedUI::RemoveWidget(std::shared_ptr<UIWidget> widget) {
    auto it = std::find(m_widgets.begin(), m_widgets.end(), widget);
    if (it != m_widgets.end()) {
        m_widgets.erase(it);
    }
}

void WindowedUI::ClearWidgets() {
    m_widgets.clear();
}

void WindowedUI::SetTitle(const std::string& title) {
    if (m_window) {
        SDL_SetWindowTitle(m_window, title.c_str());
    }
}

void WindowedUI::SetSize(int width, int height) {
    if (m_window) {
        SDL_SetWindowSize(m_window, width, height);
        m_width = width;
        m_height = height;
        UpdateLayout();
    }
}

void WindowedUI::GetSize(int& width, int& height) const {
    width = m_width;
    height = m_height;
}

bool WindowedUI::IsMinimized() const {
    return m_minimized;
}

void WindowedUI::Minimize() {
    if (m_window) {
        SDL_MinimizeWindow(m_window);
        m_minimized = true;
    }
}

void WindowedUI::Maximize() {
    if (m_window) {
        SDL_MaximizeWindow(m_window);
        m_minimized = false;
    }
}

void WindowedUI::Restore() {
    if (m_window) {
        SDL_RestoreWindow(m_window);
        m_minimized = false;
    }
}

void WindowedUI::ShowToolchainList(const std::vector<std::string>& toolchains) {
    if (m_toolchainList) {
        m_toolchainList->Clear();
        for (const auto& toolchain : toolchains) {
            m_toolchainList->AddItem(toolchain);
        }
    }
}

void WindowedUI::ShowProjectList(const std::vector<std::string>& projects) {
    if (m_projectList) {
        m_projectList->Clear();
        for (const auto& project : projects) {
            m_projectList->AddItem(project);
        }
    }
}

void WindowedUI::ShowConsoleOutput(const std::string& output) {
    if (m_consoleOutput) {
        m_consoleOutput->SetText(output);
    }
}

void WindowedUI::SetStatusText(const std::string& status) {
    if (m_statusLabel) {
        m_statusLabel->SetText(status);
    }
}

} // namespace Toolchain
} // namespace Hyperion