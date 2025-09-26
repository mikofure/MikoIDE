#pragma once

#ifdef __linux__

#include "../../ui_interface.hpp"
#include <gtk/gtk.h>
#include <cairo.h>
#include <string>
#include <memory>

class LinuxSplashScreen : public ISplashScreen {
private:
    GtkWidget* m_window;
    GtkWidget* m_drawingArea;
    GtkWidget* m_statusLabel;
    GtkWidget* m_titleLabel;
    GtkWidget* m_vbox;
    
    std::string m_statusText;
    std::string m_titleText;
    bool m_visible;
    
    // Animation support
    double m_animationProgress;
    guint m_animationTimer;
    
    // Cairo surfaces for custom drawing
    cairo_surface_t* m_logoSurface;
    
    // Callbacks
    static gboolean OnDraw(GtkWidget* widget, cairo_t* cr, gpointer user_data);
    static gboolean OnAnimationTimer(gpointer user_data);
    static void OnDestroy(GtkWidget* widget, gpointer user_data);
    
    void SetupUI();
    void LoadLogo();
    void DrawModernBackground(cairo_t* cr, int width, int height);
    void DrawLogo(cairo_t* cr, int width, int height);
    void DrawProgressIndicator(cairo_t* cr, int width, int height);
    void UpdateAnimation();
    
public:
    LinuxSplashScreen();
    ~LinuxSplashScreen() override;
    
    // ISplashScreen implementation
    bool Create(NativeInstanceHandle instance, const std::string& title = "MikoIDE") override;
    void Show() override;
    void Hide() override;
    void UpdateStatus(const std::string& status) override;
    void SetTitle(const std::string& title) override;
    NativeWindowHandle GetNativeHandle() const override { return m_window; }
    bool IsVisible() const override { return m_visible; }
    
    // Static methods for preloading
    static bool PreloadSplashImage();
    static void CleanupPreloadedImage();
};

#endif // __linux__