#pragma once

#ifdef __linux__

#include "../../ui_interface.hpp"
#include <gtk/gtk.h>
#include <cairo.h>
#include <atomic>
#include <string>

class LinuxModernDialog : public IModernDialog {
private:
    GtkWidget* m_window;
    GtkWidget* m_parentWindow;
    GtkWidget* m_drawingArea;
    GtkWidget* m_progressBar;
    GtkWidget* m_statusLabel;
    GtkWidget* m_titleLabel;
    GtkWidget* m_detailLabel;
    GtkWidget* m_cancelButton;
    GtkWidget* m_vbox;
    GtkWidget* m_hbox;
    
    // State
    std::atomic<bool> m_cancelled;
    std::string m_statusText;
    std::string m_titleText;
    int m_progress;
    size_t m_bytesDownloaded;
    size_t m_totalBytes;
    size_t m_downloadSpeed;
    
    // Animation
    double m_animationProgress;
    guint m_animationTimer;
    
    // Callbacks
    static void OnCancelClicked(GtkWidget* widget, gpointer user_data);
    static gboolean OnDraw(GtkWidget* widget, cairo_t* cr, gpointer user_data);
    static gboolean OnAnimationTimer(gpointer user_data);
    static void OnDestroy(GtkWidget* widget, gpointer user_data);
    static gboolean OnDeleteEvent(GtkWidget* widget, GdkEvent* event, gpointer user_data);
    
    void SetupUI();
    void UpdateProgressText();
    void DrawModernBackground(cairo_t* cr, int width, int height);
    void DrawCard(cairo_t* cr, int width, int height);
    void UpdateAnimation();
    
    // Utility methods
    std::string FormatBytes(size_t bytes);
    std::string FormatSpeed(size_t bytesPerSecond);
    
public:
    LinuxModernDialog();
    ~LinuxModernDialog() override;
    
    // IModernDialog implementation
    bool Create(PlatformInstance instance, PlatformWindow parent, const std::wstring& title) override;
    void Show() override;
    void Hide() override;
    void SetProgress(int percentage) override;
    void SetStatus(const std::string& status) override;
    void SetDownloadInfo(size_t bytesDownloaded, size_t totalBytes, size_t speed) override;
    void UpdateProgress(int percentage, const std::wstring& status, size_t bytesDownloaded, size_t totalBytes) override;
    bool IsCancelled() const override { return m_cancelled.load(); }
    PlatformWindow GetNativeHandle() const override { return m_window; }
};

#endif // __linux__