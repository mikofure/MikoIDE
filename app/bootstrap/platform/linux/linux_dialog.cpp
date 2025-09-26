#ifdef __linux__

#include "linux_dialog.hpp"
#include "../../utils/logger.hpp"
#include <cmath>

LinuxModernDialog::LinuxModernDialog()
    : m_window(nullptr)
    , m_drawingArea(nullptr)
    , m_surface(nullptr)
    , m_cr(nullptr)
    , m_visible(false)
    , m_animationProgress(0.0f)
    , m_animationTimer(0)
    , m_hoveredButton(-1)
    , m_pressedButton(-1)
    , m_result(DialogResult::None)
{
}

LinuxModernDialog::~LinuxModernDialog() {
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

bool LinuxModernDialog::Create(PlatformInstance instance, PlatformWindow parent, const std::wstring& title) {
    // Convert wide string title to UTF-8
    std::string titleStr;
    if (!title.empty()) {
        // Simple conversion for ASCII characters (for more complex conversion, use proper UTF-8 conversion)
        titleStr.reserve(title.length());
        for (wchar_t wc : title) {
            if (wc < 128) {
                titleStr.push_back(static_cast<char>(wc));
            } else {
                titleStr.push_back('?'); // Fallback for non-ASCII
            }
        }
    }
    
    m_titleText = titleStr;
    
    // Create main window
    m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (!m_window) {
        Logger::LogMessage("Failed to create GTK window for dialog");
        return false;
    }
    
    // Set parent window if provided
    if (parent) {
        GtkWidget* parentWidget = static_cast<GtkWidget*>(parent);
        if (GTK_IS_WINDOW(parentWidget)) {
            gtk_window_set_transient_for(GTK_WINDOW(m_window), GTK_WINDOW(parentWidget));
        }
    }
    
    // Set window properties
    gtk_window_set_title(GTK_WINDOW(m_window), titleStr.c_str());
    gtk_window_set_default_size(GTK_WINDOW(m_window), 480, 320);
    gtk_window_set_position(GTK_WINDOW(m_window), GTK_WIN_POS_CENTER);
    gtk_window_set_decorated(GTK_WINDOW(m_window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW(m_window), TRUE);
    gtk_window_set_modal(GTK_WINDOW(m_window), TRUE);
    
    // Set window type hint for dialog
    gtk_window_set_type_hint(GTK_WINDOW(m_window), GDK_WINDOW_TYPE_HINT_DIALOG);
    
    // Create drawing area
    m_drawingArea = gtk_drawing_area_new();
    gtk_widget_set_size_request(m_drawingArea, 480, 320);
    gtk_container_add(GTK_CONTAINER(m_window), m_drawingArea);
    
    // Enable events for mouse interaction
    gtk_widget_set_events(m_drawingArea, 
        GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | 
        GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
    
    // Connect signals
    g_signal_connect(m_drawingArea, "draw", G_CALLBACK(OnDrawStatic), this);
    g_signal_connect(m_drawingArea, "button-press-event", G_CALLBACK(OnButtonPressStatic), this);
    g_signal_connect(m_drawingArea, "button-release-event", G_CALLBACK(OnButtonReleaseStatic), this);
    g_signal_connect(m_drawingArea, "motion-notify-event", G_CALLBACK(OnMotionNotifyStatic), this);
    g_signal_connect(m_window, "key-press-event", G_CALLBACK(OnKeyPressStatic), this);
    g_signal_connect(m_window, "destroy", G_CALLBACK(OnDestroyStatic), this);
    g_signal_connect(m_window, "delete-event", G_CALLBACK(OnDeleteEventStatic), this);
    
    // Set up visual effects
    SetupVisualEffects();
    
    // Create Cairo surface for off-screen rendering
    CreateCairoSurface();
    
    return true;
}

void LinuxModernDialog::Show() {
    if (m_window) {
        gtk_widget_show_all(m_window);
        gtk_window_present(GTK_WINDOW(m_window));
        gtk_widget_grab_focus(m_window);
        m_visible = true;
        
        StartAnimation();
    }
}

void LinuxModernDialog::Hide() {
    if (m_window) {
        gtk_widget_hide(m_window);
        m_visible = false;
        
        if (m_animationTimer) {
            g_source_remove(m_animationTimer);
            m_animationTimer = 0;
        }
    }
}

void LinuxModernDialog::SetTitle(const std::string& title) {
    m_titleText = title;
    if (m_window) {
        gtk_window_set_title(GTK_WINDOW(m_window), title.c_str());
        if (m_visible) {
            gtk_widget_queue_draw(m_drawingArea);
        }
    }
}

void LinuxModernDialog::SetMessage(const std::string& message) {
    m_messageText = message;
    if (m_window && m_visible) {
        gtk_widget_queue_draw(m_drawingArea);
    }
}

void LinuxModernDialog::AddButton(const std::string& text, DialogResult result) {
    DialogButton button;
    button.text = text;
    button.result = result;
    button.rect = {0, 0, 0, 0}; // Will be calculated in layout
    m_buttons.push_back(button);
    
    if (m_window && m_visible) {
        CalculateLayout();
        gtk_widget_queue_draw(m_drawingArea);
    }
}

void LinuxModernDialog::ClearButtons() {
    m_buttons.clear();
    m_hoveredButton = -1;
    m_pressedButton = -1;
    
    if (m_window && m_visible) {
        gtk_widget_queue_draw(m_drawingArea);
    }
}

DialogResult LinuxModernDialog::ShowModal() {
    if (!m_window) return DialogResult::None;
    
    Show();
    
    m_result = DialogResult::None;
    
    // Run modal loop
    while (m_result == DialogResult::None && gtk_widget_get_visible(m_window)) {
        gtk_main_iteration();
    }
    
    Hide();
    return m_result;
}

NativeWindowHandle LinuxModernDialog::GetWindowHandle() const {
    return m_window;
}

bool LinuxModernDialog::IsVisible() const {
    return m_visible && m_window && gtk_widget_get_visible(m_window);
}

void LinuxModernDialog::SetupVisualEffects() {
    if (!m_window) return;
    
    // Enable compositing for transparency
    GdkScreen* screen = gtk_widget_get_screen(m_window);
    GdkVisual* visual = gdk_screen_get_rgba_visual(screen);
    
    if (visual) {
        gtk_widget_set_visual(m_window, visual);
        gtk_widget_set_app_paintable(m_window, TRUE);
    }
    
    // Set window opacity
    gtk_widget_set_opacity(m_window, 0.98);
}

void LinuxModernDialog::CreateCairoSurface() {
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

void LinuxModernDialog::StartAnimation() {
    if (m_animationTimer) {
        g_source_remove(m_animationTimer);
    }
    
    // Start animation timer (~60 FPS)
    m_animationTimer = g_timeout_add(16, OnAnimationTimeoutStatic, this);
}

void LinuxModernDialog::CalculateLayout() {
    if (m_buttons.empty()) return;
    
    // Calculate button layout
    double buttonWidth = 100.0;
    double buttonHeight = 32.0;
    double buttonSpacing = 12.0;
    double totalWidth = m_buttons.size() * buttonWidth + (m_buttons.size() - 1) * buttonSpacing;
    double startX = (480.0 - totalWidth) / 2.0;
    double buttonY = 320.0 - 60.0;
    
    for (size_t i = 0; i < m_buttons.size(); i++) {
        double x = startX + i * (buttonWidth + buttonSpacing);
        m_buttons[i].rect = {x, buttonY, x + buttonWidth, buttonY + buttonHeight};
    }
}

gboolean LinuxModernDialog::OnAnimationTimeoutStatic(gpointer user_data) {
    LinuxModernDialog* dialog = static_cast<LinuxModernDialog*>(user_data);
    return dialog->OnAnimationTimeout();
}

gboolean LinuxModernDialog::OnAnimationTimeout() {
    m_animationProgress += 0.1f;
    if (m_animationProgress > 2.0f * M_PI) {
        m_animationProgress -= 2.0f * M_PI;
    }
    
    if (m_visible && m_drawingArea) {
        gtk_widget_queue_draw(m_drawingArea);
    }
    
    return G_SOURCE_CONTINUE;
}

gboolean LinuxModernDialog::OnDrawStatic(GtkWidget* widget, cairo_t* cr, gpointer user_data) {
    LinuxModernDialog* dialog = static_cast<LinuxModernDialog*>(user_data);
    return dialog->OnDraw(widget, cr);
}

gboolean LinuxModernDialog::OnDraw(GtkWidget* widget, cairo_t* cr) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Clear background with transparency
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    
    DrawBackground(cr, allocation.width, allocation.height);
    DrawTitle(cr, allocation.width, allocation.height);
    DrawMessage(cr, allocation.width, allocation.height);
    DrawButtons(cr, allocation.width, allocation.height);
    
    return TRUE;
}

gboolean LinuxModernDialog::OnButtonPressStatic(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    LinuxModernDialog* dialog = static_cast<LinuxModernDialog*>(user_data);
    return dialog->OnButtonPress(widget, event);
}

gboolean LinuxModernDialog::OnButtonPress(GtkWidget* widget, GdkEventButton* event) {
    if (event->button == 1) { // Left mouse button
        m_pressedButton = -1;
        
        for (size_t i = 0; i < m_buttons.size(); i++) {
            const ButtonRect& rect = m_buttons[i].rect;
            if (event->x >= rect.x && event->x <= rect.x + rect.width &&
                event->y >= rect.y && event->y <= rect.y + rect.height) {
                m_pressedButton = static_cast<int>(i);
                gtk_widget_queue_draw(m_drawingArea);
                break;
            }
        }
    }
    
    return TRUE;
}

gboolean LinuxModernDialog::OnButtonReleaseStatic(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    LinuxModernDialog* dialog = static_cast<LinuxModernDialog*>(user_data);
    return dialog->OnButtonRelease(widget, event);
}

gboolean LinuxModernDialog::OnButtonRelease(GtkWidget* widget, GdkEventButton* event) {
    if (event->button == 1 && m_pressedButton >= 0) { // Left mouse button
        // Check if mouse is still over the pressed button
        const ButtonRect& rect = m_buttons[m_pressedButton].rect;
        if (event->x >= rect.x && event->x <= rect.x + rect.width &&
            event->y >= rect.y && event->y <= rect.y + rect.height) {
            m_result = m_buttons[m_pressedButton].result;
        }
        
        m_pressedButton = -1;
        gtk_widget_queue_draw(m_drawingArea);
    }
    
    return TRUE;
}

gboolean LinuxModernDialog::OnMotionNotifyStatic(GtkWidget* widget, GdkEventMotion* event, gpointer user_data) {
    LinuxModernDialog* dialog = static_cast<LinuxModernDialog*>(user_data);
    return dialog->OnMotionNotify(widget, event);
}

gboolean LinuxModernDialog::OnMotionNotify(GtkWidget* widget, GdkEventMotion* event) {
    int oldHovered = m_hoveredButton;
    m_hoveredButton = -1;
    
    for (size_t i = 0; i < m_buttons.size(); i++) {
        const ButtonRect& rect = m_buttons[i].rect;
        if (event->x >= rect.x && event->x <= rect.x + rect.width &&
            event->y >= rect.y && event->y <= rect.y + rect.height) {
            m_hoveredButton = static_cast<int>(i);
            break;
        }
    }
    
    if (oldHovered != m_hoveredButton) {
        gtk_widget_queue_draw(m_drawingArea);
        
        // Update cursor
        GdkWindow* window = gtk_widget_get_window(widget);
        if (window) {
            GdkCursor* cursor = gdk_cursor_new_for_display(
                gdk_window_get_display(window),
                m_hoveredButton >= 0 ? GDK_HAND2 : GDK_ARROW
            );
            gdk_window_set_cursor(window, cursor);
            g_object_unref(cursor);
        }
    }
    
    return TRUE;
}

gboolean LinuxModernDialog::OnKeyPressStatic(GtkWidget* widget, GdkEventKey* event, gpointer user_data) {
    LinuxModernDialog* dialog = static_cast<LinuxModernDialog*>(user_data);
    return dialog->OnKeyPress(widget, event);
}

gboolean LinuxModernDialog::OnKeyPress(GtkWidget* widget, GdkEventKey* event) {
    switch (event->keyval) {
    case GDK_KEY_Escape:
        m_result = DialogResult::Cancel;
        return TRUE;
        
    case GDK_KEY_Return:
    case GDK_KEY_KP_Enter:
        m_result = DialogResult::OK;
        return TRUE;
        
    default:
        break;
    }
    
    return FALSE;
}

void LinuxModernDialog::OnDestroyStatic(GtkWidget* widget, gpointer user_data) {
    LinuxModernDialog* dialog = static_cast<LinuxModernDialog*>(user_data);
    dialog->OnDestroy();
}

void LinuxModernDialog::OnDestroy() {
    if (m_animationTimer) {
        g_source_remove(m_animationTimer);
        m_animationTimer = 0;
    }
    
    if (m_result == DialogResult::None) {
        m_result = DialogResult::Cancel;
    }
    
    m_window = nullptr;
    m_drawingArea = nullptr;
    m_visible = false;
}

gboolean LinuxModernDialog::OnDeleteEventStatic(GtkWidget* widget, GdkEvent* event, gpointer user_data) {
    LinuxModernDialog* dialog = static_cast<LinuxModernDialog*>(user_data);
    return dialog->OnDeleteEvent(widget, event);
}

gboolean LinuxModernDialog::OnDeleteEvent(GtkWidget* widget, GdkEvent* event) {
    m_result = DialogResult::Cancel;
    return FALSE; // Allow window to be destroyed
}

void LinuxModernDialog::DrawBackground(cairo_t* cr, int width, int height) {
    // Draw rounded rectangle background with gradient
    double radius = 16.0;
    double x = 8.0;
    double y = 8.0;
    double w = width - 16.0;
    double h = height - 16.0;
    
    // Create shadow effect
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - radius + 4, y + radius + 4, radius, -M_PI/2, 0);
    cairo_arc(cr, x + w - radius + 4, y + h - radius + 4, radius, 0, M_PI/2);
    cairo_arc(cr, x + radius + 4, y + h - radius + 4, radius, M_PI/2, M_PI);
    cairo_arc(cr, x + radius + 4, y + radius + 4, radius, M_PI, 3*M_PI/2);
    cairo_close_path(cr);
    
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.3);
    cairo_fill(cr);
    
    // Create rounded rectangle path for main background
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - radius, y + radius, radius, -M_PI/2, 0);
    cairo_arc(cr, x + w - radius, y + h - radius, radius, 0, M_PI/2);
    cairo_arc(cr, x + radius, y + h - radius, radius, M_PI/2, M_PI);
    cairo_arc(cr, x + radius, y + radius, radius, M_PI, 3*M_PI/2);
    cairo_close_path(cr);
    
    // Create gradient background
    cairo_pattern_t* gradient = cairo_pattern_create_linear(0, 0, 0, height);
    cairo_pattern_add_color_stop_rgba(gradient, 0.0, 0.18, 0.18, 0.18, 0.98);
    cairo_pattern_add_color_stop_rgba(gradient, 1.0, 0.10, 0.10, 0.10, 0.98);
    
    cairo_set_source(cr, gradient);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(gradient);
    
    // Draw border
    cairo_set_source_rgba(cr, 0.35, 0.35, 0.35, 0.8);
    cairo_set_line_width(cr, 1.0);
    cairo_stroke(cr);
}

