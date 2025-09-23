#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

#include <SDL3/SDL.h>

#ifdef _WIN32
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#endif

namespace Hyperion {
namespace Toolchain {

// Forward declarations
class ToolchainManager;

// UI Theme colors
struct UITheme {
    struct {
        float r, g, b, a;
    } background = {0.1f, 0.1f, 0.1f, 1.0f};
    
    struct {
        float r, g, b, a;
    } surface = {0.15f, 0.15f, 0.15f, 1.0f};
    
    struct {
        float r, g, b, a;
    } primary = {0.2f, 0.6f, 1.0f, 1.0f};
    
    struct {
        float r, g, b, a;
    } secondary = {0.4f, 0.4f, 0.4f, 1.0f};
    
    struct {
        float r, g, b, a;
    } text = {0.9f, 0.9f, 0.9f, 1.0f};
    
    struct {
        float r, g, b, a;
    } textSecondary = {0.7f, 0.7f, 0.7f, 1.0f};
    
    struct {
        float r, g, b, a;
    } accent = {0.0f, 0.8f, 0.4f, 1.0f};
    
    struct {
        float r, g, b, a;
    } warning = {1.0f, 0.6f, 0.0f, 1.0f};
    
    struct {
        float r, g, b, a;
    } error = {1.0f, 0.3f, 0.3f, 1.0f};
};

// UI Layout structures
struct Rect {
    float x, y, width, height;
    
    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
    
    bool Contains(float px, float py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
};

struct Point {
    float x, y;
    Point() : x(0), y(0) {}
    Point(float x, float y) : x(x), y(y) {}
};

// UI Widget base class
class UIWidget {
public:
    virtual ~UIWidget() = default;
    virtual void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat) = 0;
    virtual bool HandleEvent(const SDL_Event& event) = 0;
    virtual void SetBounds(const Rect& bounds) { m_bounds = bounds; }
    virtual Rect GetBounds() const { return m_bounds; }
    virtual void SetVisible(bool visible) { m_visible = visible; }
    virtual bool IsVisible() const { return m_visible; }

protected:
    Rect m_bounds;
    bool m_visible = true;
};

// Button widget
class UIButton : public UIWidget {
public:
    UIButton(const std::string& text, std::function<void()> onClick = nullptr);
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat) override;
    bool HandleEvent(const SDL_Event& event) override;
    void SetText(const std::string& text) { m_text = text; }
    void SetOnClick(std::function<void()> onClick) { m_onClick = onClick; }

private:
    std::string m_text;
    std::function<void()> m_onClick;
    bool m_hovered = false;
    bool m_pressed = false;
};

// Text label widget
class UILabel : public UIWidget {
public:
    UILabel(const std::string& text);
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat) override;
    bool HandleEvent(const SDL_Event& event) override;
    void SetText(const std::string& text) { m_text = text; }

private:
    std::string m_text;
};

// List widget for displaying toolchains/projects
class UIList : public UIWidget {
public:
    UIList();
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat) override;
    bool HandleEvent(const SDL_Event& event) override;
    void AddItem(const std::string& item);
    void RemoveItem(const std::string& item);
    void Clear();
    void SetSelectedIndex(int index) { m_selectedIndex = index; }
    int GetSelectedIndex() const { return m_selectedIndex; }
    std::string GetSelectedItem() const;
    void SetOnSelectionChanged(std::function<void(int, const std::string&)> callback) { m_onSelectionChanged = callback; }

private:
    std::vector<std::string> m_items;
    int m_selectedIndex = -1;
    int m_scrollOffset = 0;
    std::function<void(int, const std::string&)> m_onSelectionChanged;
};

// Text input widget
class UITextInput : public UIWidget {
public:
    UITextInput(const std::string& placeholder = "");
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat) override;
    bool HandleEvent(const SDL_Event& event) override;
    void SetText(const std::string& text) { m_text = text; }
    std::string GetText() const { return m_text; }
    void SetPlaceholder(const std::string& placeholder) { m_placeholder = placeholder; }

private:
    std::string m_text;
    std::string m_placeholder;
    bool m_focused = false;
    size_t m_cursorPosition = 0;
};

// Panel widget for grouping other widgets
class UIPanel : public UIWidget {
public:
    UIPanel();
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat) override;
    bool HandleEvent(const SDL_Event& event) override;
    void AddWidget(std::shared_ptr<UIWidget> widget);
    void RemoveWidget(std::shared_ptr<UIWidget> widget);
    void Clear();

private:
    std::vector<std::shared_ptr<UIWidget>> m_widgets;
};

