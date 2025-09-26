#ifdef __linux__

#include "linux_splash.hpp"
#include "../../utils/logger.hpp"
#include <cmath>
#include <thread>
#include <chrono>

LinuxSplashScreen::LinuxSplashScreen()
    : m_window(nullptr)
    , m_drawingArea(nullptr)
    , m_surface(nullptr)
    , m_cr(nullptr)
    , m_visible(false)
    , m_animationProgress(0.0f)
    , m_animationTimer(0)
{
}

LinuxSplashScreen::~LinuxSplashScreen() {
    if (m_animationTimer) {
        g_source_remove(m_animationTimer);
        m_animationTimer = 0;
    }
    
    if (m_cr) {
        cairo_destroy(m_cr);
        m_cr = nullptr;
    }
    
    if (m_surface) {
        cairo_surface_destroy(m_surface);
        m_surface = nullptr;
    }
    
    if (m_window) {
        gtk_widget_destroy(m_window);
        m_window = nullptr;
    }
}

bool LinuxSplashScreen::Create(NativeInstanceHandle instance, const std::string& title) {
    // Create main window
    m_window = gtk_window_new(GTK_WINDOW_POPUP);
    if (!m_window) {
        Logger::LogMessage("Failed to create GTK window for splash screen");
        return false;
    }
    
    m_titleText = title;
    
    // Set window properties
    gtk_window_set_title(GTK_WINDOW(m_window), title.c_str());
    gtk_window_set_default_size(GTK_WINDOW(m_window), 480, 320);
    gtk_window_set_position(GTK_WINDOW(m_window), GTK_WIN_POS_CENTER);
    gtk_window_set_decorated(GTK_WINDOW(m_window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW(m_window), TRUE);
    
    // Set window to be always on top and skip taskbar
    gtk_window_set_type_hint(GTK_WINDOW(m_window), GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
    
    // Create drawing area
    m_drawingArea = gtk_drawing_area_new();
    gtk_widget_set_size_request(m_drawingArea, 480, 320);
    gtk_container_add(GTK_CONTAINER(m_window), m_drawingArea);
    
    // Connect signals
    g_signal_connect(m_drawingArea, "draw", G_CALLBACK(OnDrawStatic), this);
    g_signal_connect(m_window, "destroy", G_CALLBACK(OnDestroyStatic), this);
    
    // Set up visual effects
    SetupVisualEffects();
    
    // Create Cairo surface for off-screen rendering
    CreateCairoSurface();
    
    // Show all widgets
    gtk_widget_show_all(m_window);
    
    // Start animation timer
    StartAnimation();
    
    return true;
}

void LinuxSplashScreen::Show() {
    if (m_window) {
        gtk_widget_show_all(m_window);
        gtk_window_present(GTK_WINDOW(m_window));
        m_visible = true;
    }
}

void LinuxSplashScreen::Hide() {
    if (m_window) {
        gtk_widget_hide(m_window);
        m_visible = false;
        
        if (m_animationTimer) {
            g_source_remove(m_animationTimer);
            m_animationTimer = 0;
        }
    }
}

void LinuxSplashScreen::UpdateStatus(const std::string& status) {
    m_statusText = status;
    if (m_window && m_visible) {
        gtk_widget_queue_draw(m_drawingArea);
    }
}

void LinuxSplashScreen::SetTitle(const std::string& title) {
    m_titleText = title;
    if (m_window) {
        gtk_window_set_title(GTK_WINDOW(m_window), title.c_str());
        if (m_visible) {
            gtk_widget_queue_draw(m_drawingArea);
        }
    }
}

NativeWindowHandle LinuxSplashScreen::GetWindowHandle() const {
    return m_window;
}

bool LinuxSplashScreen::IsVisible() const {
    return m_visible && m_window && gtk_widget_get_visible(m_window);
}

void LinuxSplashScreen::SetupVisualEffects() {
    if (!m_window) return;
    
    // Enable compositing for transparency
    GdkScreen* screen = gtk_widget_get_screen(m_window);
    GdkVisual* visual = gdk_screen_get_rgba_visual(screen);
    
    if (visual) {
        gtk_widget_set_visual(m_window, visual);
        gtk_widget_set_app_paintable(m_window, TRUE);
    }
    
    // Set window opacity
    gtk_widget_set_opacity(m_window, 0.95);
}

void LinuxSplashScreen::CreateCairoSurface() {
    if (m_surface) {
        cairo_surface_destroy(m_surface);
        m_surface = nullptr;
    }
    
    if (m_cr) {
        cairo_destroy(m_cr);
        m_cr = nullptr;
    }
    
    // Create image surface for off-screen rendering
    m_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 480, 320);
    m_cr = cairo_create(m_surface);
}

void LinuxSplashScreen::StartAnimation() {
    if (m_animationTimer) {
        g_source_remove(m_animationTimer);
    }
    
    // Start animation timer (~60 FPS)
    m_animationTimer = g_timeout_add(16, OnAnimationTimeoutStatic, this);
}

gboolean LinuxSplashScreen::OnAnimationTimeoutStatic(gpointer user_data) {
    LinuxSplashScreen* splash = static_cast<LinuxSplashScreen*>(user_data);
    return splash->OnAnimationTimeout();
}

gboolean LinuxSplashScreen::OnAnimationTimeout() {
    m_animationProgress += 0.1f;
    if (m_animationProgress > 2.0f * M_PI) {
        m_animationProgress -= 2.0f * M_PI;
    }
    
    if (m_visible && m_drawingArea) {
        gtk_widget_queue_draw(m_drawingArea);
    }
    
    return G_SOURCE_CONTINUE;
}

gboolean LinuxSplashScreen::OnDrawStatic(GtkWidget* widget, cairo_t* cr, gpointer user_data) {
    LinuxSplashScreen* splash = static_cast<LinuxSplashScreen*>(user_data);
    return splash->OnDraw(widget, cr);
}

gboolean LinuxSplashScreen::OnDraw(GtkWidget* widget, cairo_t* cr) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Clear background with transparency
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    
    DrawModernBackground(cr, allocation.width, allocation.height);
    DrawLogo(cr, allocation.width, allocation.height);
    DrawText(cr, allocation.width, allocation.height);
    DrawProgressIndicator(cr, allocation.width, allocation.height);
    
    return TRUE;
}

