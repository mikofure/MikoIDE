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
#pragma comment(lib, "user32.lib")
#endif

namespace Hyperion {
namespace Toolchain {

// Forward declarations
class ToolchainManager;
class UIButton;
class UIHeader;
class UICategoryItem;
class UISidebar;
class UIToolchainCard;
class UIContent;

// Win32 Menu IDs
enum MenuID {
    // File Menu
    ID_FILE_NEW_PROJECT = 1001,
    ID_FILE_OPEN_PROJECT = 1002,
    ID_FILE_SAVE_PROJECT = 1003,
    ID_FILE_CLOSE_PROJECT = 1004,
    ID_FILE_EXIT = 1005,
    
    // Edit Menu
    ID_EDIT_UNDO = 2001,
    ID_EDIT_REDO = 2002,
    ID_EDIT_CUT = 2003,
    ID_EDIT_COPY = 2004,
    ID_EDIT_PASTE = 2005,
    ID_EDIT_FIND = 2006,
    ID_EDIT_REPLACE = 2007,
    ID_EDIT_PREFERENCES = 2008,
    
    // Build Menu
    ID_BUILD_BUILD = 3001,
    ID_BUILD_REBUILD = 3002,
    ID_BUILD_CLEAN = 3003,
    ID_BUILD_RUN = 3004,
    ID_BUILD_DEBUG = 3005,
    ID_BUILD_STOP = 3006,
    ID_BUILD_CONFIGURATION = 3007,
    
    // Tools Menu
    ID_TOOLS_TOOLCHAIN_MANAGER = 4001,
    ID_TOOLS_PACKAGE_MANAGER = 4002,
    ID_TOOLS_TERMINAL = 4003,
    ID_TOOLS_OPTIONS = 4004,
    
    // Help Menu
    ID_HELP_DOCUMENTATION = 5001,
    ID_HELP_KEYBOARD_SHORTCUTS = 5002,
    ID_HELP_ABOUT = 5003
};

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
    virtual void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat = nullptr, IDWriteTextFormat* regularTextFormat = nullptr, IDWriteTextFormat* semiboldCenteredTextFormat = nullptr) = 0;
    virtual bool HandleEvent(const SDL_Event& event) = 0;
    virtual void SetBounds(const Rect& bounds) { m_bounds = bounds; }
    virtual Rect GetBounds() const { return m_bounds; }
    virtual void SetVisible(bool visible) { m_visible = visible; }
    virtual bool IsVisible() const { return m_visible; }

protected:
    Rect m_bounds;
    bool m_visible = true;
};



// Text label widget
class UILabel : public UIWidget {
public:
    UILabel(const std::string& text);
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat = nullptr, IDWriteTextFormat* regularTextFormat = nullptr, IDWriteTextFormat* semiboldCenteredTextFormat = nullptr) override;
    bool HandleEvent(const SDL_Event& event) override;
    void SetText(const std::string& text) { m_text = text; }

private:
    std::string m_text;
};

// List widget for displaying toolchains/projects
class UIList : public UIWidget {
public:
    UIList();
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat = nullptr, IDWriteTextFormat* regularTextFormat = nullptr, IDWriteTextFormat* semiboldCenteredTextFormat = nullptr) override;
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
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat = nullptr, IDWriteTextFormat* regularTextFormat = nullptr, IDWriteTextFormat* semiboldCenteredTextFormat = nullptr) override;
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
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat = nullptr, IDWriteTextFormat* regularTextFormat = nullptr, IDWriteTextFormat* semiboldCenteredTextFormat = nullptr) override;
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
    IDWriteTextFormat* GetCenteredTextFormat() const { return m_centeredTextFormat; }
    IDWriteTextFormat* GetBoldTextFormat() const { return m_boldTextFormat; }

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
    
    // Win32 Menu methods
    bool CreateMenuBar();
    void HandleMenuCommand(WPARAM wParam);
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // SDL and Windows resources
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    HWND m_hwnd;
    HMENU m_menuBar;
    WNDPROC m_originalWndProc;

    // Direct2D resources
    ID2D1Factory* m_d2dFactory;
    ID2D1HwndRenderTarget* m_renderTarget;
    ID2D1SolidColorBrush* m_brush;

    // DirectWrite resources
    IDWriteFactory* m_writeFactory;
    IDWriteTextFormat* m_textFormat;
    IDWriteTextFormat* m_centeredTextFormat;
    IDWriteTextFormat* m_boldTextFormat;
    IDWriteTextFormat* m_semiboldCenteredTextFormat;
    IDWriteTextFormat* m_regularTextFormat;

    // UI state
    UITheme m_theme;
    std::vector<std::shared_ptr<UIWidget>> m_widgets;
    std::shared_ptr<UIWidget> m_focusedWidget;
    
    // UI Panels
    std::shared_ptr<UIPanel> m_mainPanel;
    std::shared_ptr<UIPanel> m_headerPanel;
    std::shared_ptr<UIPanel> m_sidebarPanel;
    std::shared_ptr<UIPanel> m_mainContentPanel;
    std::shared_ptr<UIPanel> m_toolchainPanel;
    std::shared_ptr<UIPanel> m_projectPanel;
    std::shared_ptr<UIPanel> m_projectActionsPanel;
    std::shared_ptr<UIPanel> m_buildActionsPanel;
    std::shared_ptr<UIPanel> m_consolePanel;
    std::shared_ptr<UIPanel> m_statusPanel;
    
    // UI Elements
    std::shared_ptr<UILabel> m_titleLabel;
    std::shared_ptr<UILabel> m_toolchainLabel;
    std::shared_ptr<UILabel> m_projectLabel;
    std::shared_ptr<UILabel> m_consoleLabel;
    std::shared_ptr<UILabel> m_progressLabel;
    std::shared_ptr<UIList> m_toolchainList;
    std::shared_ptr<UIList> m_projectList;
    std::shared_ptr<UILabel> m_consoleOutput;
    std::shared_ptr<UILabel> m_statusLabel;
    


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
    
    // New UI components for redesigned interface
    std::shared_ptr<UIHeader> m_header;
    std::shared_ptr<UISidebar> m_sidebar;
    std::shared_ptr<UIContent> m_content;
};

