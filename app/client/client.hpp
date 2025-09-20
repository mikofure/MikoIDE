#pragma once

// Windows includes for dwmapi
#include <windows.h>
#include <dwmapi.h>

// SDL3 includes
#include <SDL3/SDL.h>

// CEF includes
#include "include/cef_client.h"
#include "include/cef_display_handler.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/cef_context_menu_handler.h"
#include "include/cef_drag_handler.h"
#include "include/cef_request_handler.h"
#include "include/cef_keyboard_handler.h"
#include "include/cef_download_handler.h"
#include "include/cef_render_handler.h"
#include "include/wrapper/cef_message_router.h"
#include "include/cef_task.h"
#include "../resources/resources.hpp"
#include "../renderer/dx11_renderer.hpp"
#include <list>
#include <memory>
#include <string>

// Forward declarations
std::string GetDataURI(const std::string& data, const std::string& mime_type);
std::string GetDownloadPath(const std::string& suggested_name);

class SimpleClient;

// Task class for CefPostTask compatibility
class CloseBrowserTask : public CefTask {
public:
    CloseBrowserTask(CefRefPtr<SimpleClient> client, bool force_close);
    void Execute() override;

private:
    CefRefPtr<SimpleClient> client_;
    bool force_close_;
    IMPLEMENT_REFCOUNTING(CloseBrowserTask);
};

// SDL3 Window wrapper with CEF OSR integration and dwmapi rounded edges
class SDL3Window {
public:
    SDL3Window();
    ~SDL3Window();

    bool Initialize(int width, int height);
    void Shutdown();
    
    // Window management
    void Show();
    void Hide();
    void Minimize();
    void Maximize();
    void Restore();
    void Close();
    
    // Rounded corners using dwmapi
    void ApplyRoundedCorners();
    void SetBorderless(bool borderless);
    
    // Event handling
    bool HandleEvent(const SDL_Event& event);
    bool HandleWindowDragging(const SDL_Event& event);
    void Render();
    
    // Getters
    SDL_Window* GetSDLWindow() const { return window_; }
    SDL_Renderer* GetRenderer() const { return renderer_; }
    SDL_Texture* GetTexture() const { return texture_; }
    HWND GetHWND() const;
    
    // CEF OSR integration
    void UpdateTexture(const void* buffer, int width, int height);
    void Resize(int width, int height);
    void SetClient(CefRefPtr<SimpleClient> client) { client_ = client; }
    
    // DX11 Renderer integration
    bool IsDX11Available() const { return dx11_renderer_ && dx11_renderer_->IsInitialized(); }
    void EnableDX11Rendering(bool enable);
    DX11Renderer* GetDX11Renderer() const { return dx11_renderer_.get(); }
    
    // Window state
    bool IsMinimized() const { return minimized_; }
    bool IsMaximized() const { return maximized_; }
    bool ShouldClose() const { return should_close_; }
    
    // DPI and scaling support
    float GetDPIScale() const { return dpi_scale_; }
    void UpdateDPIScale();
    int GetScaledWidth() const { return static_cast<int>(width_ * dpi_scale_); }
    int GetScaledHeight() const { return static_cast<int>(height_ * dpi_scale_); }
    
    // Mouse and keyboard input for CEF
    void SendMouseEvent(const SDL_Event& event);
    void SendKeyEvent(const SDL_Event& event);
    void SendScrollEvent(const SDL_Event& event);

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
    HWND hwnd_;
    
    CefRefPtr<SimpleClient> client_;
    
    int width_;
    int height_;
    bool minimized_;
    bool maximized_;
    bool should_close_;
    bool borderless_;
    
    // Mouse state
    bool mouse_captured_;
    int last_mouse_x_;
    int last_mouse_y_;
    
    // Window dragging state
    bool is_dragging_;
    int drag_start_x_;
    int drag_start_y_;
    int window_start_x_;
    int window_start_y_;
    
    // DX11 Renderer for performance optimization
    std::unique_ptr<DX11Renderer> dx11_renderer_;
    bool dx11_enabled_;
    
    // DPI scaling support
    float dpi_scale_;
    
    void InitializeDwmApi();
    void UpdateWindowStyle();
};

// CEF OSR Render Handler
class OSRRenderHandler : public CefRenderHandler {
public:
    OSRRenderHandler(SDL3Window* window);
    
    // CefRenderHandler methods
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    void OnPaint(CefRefPtr<CefBrowser> browser,
                 PaintElementType type,
                 const RectList& dirtyRects,
                 const void* buffer,
                 int width,
                 int height) override;

    bool StartDragging(CefRefPtr<CefBrowser> browser,
                      CefRefPtr<CefDragData> drag_data,
                      CefRenderHandler::DragOperationsMask allowed_ops,
                      int x, int y) override;
    void UpdateDragCursor(CefRefPtr<CefBrowser> browser,
                         CefRenderHandler::DragOperation operation) override;

private:
    SDL3Window* window_;
    IMPLEMENT_REFCOUNTING(OSRRenderHandler);
};