// Main windowed UI class
class WindowedUI {
public:
    WindowedUI();
    ~WindowedUI();

    // Initialization and cleanup
    bool Initialize(const std::string& title, int width, int height);
    void Shutdown();

    // Event handling
    bool HandleEvent(const SDL_Event& event);

    // Rendering
    void Render();
    void BeginFrame();
    void EndFrame();

    // Window management
    void SetTitle(const std::string& title);
    void SetSize(int width, int height);
    void GetSize(int& width, int& height) const;
    bool IsMinimized() const;
    void Minimize();
    void Maximize();
    void Restore();

    // UI Management
    void AddWidget(std::shared_ptr<UIWidget> widget);
    void RemoveWidget(std::shared_ptr<UIWidget> widget);
    void ClearWidgets();

    // Theme management
    void SetTheme(const UITheme& theme) { m_theme = theme; }
    const UITheme& GetTheme() const { return m_theme; }

    // Direct2D/DirectWrite access
    ID2D1RenderTarget* GetRenderTarget() const { return m_renderTarget; }
    IDWriteTextFormat* GetTextFormat() const { return m_textFormat; }

    // Layout helpers
    void SetupMainLayout();
    void UpdateLayout();

    // Toolchain-specific UI methods
    void ShowToolchainList(const std::vector<std::string>& toolchains);
    void ShowProjectList(const std::vector<std::string>& projects);
    void ShowConsoleOutput(const std::string& output);
    void SetStatusText(const std::string& status);

    // Event callback type
    using UIEventCallback = std::function<void(const std::string&, const std::string&)>;

    // Event callback setters
    void SetToolchainSelectedCallback(UIEventCallback callback) { m_onToolchainSelected = callback; }
    void SetProjectSelectedCallback(UIEventCallback callback) { m_onProjectSelected = callback; }
    void SetBuildRequestedCallback(std::function<void()> callback) { m_onBuildRequested = callback; }
    void SetRunRequestedCallback(std::function<void()> callback) { m_onRunRequested = callback; }
    void SetNewProjectCallback(std::function<void()> callback) { m_onNewProject = callback; }
    void SetOpenProjectCallback(std::function<void()> callback) { m_onOpenProject = callback; }

private:
    // Internal initialization methods
    bool InitializeSDL(const std::string& title, int width, int height);
    bool InitializeDirect2D();
    bool InitializeDirectWrite();
    void CreateUIElements();

    // Internal rendering methods
    void RenderBackground();
    void RenderWidgets();
    void RenderStatusBar();

    // Helper methods
    D2D1_COLOR_F ColorFromTheme(float r, float g, float b, float a) const;
    void HandleMouseEvent(const SDL_Event& event);
    void HandleKeyboardEvent(const SDL_Event& event);

    // SDL and Windows resources
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    HWND m_hwnd;

    // Direct2D resources
    ID2D1Factory* m_d2dFactory;
    ID2D1HwndRenderTarget* m_renderTarget;
    ID2D1SolidColorBrush* m_brush;

    // DirectWrite resources
    IDWriteFactory* m_writeFactory;
    IDWriteTextFormat* m_textFormat;

    // UI state
    UITheme m_theme;
    std::vector<std::shared_ptr<UIWidget>> m_widgets;
    std::shared_ptr<UIWidget> m_focusedWidget;
    
    // Layout panels
    std::shared_ptr<UIPanel> m_mainPanel;
    std::shared_ptr<UIPanel> m_toolchainPanel;
    std::shared_ptr<UIPanel> m_projectPanel;
    std::shared_ptr<UIPanel> m_consolePanel;
    
    // UI elements
    std::shared_ptr<UIList> m_toolchainList;
    std::shared_ptr<UIList> m_projectList;
    std::shared_ptr<UILabel> m_consoleOutput;
    std::shared_ptr<UILabel> m_statusLabel;
    std::shared_ptr<UIButton> m_buildButton;
    std::shared_ptr<UIButton> m_runButton;
    std::shared_ptr<UIButton> m_newProjectButton;
    std::shared_ptr<UIButton> m_openProjectButton;

    // Event callbacks
    UIEventCallback m_onToolchainSelected;
    UIEventCallback m_onProjectSelected;
    std::function<void()> m_onBuildRequested;
    std::function<void()> m_onRunRequested;
    std::function<void()> m_onNewProject;
    std::function<void()> m_onOpenProject;

    // Window state
    int m_width, m_height;
    bool m_initialized;
    bool m_minimized;
};

} // namespace Toolchain
} // namespace Hyperion