void LinuxSplashScreen::OnDestroyStatic(GtkWidget* widget, gpointer user_data) {
    LinuxSplashScreen* splash = static_cast<LinuxSplashScreen*>(user_data);
    splash->OnDestroy();
}

void LinuxSplashScreen::OnDestroy() {
    if (m_animationTimer) {
        g_source_remove(m_animationTimer);
        m_animationTimer = 0;
    }
    
    m_window = nullptr;
    m_drawingArea = nullptr;
    m_visible = false;
}

void LinuxSplashScreen::DrawModernBackground(cairo_t* cr, int width, int height) {
    // Draw rounded rectangle background with gradient
    double radius = 16.0;
    double x = 8.0;
    double y = 8.0;
    double w = width - 16.0;
    double h = height - 16.0;
    
    // Create rounded rectangle path
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - radius, y + radius, radius, -M_PI/2, 0);
    cairo_arc(cr, x + w - radius, y + h - radius, radius, 0, M_PI/2);
    cairo_arc(cr, x + radius, y + h - radius, radius, M_PI/2, M_PI);
    cairo_arc(cr, x + radius, y + radius, radius, M_PI, 3*M_PI/2);
    cairo_close_path(cr);
    
    // Create gradient background
    cairo_pattern_t* gradient = cairo_pattern_create_linear(0, 0, 0, height);
    cairo_pattern_add_color_stop_rgba(gradient, 0.0, 0.15, 0.15, 0.15, 0.98);
    cairo_pattern_add_color_stop_rgba(gradient, 1.0, 0.08, 0.08, 0.08, 0.98);
    
    cairo_set_source(cr, gradient);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(gradient);
    
    // Draw border
    cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 0.8);
    cairo_set_line_width(cr, 1.0);
    cairo_stroke(cr);
}

void LinuxSplashScreen::DrawLogo(cairo_t* cr, int width, int height) {
    // Draw a simple logo placeholder (circle with gradient)
    double centerX = width / 2.0;
    double centerY = 80.0;
    double radius = 32.0;
    
    cairo_arc(cr, centerX, centerY, radius, 0, 2 * M_PI);
    
    // Create radial gradient for logo
    cairo_pattern_t* gradient = cairo_pattern_create_radial(
        centerX, centerY, 0,
        centerX, centerY, radius
    );
    cairo_pattern_add_color_stop_rgba(gradient, 0.0, 0.0, 0.48, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgba(gradient, 1.0, 0.0, 0.24, 0.6, 1.0);
    
    cairo_set_source(cr, gradient);
    cairo_fill(cr);
    cairo_pattern_destroy(gradient);
    
    // Add highlight
    cairo_arc(cr, centerX - 8, centerY - 8, 8, 0, 2 * M_PI);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.3);
    cairo_fill(cr);
}

void LinuxSplashScreen::DrawText(cairo_t* cr, int width, int height) {
    cairo_select_font_face(cr, "Ubuntu", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    
    // Draw title
    if (!m_titleText.empty()) {
        cairo_set_font_size(cr, 24.0);
        cairo_text_extents_t extents;
        cairo_text_extents(cr, m_titleText.c_str(), &extents);
        
        double x = (width - extents.width) / 2.0;
        double y = 160.0;
        
        cairo_move_to(cr, x, y);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
        cairo_show_text(cr, m_titleText.c_str());
    }
    
    // Draw status
    if (!m_statusText.empty()) {
        cairo_set_font_size(cr, 14.0);
        cairo_text_extents_t extents;
        cairo_text_extents(cr, m_statusText.c_str(), &extents);
        
        double x = (width - extents.width) / 2.0;
        double y = 200.0;
        
        cairo_move_to(cr, x, y);
        cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1.0);
        cairo_show_text(cr, m_statusText.c_str());
    }
}

void LinuxSplashScreen::DrawProgressIndicator(cairo_t* cr, int width, int height) {
    double centerX = width / 2.0;
    double centerY = 260.0;
    double radius = 4.0;
    double spacing = 16.0;
    
    // Draw animated progress dots
    for (int i = 0; i < 3; i++) {
        double x = centerX + (i - 1) * spacing;
        double alpha = (sin(m_animationProgress + i * 0.5) + 1.0) / 2.0;
        
        cairo_arc(cr, x, centerY, radius, 0, 2 * M_PI);
        cairo_set_source_rgba(cr, 0.0, 0.48, 1.0, alpha);
        cairo_fill(cr);
    }
}

#endif // __linux__