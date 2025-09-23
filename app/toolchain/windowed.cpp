#include "windowed.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>

#ifdef _WIN32
#include <SDL3/SDL_system.h>
#endif

namespace Hyperion {
namespace Toolchain {



// UILabel Implementation
UILabel::UILabel(const std::string& text) : m_text(text) {
}

void UILabel::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat, IDWriteTextFormat* regularTextFormat, IDWriteTextFormat* semiboldCenteredTextFormat) {
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

void UIList::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat, IDWriteTextFormat* regularTextFormat, IDWriteTextFormat* semiboldCenteredTextFormat) {
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

void UITextInput::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat, IDWriteTextFormat* regularTextFormat, IDWriteTextFormat* semiboldCenteredTextFormat) {
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

void UIPanel::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat, IDWriteTextFormat* regularTextFormat, IDWriteTextFormat* semiboldCenteredTextFormat) {
    if (!IsVisible()) return;
    
    for (auto& widget : m_widgets) {
        if (widget && widget->IsVisible()) {
            widget->Render(renderTarget, textFormat, boldTextFormat, regularTextFormat, semiboldCenteredTextFormat);
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
    , m_menuBar(nullptr)
    , m_originalWndProc(nullptr)
    , m_d2dFactory(nullptr)
    , m_renderTarget(nullptr)
    , m_brush(nullptr)
    , m_writeFactory(nullptr)
    , m_textFormat(nullptr)
    , m_focusedWidget(nullptr)
    , m_width(1200)
    , m_height(800)
    , m_initialized(false)
    , m_minimized(false)
    , m_header(nullptr)
    , m_sidebar(nullptr)
    , m_content(nullptr) {
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
    
#ifdef _WIN32
    if (!CreateMenuBar()) {
        std::cerr << "Failed to create menu bar" << std::endl;
        return false;
    }
#endif
    
    CreateUIElements();
    SetupMainLayout();
    
    m_initialized = true;
    return true;
}

void WindowedUI::Shutdown() {
    if (!m_initialized) return;
    
    // Release DirectWrite resources
    if (m_boldTextFormat) {
        m_boldTextFormat->Release();
        m_boldTextFormat = nullptr;
    }
    
    if (m_centeredTextFormat) {
        m_centeredTextFormat->Release();
        m_centeredTextFormat = nullptr;
    }
    
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

#ifdef _WIN32
    // Restore original window procedure and cleanup menu
    if (m_hwnd && m_originalWndProc) {
        SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)m_originalWndProc);
        m_originalWndProc = nullptr;
    }
    
    if (m_menuBar) {
        DestroyMenu(m_menuBar);
        m_menuBar = nullptr;
    }
#endif
    
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
    
    // Create centered text format for buttons
    hr = m_writeFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        14.0f,
        L"",
        &m_centeredTextFormat);
    
    if (FAILED(hr)) {
        return false;
    }
    
    m_centeredTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_centeredTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    
    // Create bold text format for headers
    hr = m_writeFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_SEMI_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        14.0f,
        L"",
        &m_boldTextFormat);
    
    if (FAILED(hr)) {
        return false;
    }
    
    m_boldTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_boldTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    
    // Create semibold centered text format for buttons
    hr = m_writeFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_SEMI_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        14.0f,
        L"",
        &m_semiboldCenteredTextFormat);
    
    if (FAILED(hr)) {
        return false;
    }
    
    m_semiboldCenteredTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_semiboldCenteredTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    
    // Create regular text format for path text
    hr = m_writeFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        14.0f,
        L"",
        &m_regularTextFormat);
    
    if (FAILED(hr)) {
        return false;
    }
    
    m_regularTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_regularTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    
    return true;
#else
    return false; // DirectWrite is Windows-only
#endif
}

