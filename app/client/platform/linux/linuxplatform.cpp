#include "linuxplatform.hpp"
#include <SDL3/SDL.h>
#if __has_include(<SDL3/SDL_syswm.h>)
#include <SDL3/SDL_syswm.h>
#define HYPERION_HAS_SDL_SYSWM 1
#endif
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>

// LinuxPlatformWindow implementation
LinuxPlatformWindow::LinuxPlatformWindow() 
    : x11_window_(0), x11_display_(nullptr), sdl_window_(nullptr), dpi_scale_(1.0f) {
}

LinuxPlatformWindow::~LinuxPlatformWindow() {
    Shutdown();
}

bool LinuxPlatformWindow::Initialize(SDL_Window* sdl_window) {
    if (!sdl_window) {
        return false;
    }

#ifndef HYPERION_HAS_SDL_SYSWM
    (void)sdl_window;
    std::cerr << "SDL syswm.h not available; cannot query native window info." << std::endl;
    return false;
#else
    sdl_window_ = sdl_window;

    SDL_SysWMinfo wm_info;
    SDL_VERSION(&wm_info.version);

    if (!SDL_GetWindowWMInfo(sdl_window_, &wm_info)) {
        std::cerr << "Failed to get X11 window info from SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    if (wm_info.subsystem != SDL_SYSWM_X11) {
        std::cerr << "Not running on X11" << std::endl;
        return false;
    }

    x11_display_ = wm_info.info.x11.display;
    x11_window_ = wm_info.info.x11.window;

    if (!x11_display_ || !x11_window_) {
        std::cerr << "Failed to get X11 display or window" << std::endl;
        return false;
    }

    InitializeX11Properties();
    UpdateDPIScale();

    return true;
#endif
}

void LinuxPlatformWindow::Shutdown() {
    x11_window_ = 0;
    x11_display_ = nullptr;
    sdl_window_ = nullptr;
}

PlatformWindowHandle LinuxPlatformWindow::GetNativeHandle() const {
    return reinterpret_cast<void*>(x11_window_);
}

void LinuxPlatformWindow::SetRoundedCorners(bool enable) {
    // X11 doesn't have native rounded corners support
    // This would require compositing manager support or custom rendering
    // For now, this is a no-op on Linux
    (void)enable;
}

void LinuxPlatformWindow::SetDarkMode(bool enable) {
    if (!x11_display_ || !x11_window_) return;
    
    // Set GTK theme hint for dark mode
    const char* theme = enable ? "dark" : "light";
    SetWindowProperty("_GTK_THEME_VARIANT", theme);
}

void LinuxPlatformWindow::SetBorderless(bool borderless) {
    if (!x11_display_ || !x11_window_) return;
    
    // Set window type to override redirect for borderless
    if (borderless) {
        XSetWindowAttributes attrs;
        attrs.override_redirect = True;
        XChangeWindowAttributes(x11_display_, x11_window_, CWOverrideRedirect, &attrs);
    } else {
        XSetWindowAttributes attrs;
        attrs.override_redirect = False;
        XChangeWindowAttributes(x11_display_, x11_window_, CWOverrideRedirect, &attrs);
    }
    
    XFlush(x11_display_);
}

void LinuxPlatformWindow::ExtendFrameIntoClientArea() {
    // This is typically handled by the window manager on Linux
    // We can set window properties to request client-side decorations
    if (!x11_display_ || !x11_window_) return;
    
    // Request client-side decorations
    SetWindowProperty("_MOTIF_WM_HINTS", "2, 0, 0, 0, 0");
}

float LinuxPlatformWindow::GetDPIScale() const {
    return dpi_scale_;
}

void LinuxPlatformWindow::UpdateDPIScale() {
    if (!x11_display_) {
        dpi_scale_ = 1.0f;
        return;
    }
    
    // Get DPI from X11
    int screen = DefaultScreen(x11_display_);
    double xres = ((double)DisplayWidth(x11_display_, screen) * 25.4) / 
                  ((double)DisplayWidthMM(x11_display_, screen));
    
    dpi_scale_ = static_cast<float>(xres / 96.0); // 96 DPI is 100% scaling
    
    // Clamp to reasonable values
    if (dpi_scale_ < 0.5f) dpi_scale_ = 0.5f;
    if (dpi_scale_ > 4.0f) dpi_scale_ = 4.0f;
}

void LinuxPlatformWindow::SetLayeredWindow(bool enable, int alpha) {
    if (!x11_display_ || !x11_window_) return;
    
    if (enable) {
        // Set window opacity
        Atom opacity_atom = XInternAtom(x11_display_, "_NET_WM_WINDOW_OPACITY", False);
        if (opacity_atom != None) {
            unsigned long opacity = (alpha * 0xFFFFFFFF) / 255;
            XChangeProperty(x11_display_, x11_window_, opacity_atom, XA_CARDINAL, 32,
                          PropModeReplace, (unsigned char*)&opacity, 1);
        }
    } else {
        // Remove opacity property
        Atom opacity_atom = XInternAtom(x11_display_, "_NET_WM_WINDOW_OPACITY", False);
        if (opacity_atom != None) {
            XDeleteProperty(x11_display_, x11_window_, opacity_atom);
        }
    }
    
    XFlush(x11_display_);
}