void LinuxModernDialog::DrawTitle(cairo_t* cr, int width, int height) {
    if (m_titleText.empty()) return;
    
    cairo_select_font_face(cr, "Ubuntu", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20.0);
    
    cairo_text_extents_t extents;
    cairo_text_extents(cr, m_titleText.c_str(), &extents);
    
    double x = (width - extents.width) / 2.0;
    double y = 50.0;
    
    cairo_move_to(cr, x, y);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_show_text(cr, m_titleText.c_str());
}

void LinuxModernDialog::DrawMessage(cairo_t* cr, int width, int height) {
    if (m_messageText.empty()) return;
    
    cairo_select_font_face(cr, "Ubuntu", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 14.0);
    
    // Simple text wrapping for long messages
    std::vector<std::string> lines;
    std::string currentLine;
    std::istringstream words(m_messageText);
    std::string word;
    
    while (words >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        cairo_text_extents_t extents;
        cairo_text_extents(cr, testLine.c_str(), &extents);
        
        if (extents.width > width - 60) {
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine = word;
            } else {
                lines.push_back(word);
            }
        } else {
            currentLine = testLine;
        }
    }
    
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    
    // Draw lines
    double startY = 100.0;
    double lineHeight = 20.0;
    
    for (size_t i = 0; i < lines.size(); i++) {
        cairo_text_extents_t extents;
        cairo_text_extents(cr, lines[i].c_str(), &extents);
        
        double x = (width - extents.width) / 2.0;
        double y = startY + i * lineHeight;
        
        cairo_move_to(cr, x, y);
        cairo_set_source_rgba(cr, 0.9, 0.9, 0.9, 1.0);
        cairo_show_text(cr, lines[i].c_str());
    }
}