void WindowedUI::CreateUIElements() {
    // Clear existing widgets
    ClearWidgets();
    
    // Create new UI components for the redesigned interface
    m_header = std::make_shared<UIHeader>("C:\\buildworkers\\hyperion");
    m_header->SetOnChangeDirectory([this]() {
        // Handle change directory action
        std::cout << "Change directory requested" << std::endl;
    });
    
    m_sidebar = std::make_shared<UISidebar>();
    m_sidebar->SetOnCategorySelected([this](const std::string& category) {
        // Handle category selection
        std::cout << "Category selected: " << category << std::endl;
        if (m_content) {
            m_content->FilterByCategory(category);
        }
    });
    
    // Add categories to sidebar
    m_sidebar->AddCategory("All", 7);
    m_sidebar->AddCategory("Compiler", 2);
    m_sidebar->AddCategory("Runtime", 3);
    m_sidebar->AddCategory("SDK", 1);
    m_sidebar->AddCategory("C++ Library", 1);
    m_sidebar->AddCategory("Language Server", 0);
    m_sidebar->AddCategory("Utilities", 0);
    
    m_content = std::make_shared<UIContent>();
    
    // Add sample toolchain cards
    auto pythonCard = std::make_shared<UIToolchainCard>(
        "Python", "3.13.0", 
        "A programming language that lets you work quickly and integrate systems more effectively",
        "Runtime"
    );
    pythonCard->SetOnInstall([](const std::string& name) {
        std::cout << "Installing " << name << std::endl;
    });
    m_content->AddCard(pythonCard);
    
    auto gccCard = std::make_shared<UIToolchainCard>(
        "GCC", "13.2.0",
        "The GNU Compiler Collection includes front ends for C, C++, Objective-C, Fortran, Ada, Go, and D",
        "Compiler"
    );
    gccCard->SetOnInstall([](const std::string& name) {
        std::cout << "Installing " << name << std::endl;
    });
    m_content->AddCard(gccCard);
    
    auto nodeCard = std::make_shared<UIToolchainCard>(
        "Node.js", "20.10.0",
        "Node.js is a JavaScript runtime built on Chrome's V8 JavaScript engine",
        "Runtime"
    );
    nodeCard->SetOnInstall([](const std::string& name) {
        std::cout << "Installing " << name << std::endl;
    });
    m_content->AddCard(nodeCard);
    
    // Add components to the main widget list
    AddWidget(m_header);
    AddWidget(m_sidebar);
    AddWidget(m_content);
}

void WindowedUI::SetupMainLayout() {
    UpdateLayout();
}