// Simple CEF client implementation with OSR support
class SimpleClient : public CefClient,
                     public CefDisplayHandler,
                     public CefLifeSpanHandler,
                     public CefLoadHandler,
                     public CefContextMenuHandler,
                     public CefDragHandler,
                     public CefRequestHandler,
                     public CefKeyboardHandler,
                     public CefDownloadHandler,
                     public CefMessageRouterBrowserSide::Handler {
public:
    SimpleClient(SDL3Window* window);

    // CefClient methods
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override;
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override;
    virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override;
    virtual CefRefPtr<CefDragHandler> GetDragHandler() override;
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override;
    virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() override;
    virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() override;
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override;
    
    // Draggable region support
    bool IsPointInDragRegion(int x, int y) const;

    // CefMessageRouterBrowserSide::Handler methods
    virtual bool OnQuery(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        int64_t query_id,
                        const CefString& request,
                        bool persistent,
                        CefRefPtr<CefMessageRouterBrowserSide::Callback> callback) override;

    // CefDisplayHandler methods
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                              const CefString& title) override;
    virtual bool OnCursorChange(CefRefPtr<CefBrowser> browser,
                               CefCursorHandle cursor,
                               cef_cursor_type_t type,
                               const CefCursorInfo& custom_cursor_info) override;

    // CefLifeSpanHandler methods
    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             int popup_id,
                             const CefString& target_url,
                             const CefString& target_frame_name,
                             CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                             bool user_gesture,
                             const CefPopupFeatures& popupFeatures,
                             CefWindowInfo& windowInfo,
                             CefRefPtr<CefClient>& client,
                             CefBrowserSettings& settings,
                             CefRefPtr<CefDictionaryValue>& extra_info,
                             bool* no_javascript_access) override;
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler methods
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            ErrorCode errorCode,
                            const CefString& errorText,
                            const CefString& failedUrl) override;
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            TransitionType transition_type) override;

    // CefContextMenuHandler methods
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefContextMenuParams> params,
                                   CefRefPtr<CefMenuModel> model) override;
    
    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefContextMenuParams> params,
                                    int command_id,
                                    EventFlags event_flags) override;

    // CefDragHandler methods
    virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefDragData> dragData,
                           CefDragHandler::DragOperationsMask mask) override;
    
    virtual void OnDraggableRegionsChanged(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const std::vector<CefDraggableRegion>& regions) override;

    // CefRequestHandler methods
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefRequest> request,
                              bool user_gesture,
                              bool is_redirect) override;
                              
    virtual bool OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 const CefString& target_url,
                                 CefRequestHandler::WindowOpenDisposition target_disposition,
                                 bool user_gesture) override;
    
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefProcessId source_process,
                                         CefRefPtr<CefProcessMessage> message) override;

    // CefRequestHandler methods
    virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request);

    // CefKeyboardHandler methods
    virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                             const CefKeyEvent& event,
                             CefEventHandle os_event,
                             bool* is_keyboard_shortcut) override;

    // CefDownloadHandler methods
    virtual bool OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefDownloadItem> download_item,
                                const CefString& suggested_name,
                                CefRefPtr<CefBeforeDownloadCallback> callback) override;
    virtual void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefDownloadItem> download_item,
                                 CefRefPtr<CefDownloadItemCallback> callback) override;

    // Browser management
    void CloseAllBrowsers(bool force_close);
    void DoCloseAllBrowsers(bool force_close);
    CefRefPtr<CefBrowser> GetFirstBrowser();
    bool HasBrowsers();
    void SpawnNewWindow();
    
    // OSR specific methods
    void SendMouseClickEvent(int x, int y, CefBrowserHost::MouseButtonType button, bool mouse_up, int click_count);
    void SendMouseMoveEvent(int x, int y, bool mouse_leave);
    void SendMouseWheelEvent(int x, int y, int delta_x, int delta_y);
    void SendKeyEvent(const CefKeyEvent& event);
    void SendFocusEvent(bool set_focus);

private:
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browser_list_;
    
    // Message router for handling JavaScript queries
    CefRefPtr<CefMessageRouterBrowserSide> message_router_;
    
    // Binary resource provider for handling miko:// protocol
    CefRefPtr<BinaryResourceProvider> resource_provider_;
    
    // OSR render handler
    CefRefPtr<OSRRenderHandler> render_handler_;
    
    // SDL3 window reference
    SDL3Window* window_;
    
    // Draggable regions for window movement
    std::vector<CefDraggableRegion> draggable_regions_;

    IMPLEMENT_REFCOUNTING(SimpleClient);
};