void LinuxModernDialog::DrawButtons(cairo_t* cr, int width, int height) {
    CalculateLayout();
    
    cairo_select_font_face(cr, "Ubuntu", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 13.0);
    
    for (size_t i = 0; i < m_buttons.size(); i++) {
        const DialogButton& button = m_buttons[i];
        const ButtonRect& rect = button.rect;
        
        // Determine button colors based on state
        double bgR = 0.2, bgG = 0.2, bgB = 0.2, bgA = 1.0;
        double textR = 1.0, textG = 1.0, textB = 1.0, textA = 1.0;
        
        if (static_cast<int>(i) == m_pressedButton) {
            bgR = 0.0; bgG = 0.48; bgB = 1.0;
        } else if (static_cast<int>(i) == m_hoveredButton) {
            bgR = 0.3; bgG = 0.3; bgB = 0.3;
        }
        
        // Draw button background
        double radius = 6.0;
        cairo_new_sub_path(cr);
        cairo_arc(cr, rect.x + rect.width - radius, rect.y + radius, radius, -M_PI/2, 0);
        cairo_arc(cr, rect.x + rect.width - radius, rect.y + rect.height - radius, radius, 0, M_PI/2);
        cairo_arc(cr, rect.x + radius, rect.y + rect.height - radius, radius, M_PI/2, M_PI);
        cairo_arc(cr, rect.x + radius, rect.y + radius, radius, M_PI, 3*M_PI/2);
        cairo_close_path(cr);
        
        cairo_set_source_rgba(cr, bgR, bgG, bgB, bgA);
        cairo_fill_preserve(cr);
        
        // Draw button border
        cairo_set_source_rgba(cr, 0.4, 0.4, 0.4, 1.0);
        cairo_set_line_width(cr, 1.0);
        cairo_stroke(cr);
        
        // Draw button text
        cairo_text_extents_t extents;
        cairo_text_extents(cr, button.text.c_str(), &extents);
        
        double textX = rect.x + (rect.width - extents.width) / 2.0;
        double textY = rect.y + (rect.height + extents.height) / 2.0;
        
        cairo_move_to(cr, textX, textY);
        cairo_set_source_rgba(cr, textR, textG, textB, textA);
        cairo_show_text(cr, button.text.c_str());
    }
}