void WindowedUI::UpdateLayout() {
    if (!m_header || !m_sidebar || !m_content) return;
    
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;
    
    // Header layout (full width, fixed height at top)
    int headerHeight = 80;
    m_header->SetBounds({0.0f, 0.0f, static_cast<float>(width), static_cast<float>(headerHeight)});
    
    // Body layout (below header)
    int bodyY = headerHeight;
    int bodyHeight = height - headerHeight;
    
    // Sidebar layout (1/4 width on left side of body)
    int sidebarWidth = width / 4;
    m_sidebar->SetBounds({0.0f, static_cast<float>(bodyY), static_cast<float>(sidebarWidth), static_cast<float>(bodyHeight)});
    
    // Content layout (3/4 width on right side of body)
    int contentX = sidebarWidth;
    int contentWidth = width - sidebarWidth;
    m_content->SetBounds({static_cast<float>(contentX), static_cast<float>(bodyY), static_cast<float>(contentWidth), static_cast<float>(bodyHeight)});
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
            // Use specific text formats for header widget
            if (widget == m_header && m_boldTextFormat && m_regularTextFormat && m_semiboldCenteredTextFormat) {
                // Cast to UIHeader to call the extended Render method
                auto header = std::dynamic_pointer_cast<UIHeader>(widget);
                if (header) {
                    header->Render(m_renderTarget, m_textFormat, m_boldTextFormat, m_regularTextFormat, m_semiboldCenteredTextFormat);
                } else {
                    widget->Render(m_renderTarget, m_textFormat, m_boldTextFormat, m_regularTextFormat, m_semiboldCenteredTextFormat);
                }
            } else {
                // Use left-aligned text format for other widgets
                widget->Render(m_renderTarget, m_textFormat, m_boldTextFormat, m_regularTextFormat, m_semiboldCenteredTextFormat);
            }
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

#ifdef _WIN32
bool WindowedUI::CreateMenuBar() {
    if (!m_hwnd) {
        return false;
    }
    
    // Create the main menu bar
    m_menuBar = CreateMenu();
    if (!m_menuBar) {
        return false;
    }
    
    // Create File menu
    HMENU fileMenu = CreatePopupMenu();
    AppendMenuW(fileMenu, MF_STRING, ID_FILE_NEW_PROJECT, L"&New Project\tCtrl+N");
    AppendMenuW(fileMenu, MF_STRING, ID_FILE_OPEN_PROJECT, L"&Open Project\tCtrl+O");
    AppendMenuW(fileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(fileMenu, MF_STRING, ID_FILE_SAVE_PROJECT, L"&Save Project\tCtrl+S");
    AppendMenuW(fileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(fileMenu, MF_STRING, ID_FILE_CLOSE_PROJECT, L"&Close Project");
    AppendMenuW(fileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(fileMenu, MF_STRING, ID_FILE_EXIT, L"E&xit\tAlt+F4");
    AppendMenuW(m_menuBar, MF_POPUP, (UINT_PTR)fileMenu, L"&File");
    
    // Create Edit menu
    HMENU editMenu = CreatePopupMenu();
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_UNDO, L"&Undo\tCtrl+Z");
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_REDO, L"&Redo\tCtrl+Y");
    AppendMenuW(editMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_CUT, L"Cu&t\tCtrl+X");
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_COPY, L"&Copy\tCtrl+C");
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_PASTE, L"&Paste\tCtrl+V");
    AppendMenuW(editMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_FIND, L"&Find\tCtrl+F");
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_REPLACE, L"&Replace\tCtrl+H");
    AppendMenuW(editMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(editMenu, MF_STRING, ID_EDIT_PREFERENCES, L"&Preferences");
    AppendMenuW(m_menuBar, MF_POPUP, (UINT_PTR)editMenu, L"&Edit");
    
    // Create Build menu
    HMENU buildMenu = CreatePopupMenu();
    AppendMenuW(buildMenu, MF_STRING, ID_BUILD_BUILD, L"&Build\tF7");
    AppendMenuW(buildMenu, MF_STRING, ID_BUILD_REBUILD, L"&Rebuild All\tCtrl+Alt+F7");
    AppendMenuW(buildMenu, MF_STRING, ID_BUILD_CLEAN, L"&Clean\tCtrl+Alt+C");
    AppendMenuW(buildMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(buildMenu, MF_STRING, ID_BUILD_RUN, L"&Run\tF5");
    AppendMenuW(buildMenu, MF_STRING, ID_BUILD_DEBUG, L"&Debug\tF9");
    AppendMenuW(buildMenu, MF_STRING, ID_BUILD_STOP, L"&Stop\tShift+F5");
    AppendMenuW(buildMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(buildMenu, MF_STRING, ID_BUILD_CONFIGURATION, L"&Configuration Manager");
    AppendMenuW(m_menuBar, MF_POPUP, (UINT_PTR)buildMenu, L"&Build");
    
    // Create Tools menu
    HMENU toolsMenu = CreatePopupMenu();
    AppendMenuW(toolsMenu, MF_STRING, ID_TOOLS_TOOLCHAIN_MANAGER, L"&Toolchain Manager");
    AppendMenuW(toolsMenu, MF_STRING, ID_TOOLS_PACKAGE_MANAGER, L"&Package Manager");
    AppendMenuW(toolsMenu, MF_STRING, ID_TOOLS_TERMINAL, L"&Terminal");
    AppendMenuW(toolsMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(toolsMenu, MF_STRING, ID_TOOLS_OPTIONS, L"&Options");
    AppendMenuW(m_menuBar, MF_POPUP, (UINT_PTR)toolsMenu, L"&Tools");
    
    // Create Help menu
    HMENU helpMenu = CreatePopupMenu();
    AppendMenuW(helpMenu, MF_STRING, ID_HELP_DOCUMENTATION, L"&Documentation");
    AppendMenuW(helpMenu, MF_STRING, ID_HELP_KEYBOARD_SHORTCUTS, L"&Keyboard Shortcuts");
    AppendMenuW(helpMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(helpMenu, MF_STRING, ID_HELP_ABOUT, L"&About Hyperion Toolchain");
    AppendMenuW(m_menuBar, MF_POPUP, (UINT_PTR)helpMenu, L"&Help");
    
    // Set the menu to the window
    SetMenu(m_hwnd, m_menuBar);
    
    // Subclass the window to handle menu messages
    m_originalWndProc = (WNDPROC)SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);
    
    return true;
}

void WindowedUI::HandleMenuCommand(WPARAM wParam) {
    UINT menuId = LOWORD(wParam);
    
    switch (menuId) {
        // File menu
        case ID_FILE_NEW_PROJECT:
            if (m_onNewProject) m_onNewProject();
            break;
        case ID_FILE_OPEN_PROJECT:
            if (m_onOpenProject) m_onOpenProject();
            break;
        case ID_FILE_SAVE_PROJECT:
            // TODO: Implement save project functionality
            break;
        case ID_FILE_CLOSE_PROJECT:
            // TODO: Implement close project functionality
            break;
        case ID_FILE_EXIT:
            PostQuitMessage(0);
            break;
            
        // Edit menu
        case ID_EDIT_UNDO:
            // TODO: Implement undo functionality
            break;
        case ID_EDIT_REDO:
            // TODO: Implement redo functionality
            break;
        case ID_EDIT_CUT:
            // TODO: Implement cut functionality
            break;
        case ID_EDIT_COPY:
            // TODO: Implement copy functionality
            break;
        case ID_EDIT_PASTE:
            // TODO: Implement paste functionality
            break;
        case ID_EDIT_FIND:
            // TODO: Implement find functionality
            break;
        case ID_EDIT_REPLACE:
            // TODO: Implement replace functionality
            break;
        case ID_EDIT_PREFERENCES:
            // TODO: Implement preferences dialog
            break;
            
        // Build menu
        case ID_BUILD_BUILD:
            if (m_onBuildRequested) m_onBuildRequested();
            break;
        case ID_BUILD_REBUILD:
            // TODO: Implement rebuild functionality
            break;
        case ID_BUILD_CLEAN:
            // TODO: Implement clean functionality
            break;
        case ID_BUILD_RUN:
            if (m_onRunRequested) m_onRunRequested();
            break;
        case ID_BUILD_DEBUG:
            // TODO: Implement debug functionality
            break;
        case ID_BUILD_STOP:
            // TODO: Implement stop functionality
            break;
        case ID_BUILD_CONFIGURATION:
            // TODO: Implement configuration manager
            break;
            
        // Tools menu
        case ID_TOOLS_TOOLCHAIN_MANAGER:
            // Already in toolchain manager
            break;
        case ID_TOOLS_PACKAGE_MANAGER:
            // TODO: Implement package manager
            break;
        case ID_TOOLS_TERMINAL:
            // TODO: Implement terminal
            break;
        case ID_TOOLS_OPTIONS:
            // TODO: Implement options dialog
            break;
            
        // Help menu
        case ID_HELP_DOCUMENTATION:
            // TODO: Open documentation
            break;
        case ID_HELP_KEYBOARD_SHORTCUTS:
            // TODO: Show keyboard shortcuts dialog
            break;
        case ID_HELP_ABOUT:
            MessageBoxW(m_hwnd, L"Hyperion Toolchain Manager\nVersion 1.0\n\nBuilt with SDL3, Direct2D, and DirectWrite", 
                       L"About Hyperion Toolchain", MB_OK | MB_ICONINFORMATION);
            break;
    }
}

LRESULT CALLBACK WindowedUI::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WindowedUI* ui = (WindowedUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (uMsg) {
        case WM_COMMAND:
            if (ui) {
                ui->HandleMenuCommand(wParam);
            }
            return 0;
            
        case WM_SIZE:
            // Handle window resize for Direct2D render target
            if (ui && ui->m_renderTarget) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
                ui->m_renderTarget->Resize(size);
            }
            break;
    }
    
    // Call the original window procedure for other messages
    if (ui && ui->m_originalWndProc) {
        return CallWindowProc(ui->m_originalWndProc, hwnd, uMsg, wParam, lParam);
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

} // namespace Toolchain
} // namespace Hyperion

// New UI class implementations
namespace Hyperion {
namespace Toolchain {

UIButton::UIButton(const std::string& text, std::function<void()> onClick) 
    : m_text(text), m_onClick(onClick) {
}

void UIButton::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat, IDWriteTextFormat* regularTextFormat, IDWriteTextFormat* semiboldCenteredTextFormat) {
    if (!IsVisible() || !renderTarget || !textFormat) return;

    ID2D1SolidColorBrush* bgBrush = nullptr;
    ID2D1SolidColorBrush* textBrush = nullptr;
    
    // Determine button colors based on state
    float bgR, bgG, bgB, bgA;
    if (m_pressed) {
        bgR = m_bgColor.r * 0.8f;
        bgG = m_bgColor.g * 0.8f;
        bgB = m_bgColor.b * 0.8f;
        bgA = m_bgColor.a;
    } else if (m_hovered) {
        bgR = m_bgColor.r * 1.1f;
        bgG = m_bgColor.g * 1.1f;
        bgB = m_bgColor.b * 1.1f;
        bgA = m_bgColor.a;
    } else {
        bgR = m_bgColor.r;
        bgG = m_bgColor.g;
        bgB = m_bgColor.b;
        bgA = m_bgColor.a;
    }
    
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(bgR, bgG, bgB, bgA), &bgBrush);
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(m_textColor.r, m_textColor.g, m_textColor.b, m_textColor.a), &textBrush);
    
    if (bgBrush && textBrush) {
        // Draw button background
        D2D1_RECT_F bgRect = D2D1::RectF(m_bounds.x, m_bounds.y, 
                                         m_bounds.x + m_bounds.width, 
                                         m_bounds.y + m_bounds.height);
        renderTarget->FillRectangle(bgRect, bgBrush);
        
        // Draw button text (centered) - using semibold centered text format
        std::wstring wtext(m_text.begin(), m_text.end());
        renderTarget->DrawText(wtext.c_str(), static_cast<UINT32>(wtext.length()),
                              textFormat, bgRect, textBrush, D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL);
        
        bgBrush->Release();
        textBrush->Release();
    }
}

bool UIButton::HandleEvent(const SDL_Event& event) {
    if (!IsVisible()) return false;
    
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        float mouseX = static_cast<float>(event.motion.x);
        float mouseY = static_cast<float>(event.motion.y);
        m_hovered = m_bounds.Contains(mouseX, mouseY);
        return m_hovered;
    }
    
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            float mouseX = static_cast<float>(event.button.x);
            float mouseY = static_cast<float>(event.button.y);
            if (m_bounds.Contains(mouseX, mouseY)) {
                m_pressed = true;
                return true;
            }
        }
    }
    
    if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (event.button.button == SDL_BUTTON_LEFT && m_pressed) {
            float mouseX = static_cast<float>(event.button.x);
            float mouseY = static_cast<float>(event.button.y);
            m_pressed = false;
            if (m_bounds.Contains(mouseX, mouseY) && m_onClick) {
                m_onClick();
                return true;
            }
        }
    }
    
    return false;
}

