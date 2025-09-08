#pragma once
#include "include/cef_client.h"
#include "include/cef_display_handler.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/cef_context_menu_handler.h"
#include "include/cef_drag_handler.h"
#include "include/cef_request_handler.h"
#include "include/cef_keyboard_handler.h"
#include "include/cef_download_handler.h"
#include "include/wrapper/cef_message_router.h"
#include "include/cef_task.h"
#include "binaryresourceprovider.hpp"
#include <list>

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

// Simple CEF client implementation
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
    SimpleClient();

    // CefClient methods
  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override;
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override;
  virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override;
  virtual CefRefPtr<CefDragHandler> GetDragHandler() override;
  virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override;
  virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() override;
  virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() override;

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

private:
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browser_list_;
    
    // Message router for handling JavaScript queries
    CefRefPtr<CefMessageRouterBrowserSide> message_router_;
    
    // Binary resource provider for handling miko:// protocol
    CefRefPtr<BinaryResourceProvider> resource_provider_;

    IMPLEMENT_REFCOUNTING(SimpleClient);
};