void LinuxPlatformWindow::SetTopMost(bool topmost) {
    if (!x11_display_ || !x11_window_) return;
    
    Atom state_atom = XInternAtom(x11_display_, "_NET_WM_STATE", False);
    Atom above_atom = XInternAtom(x11_display_, "_NET_WM_STATE_ABOVE", False);
    
    if (state_atom != None && above_atom != None) {
        XEvent event;
        event.xclient.type = ClientMessage;
        event.xclient.window = x11_window_;
        event.xclient.message_type = state_atom;
        event.xclient.format = 32;
        event.xclient.data.l[0] = topmost ? 1 : 0; // _NET_WM_STATE_ADD or _NET_WM_STATE_REMOVE
        event.xclient.data.l[1] = above_atom;
        event.xclient.data.l[2] = 0;
        event.xclient.data.l[3] = 1; // Source indication: application
        event.xclient.data.l[4] = 0;
        
        XSendEvent(x11_display_, DefaultRootWindow(x11_display_), False,
                   SubstructureRedirectMask | SubstructureNotifyMask, &event);
    }
    
    XFlush(x11_display_);
}

void LinuxPlatformWindow::InitializeX11Properties() {
    if (!x11_display_ || !x11_window_) return;
    
    // Set window class for proper identification
    XClassHint class_hint;
    class_hint.res_name = const_cast<char*>("hyperion");
    class_hint.res_class = const_cast<char*>("Hyperion");
    XSetClassHint(x11_display_, x11_window_, &class_hint);
    
    // Set window type to normal
    SetWindowType("_NET_WM_WINDOW_TYPE_NORMAL");
}

void LinuxPlatformWindow::SetWindowProperty(const char* property_name, const char* value) {
    if (!x11_display_ || !x11_window_) return;
    
    Atom property = XInternAtom(x11_display_, property_name, False);
    if (property != None) {
        XChangeProperty(x11_display_, x11_window_, property, XA_STRING, 8,
                       PropModeReplace, (unsigned char*)value, strlen(value));
    }
}

void LinuxPlatformWindow::SetWindowType(const char* type) {
    if (!x11_display_ || !x11_window_) return;
    
    Atom type_atom = XInternAtom(x11_display_, "_NET_WM_WINDOW_TYPE", False);
    Atom window_type = XInternAtom(x11_display_, type, False);
    
    if (type_atom != None && window_type != None) {
        XChangeProperty(x11_display_, x11_window_, type_atom, XA_ATOM, 32,
                       PropModeReplace, (unsigned char*)&window_type, 1);
    }
}

// LinuxPlatformFileSystem implementation
std::string LinuxPlatformFileSystem::GetDownloadsPath() const {
    return GetXDGDirectory("XDG_DOWNLOAD_DIR", "Downloads");
}

std::string LinuxPlatformFileSystem::GetAppDataPath() const {
    return GetXDGDirectory("XDG_CONFIG_HOME", ".config");
}

std::string LinuxPlatformFileSystem::GetTempPath() const {
    const char* tmp_dir = getenv("TMPDIR");
    if (!tmp_dir) tmp_dir = getenv("TMP");
    if (!tmp_dir) tmp_dir = getenv("TEMP");
    if (!tmp_dir) tmp_dir = "/tmp";
    
    return std::string(tmp_dir);
}

std::string LinuxPlatformFileSystem::GetKnownFolderPath(KnownFolder folder) const {
    switch (folder) {
        case KnownFolder::Downloads:
            return GetDownloadsPath();
        case KnownFolder::AppData:
            return GetAppDataPath();
        case KnownFolder::Temp:
            return GetTempPath();
        default:
            return "";
    }
}

bool LinuxPlatformFileSystem::FileExists(const std::string& path) const {
    return std::filesystem::exists(path);
}

bool LinuxPlatformFileSystem::CreateDirectory(const std::string& path) const {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::exception&) {
        return false;
    }
}

std::string LinuxPlatformFileSystem::GetHomeDirectory() const {
    const char* home = getenv("HOME");
    if (home) {
        return std::string(home);
    }
    
    // Fallback to passwd entry
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        return std::string(pw->pw_dir);
    }
    
    return "";
}

std::string LinuxPlatformFileSystem::GetXDGDirectory(const char* xdg_var, const char* fallback) const {
    const char* xdg_path = getenv(xdg_var);
    if (xdg_path) {
        return std::string(xdg_path);
    }
    
    // Use fallback relative to home directory
    std::string home = GetHomeDirectory();
    if (!home.empty()) {
        return home + "/" + fallback;
    }
    
    return "";
}