void UIButton::SetStyle(const std::string& bgColor, const std::string& textColor) {
    // Parse hex colors (simplified - assumes #RRGGBB format)
    if (bgColor == "bg-[#D9D9D9]") {
        m_bgColor = {0.85f, 0.85f, 0.85f, 1.0f};
    } else if (bgColor == "bg-[#2A2A2A]") {
        m_bgColor = {0.16f, 0.16f, 0.16f, 1.0f};
    }
    
    if (textColor == "text-black") {
        m_textColor = {0.0f, 0.0f, 0.0f, 1.0f};
    } else if (textColor == "text-white") {
        m_textColor = {1.0f, 1.0f, 1.0f, 1.0f};
    }
}

// UIHeader Implementation
UIHeader::UIHeader(const std::string& path) : m_path(path) {
    m_changeDirectoryButton = std::make_shared<UIButton>("Change Directory");
    m_changeDirectoryButton->SetStyle("bg-[#D9D9D9]", "text-black");
}

void UIHeader::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat, IDWriteTextFormat* regularTextFormat, IDWriteTextFormat* semiboldCenteredTextFormat) {
    if (!IsVisible() || !renderTarget || !textFormat) return;

    ID2D1SolidColorBrush* textBrush = nullptr;
    ID2D1SolidColorBrush* bgBrush = nullptr;
    
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.9f, 0.9f, 1.0f), &textBrush);
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.1f, 0.1f, 0.1f, 1.0f), &bgBrush);
    
    if (textBrush && bgBrush) {
        // Draw header background
        D2D1_RECT_F bgRect = D2D1::RectF(m_bounds.x, m_bounds.y, 
                                         m_bounds.x + m_bounds.width, 
                                         m_bounds.y + m_bounds.height);
        renderTarget->FillRectangle(bgRect, bgBrush);
        
        // Draw "Toolchain Path" label with bold formatting (16px bold)
        D2D1_RECT_F labelRect = D2D1::RectF(m_bounds.x + 20, m_bounds.y + 10, 
                                            m_bounds.x + 200, m_bounds.y + 30);
        std::wstring labelText = L"Toolchain Path";
        renderTarget->DrawText(labelText.c_str(), static_cast<UINT32>(labelText.length()),
                              boldTextFormat ? boldTextFormat : textFormat, labelRect, textBrush);
        
        // Draw path value with regular formatting (16px regular)
        D2D1_RECT_F pathRect = D2D1::RectF(m_bounds.x + 20, m_bounds.y + 35, 
                                           m_bounds.x + m_bounds.width - 200, m_bounds.y + 55);
        std::wstring pathText(m_path.begin(), m_path.end());
        renderTarget->DrawText(pathText.c_str(), static_cast<UINT32>(pathText.length()),
                              regularTextFormat ? regularTextFormat : textFormat, pathRect, textBrush);
        
        // Position and render the change directory button
        float buttonWidth = 150;
        float buttonHeight = 35;
        m_changeDirectoryButton->SetBounds(Rect(m_bounds.x + m_bounds.width - buttonWidth - 20, 
                                               m_bounds.y + 15, buttonWidth, buttonHeight));
        m_changeDirectoryButton->Render(renderTarget, semiboldCenteredTextFormat ? semiboldCenteredTextFormat : textFormat, boldTextFormat, regularTextFormat, semiboldCenteredTextFormat);
        
        textBrush->Release();
        bgBrush->Release();
    }
}