// New UI classes for the redesigned interface
class UIButton : public UIWidget {
public:
    UIButton(const std::string& text, std::function<void()> onClick = nullptr);
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat = nullptr, IDWriteTextFormat* regularTextFormat = nullptr, IDWriteTextFormat* semiboldCenteredTextFormat = nullptr) override;
    bool HandleEvent(const SDL_Event& event) override;
    void SetOnClick(std::function<void()> onClick) { m_onClick = onClick; }
    void SetStyle(const std::string& bgColor, const std::string& textColor);

private:
    std::string m_text;
    std::function<void()> m_onClick;
    bool m_hovered = false;
    bool m_pressed = false;
    struct {
        float r, g, b, a;
    } m_bgColor = {0.85f, 0.85f, 0.85f, 1.0f}; // Default light gray
    struct {
        float r, g, b, a;
    } m_textColor = {0.0f, 0.0f, 0.0f, 1.0f}; // Default black
};

class UIHeader : public UIWidget {
public:
    UIHeader(const std::string& path = "");
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat = nullptr, IDWriteTextFormat* regularTextFormat = nullptr, IDWriteTextFormat* semiboldCenteredTextFormat = nullptr);
    bool HandleEvent(const SDL_Event& event) override;
    void SetOnChangeDirectory(std::function<void()> callback) { m_onChangeDirectory = callback; }

private:
    std::string m_path;
    std::shared_ptr<UIButton> m_changeDirectoryButton;
    std::function<void()> m_onChangeDirectory;
};

class UICategoryItem : public UIWidget {
public:
    UICategoryItem(const std::string& name, int count = 0);
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat = nullptr, IDWriteTextFormat* regularTextFormat = nullptr, IDWriteTextFormat* semiboldCenteredTextFormat = nullptr) override;
    bool HandleEvent(const SDL_Event& event) override;
    void SetSelected(bool selected) { m_selected = selected; }
    void SetOnClick(std::function<void()> onClick) { m_onClick = onClick; }

private:
    std::string m_name;
    int m_count;
    bool m_selected = false;
    bool m_hovered = false;
    std::function<void()> m_onClick;
};

class UISidebar : public UIWidget {
public:
    UISidebar();
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat = nullptr, IDWriteTextFormat* regularTextFormat = nullptr, IDWriteTextFormat* semiboldCenteredTextFormat = nullptr) override;
    bool HandleEvent(const SDL_Event& event) override;
    void AddCategory(const std::string& name, int count);
    void SetSelectedCategory(const std::string& name);
    void SetOnCategorySelected(std::function<void(const std::string&)> callback) { m_onCategorySelected = callback; }

private:
    std::vector<std::shared_ptr<UICategoryItem>> m_categories;
    std::string m_selectedCategory = "All";
    std::function<void(const std::string&)> m_onCategorySelected;
};

class UIToolchainCard : public UIWidget {
public:
    UIToolchainCard(const std::string& name, const std::string& version, 
                   const std::string& description, const std::string& badge);
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat = nullptr, IDWriteTextFormat* regularTextFormat = nullptr, IDWriteTextFormat* semiboldCenteredTextFormat = nullptr) override;
    bool HandleEvent(const SDL_Event& event) override;
    void SetOnInstall(std::function<void(const std::string&)> callback) { m_onInstall = callback; }

private:
    std::string m_name;
    std::string m_version;
    std::string m_description;
    std::string m_badge;
    std::shared_ptr<UIButton> m_installButton;
    std::function<void(const std::string&)> m_onInstall;
};

class UIContent : public UIWidget {
public:
    UIContent();
    void Render(ID2D1RenderTarget* renderTarget, IDWriteTextFormat* textFormat, IDWriteTextFormat* boldTextFormat = nullptr, IDWriteTextFormat* regularTextFormat = nullptr, IDWriteTextFormat* semiboldCenteredTextFormat = nullptr) override;
    bool HandleEvent(const SDL_Event& event) override;
    void AddCard(std::shared_ptr<UIToolchainCard> card);
    void ClearCards();
    void FilterByCategory(const std::string& category);

private:
    std::vector<std::shared_ptr<UIToolchainCard>> m_cards;
    std::string m_currentCategory = "All";
    int m_scrollOffset = 0;
};

} // namespace Toolchain
} // namespace Hyperion