void LinuxModernDialog::SetProgress(int percentage) {
    m_progress = std::clamp(percentage, 0, 100);
    if (m_drawingArea) {
        gtk_widget_queue_draw(m_drawingArea);
    }
}

void LinuxModernDialog::SetStatus(const std::string& status) {
    m_statusText = status;
    if (m_drawingArea) {
        gtk_widget_queue_draw(m_drawingArea);
    }
}

void LinuxModernDialog::SetDownloadInfo(size_t bytesDownloaded, size_t totalBytes, size_t speed) {
    m_bytesDownloaded = bytesDownloaded;
    m_totalBytes = totalBytes;
    m_downloadSpeed = speed;
    if (m_drawingArea) {
        gtk_widget_queue_draw(m_drawingArea);
    }
}

void LinuxModernDialog::UpdateProgress(int percentage, const std::wstring& status, size_t bytesDownloaded, size_t totalBytes) {
    m_progress = std::clamp(percentage, 0, 100);
    
    // Convert wide string status to UTF-8
    std::string statusStr;
    if (!status.empty()) {
        statusStr.reserve(status.length());
        for (wchar_t wc : status) {
            if (wc < 128) {
                statusStr.push_back(static_cast<char>(wc));
            } else {
                statusStr.push_back('?'); // Fallback for non-ASCII
            }
        }
    }
    
    m_statusText = statusStr;
    m_bytesDownloaded = bytesDownloaded;
    m_totalBytes = totalBytes;
    
    if (m_drawingArea) {
        gtk_widget_queue_draw(m_drawingArea);
    }
}

#endif // __linux__