bool UIHeader::HandleEvent(const SDL_Event& event) {
    if (!IsVisible()) return false;
    
    if (m_changeDirectoryButton->HandleEvent(event)) {
        if (m_onChangeDirectory) {
            m_onChangeDirectory();
        }
        return true;
    }
    
    return false;
}

// UICategoryItem Implementation
UICategoryItem::UICategoryItem(const std::string& name, int count) 
    : m_name(name), m_count(count) {
}

void UICategoryItem::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat, IDWriteTextFormat* regularTextFormat, IDWriteTextFormat* semiboldCenteredTextFormat) {
    if (!IsVisible() || !renderTarget || !textFormat) return;

    ID2D1SolidColorBrush* textBrush = nullptr;
    ID2D1SolidColorBrush* bgBrush = nullptr;
    ID2D1SolidColorBrush* badgeBrush = nullptr;
    
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.9f, 0.9f, 1.0f), &textBrush);
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.6f, 0.6f, 0.6f, 1.0f), &badgeBrush);
    
    // Background color based on state
    if (m_selected) {
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.2f, 0.6f, 1.0f, 0.3f), &bgBrush);
    } else if (m_hovered) {
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.2f, 0.2f, 0.2f, 1.0f), &bgBrush);
    } else {
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.15f, 0.15f, 0.15f, 1.0f), &bgBrush);
    }
    
    if (textBrush && bgBrush && badgeBrush) {
        // Draw background
        D2D1_RECT_F bgRect = D2D1::RectF(m_bounds.x, m_bounds.y, 
                                         m_bounds.x + m_bounds.width, 
                                         m_bounds.y + m_bounds.height);
        renderTarget->FillRectangle(bgRect, bgBrush);
        
        // Draw category name with left alignment
        D2D1_RECT_F nameRect = D2D1::RectF(m_bounds.x + 15, m_bounds.y + 8, 
                                           m_bounds.x + m_bounds.width - 50, m_bounds.y + m_bounds.height - 8);
        std::wstring nameText(m_name.begin(), m_name.end());
        renderTarget->DrawText(nameText.c_str(), static_cast<UINT32>(nameText.length()),
                              textFormat, nameRect, textBrush);
        
        // Draw count badge
        D2D1_RECT_F badgeRect = D2D1::RectF(m_bounds.x + m_bounds.width - 40, m_bounds.y + 8, 
                                            m_bounds.x + m_bounds.width - 10, m_bounds.y + m_bounds.height - 8);
        std::wstring countText = std::to_wstring(m_count);
        renderTarget->DrawText(countText.c_str(), static_cast<UINT32>(countText.length()),
                              textFormat, badgeRect, badgeBrush);
        
        textBrush->Release();
        bgBrush->Release();
        badgeBrush->Release();
    }
}

bool UICategoryItem::HandleEvent(const SDL_Event& event) {
    if (!IsVisible()) return false;
    
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        float mouseX = static_cast<float>(event.motion.x);
        float mouseY = static_cast<float>(event.motion.y);
        m_hovered = m_bounds.Contains(mouseX, mouseY);
        return m_hovered;
    }
    
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            float mouseX = static_cast<float>(event.button.x);
            float mouseY = static_cast<float>(event.button.y);
            if (m_bounds.Contains(mouseX, mouseY)) {
                if (m_onClick) {
                    m_onClick();
                }
                return true;
            }
        }
    }
    
    return false;
}

// UISidebar Implementation
UISidebar::UISidebar() {
    // Categories are added in CreateUIElements to avoid duplication
}

void UISidebar::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat, IDWriteTextFormat* regularTextFormat, IDWriteTextFormat* semiboldCenteredTextFormat) {
    if (!IsVisible() || !renderTarget || !textFormat) return;

    ID2D1SolidColorBrush* bgBrush = nullptr;
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.12f, 0.12f, 0.12f, 1.0f), &bgBrush);
    
    if (bgBrush) {
        // Draw sidebar background
        D2D1_RECT_F bgRect = D2D1::RectF(m_bounds.x, m_bounds.y, 
                                         m_bounds.x + m_bounds.width, 
                                         m_bounds.y + m_bounds.height);
        renderTarget->FillRectangle(bgRect, bgBrush);
        
        // Render category items
        float itemHeight = 40.0f;
        float currentY = m_bounds.y + 10;
        
        for (auto& category : m_categories) {
            category->SetBounds(Rect(m_bounds.x, currentY, m_bounds.width, itemHeight));
            category->Render(renderTarget, textFormat, boldTextFormat, regularTextFormat, semiboldCenteredTextFormat);
            currentY += itemHeight + 2;
        }
        
        bgBrush->Release();
    }
}

bool UISidebar::HandleEvent(const SDL_Event& event) {
    if (!IsVisible()) return false;
    
    for (auto& category : m_categories) {
        if (category->HandleEvent(event)) {
            return true;
        }
    }
    
    return false;
}

void UISidebar::AddCategory(const std::string& name, int count) {
    auto categoryItem = std::make_shared<UICategoryItem>(name, count);
    categoryItem->SetOnClick([this, name]() {
        SetSelectedCategory(name);
        if (m_onCategorySelected) {
            m_onCategorySelected(name);
        }
    });
    
    if (name == m_selectedCategory) {
        categoryItem->SetSelected(true);
    }
    
    m_categories.push_back(categoryItem);
}

void UISidebar::SetSelectedCategory(const std::string& name) {
    m_selectedCategory = name;
    for (auto& category : m_categories) {
        // This is a simplified approach - in a real implementation, 
        // you'd need to store the category name in UICategoryItem
        category->SetSelected(false);
    }
    
    // Find and select the correct category
    for (auto& category : m_categories) {
        // For now, we'll just select the first one as an example
        // In a real implementation, you'd compare the category name
        if (name == "All" && category == m_categories[0]) {
            category->SetSelected(true);
            break;
        }
    }
}

// UIToolchainCard Implementation
UIToolchainCard::UIToolchainCard(const std::string& name, const std::string& version, 
                               const std::string& description, const std::string& badge)
    : m_name(name), m_version(version), m_description(description), m_badge(badge) {
    m_installButton = std::make_shared<UIButton>("Install");
    m_installButton->SetStyle("#2A2A2A", "#FFFFFF");
}



void UIToolchainCard::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat, IDWriteTextFormat* regularTextFormat, IDWriteTextFormat* semiboldCenteredTextFormat) {
    if (!IsVisible() || !renderTarget || !textFormat) return;

    ID2D1SolidColorBrush* textBrush = nullptr;
    ID2D1SolidColorBrush* bgBrush = nullptr;
    ID2D1SolidColorBrush* badgeBrush = nullptr;
    
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.9f, 0.9f, 1.0f), &textBrush);
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.18f, 0.18f, 0.18f, 1.0f), &bgBrush);
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.2f, 0.6f, 1.0f, 1.0f), &badgeBrush);
    
    if (textBrush && bgBrush && badgeBrush) {
        // Draw card background
        D2D1_RECT_F bgRect = D2D1::RectF(m_bounds.x, m_bounds.y, 
                                         m_bounds.x + m_bounds.width, 
                                         m_bounds.y + m_bounds.height);
        renderTarget->FillRectangle(bgRect, bgBrush);
        
        // Draw icon placeholder (simplified as a circle)
        ID2D1SolidColorBrush* iconBrush = nullptr;
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.4f, 0.4f, 0.4f, 1.0f), &iconBrush);
        if (iconBrush) {
            D2D1_ELLIPSE iconEllipse = D2D1::Ellipse(D2D1::Point2F(m_bounds.x + 30, m_bounds.y + 30), 20, 20);
            renderTarget->FillEllipse(iconEllipse, iconBrush);
            iconBrush->Release();
        }
        
        // Draw toolchain name with left alignment
        D2D1_RECT_F nameRect = D2D1::RectF(m_bounds.x + 70, m_bounds.y + 15, 
                                           m_bounds.x + m_bounds.width - 150, m_bounds.y + 35);
        std::wstring nameText(m_name.begin(), m_name.end());
        renderTarget->DrawText(nameText.c_str(), static_cast<UINT32>(nameText.length()),
                              textFormat, nameRect, textBrush);
        
        // Draw version with left alignment
        D2D1_RECT_F versionRect = D2D1::RectF(m_bounds.x + 70, m_bounds.y + 40, 
                                              m_bounds.x + m_bounds.width - 150, m_bounds.y + 55);
        std::wstring versionText(m_version.begin(), m_version.end());
        renderTarget->DrawText(versionText.c_str(), static_cast<UINT32>(versionText.length()),
                              textFormat, versionRect, textBrush);
        
        // Draw description with left alignment
        D2D1_RECT_F descRect = D2D1::RectF(m_bounds.x + 70, m_bounds.y + 60, 
                                           m_bounds.x + m_bounds.width - 150, m_bounds.y + 90);
        std::wstring descText(m_description.begin(), m_description.end());
        renderTarget->DrawText(descText.c_str(), static_cast<UINT32>(descText.length()),
                              textFormat, descRect, textBrush);
        
        // Draw badge
        D2D1_RECT_F badgeRect = D2D1::RectF(m_bounds.x + m_bounds.width - 140, m_bounds.y + 15, 
                                            m_bounds.x + m_bounds.width - 80, m_bounds.y + 35);
        renderTarget->FillRectangle(badgeRect, badgeBrush);
        
        std::wstring badgeText(m_badge.begin(), m_badge.end());
        renderTarget->DrawText(badgeText.c_str(), static_cast<UINT32>(badgeText.length()),
                              textFormat, badgeRect, textBrush);
        
        // Position and render install button
        float buttonWidth = 70;
        float buttonHeight = 30;
        m_installButton->SetBounds(Rect(m_bounds.x + m_bounds.width - buttonWidth - 10, 
                                       m_bounds.y + m_bounds.height - buttonHeight - 10, 
                                       buttonWidth, buttonHeight));
        m_installButton->Render(renderTarget, textFormat, boldTextFormat, regularTextFormat, semiboldCenteredTextFormat);
        
        textBrush->Release();
        bgBrush->Release();
        badgeBrush->Release();
    }
}

bool UIToolchainCard::HandleEvent(const SDL_Event& event) {
    if (!IsVisible()) return false;
    
    if (m_installButton->HandleEvent(event)) {
        if (m_onInstall) {
            m_onInstall(m_name);
        }
        return true;
    }
    
    return false;
}

// UIContent Implementation
UIContent::UIContent() {
    // Constructor is now empty - cards are added in CreateUIElements
}

void UIContent::Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat, IDWriteTextFormat* regularTextFormat, IDWriteTextFormat* semiboldCenteredTextFormat) {
    if (!IsVisible() || !renderTarget || !textFormat) return;

    ID2D1SolidColorBrush* bgBrush = nullptr;
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.1f, 0.1f, 0.1f, 1.0f), &bgBrush);
    
    if (bgBrush) {
        // Draw content background
        D2D1_RECT_F bgRect = D2D1::RectF(m_bounds.x, m_bounds.y, 
                                         m_bounds.x + m_bounds.width, 
                                         m_bounds.y + m_bounds.height);
        renderTarget->FillRectangle(bgRect, bgBrush);
        
        // Render toolchain cards
        float cardHeight = 120.0f;
        float cardSpacing = 10.0f;
        float currentY = m_bounds.y + 20 - (m_scrollOffset * (cardHeight + cardSpacing));
        
        for (auto& card : m_cards) {
            if (currentY + cardHeight > m_bounds.y && currentY < m_bounds.y + m_bounds.height) {
                card->SetBounds(Rect(m_bounds.x + 20, currentY, m_bounds.width - 40, cardHeight));
                card->Render(renderTarget, textFormat, boldTextFormat, regularTextFormat, semiboldCenteredTextFormat);
            }
            currentY += cardHeight + cardSpacing;
        }
        
        bgBrush->Release();
    }
}

bool UIContent::HandleEvent(const SDL_Event& event) {
    if (!IsVisible()) return false;
    
    for (auto& card : m_cards) {
        if (card->HandleEvent(event)) {
            return true;
        }
    }
    
    // Handle scrolling
    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        float mouseX = static_cast<float>(event.wheel.mouse_x);
        float mouseY = static_cast<float>(event.wheel.mouse_y);
        if (m_bounds.Contains(mouseX, mouseY)) {
            m_scrollOffset -= event.wheel.y;
            if (m_scrollOffset < 0) m_scrollOffset = 0;
            return true;
        }
    }
    
    return false;
}

void UIContent::AddCard(std::shared_ptr<UIToolchainCard> card) {
    m_cards.push_back(card);
}

void UIContent::ClearCards() {
    m_cards.clear();
}

void UIContent::FilterByCategory(const std::string& category) {
    m_currentCategory = category;
    // In a real implementation, you would filter the cards based on category
    // For now, we'll just store the current category
}

} // namespace Toolchain
} // namespace Hyperion