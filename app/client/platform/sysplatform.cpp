#include "sysplatform.hpp"

#ifdef _WIN32
    #include "windows/windowsplatform.hpp"
#elif __linux__
    #include "linux/linuxplatform.hpp"
#endif

#include <thread>

// Platform factory implementations
std::unique_ptr<IPlatformWindow> PlatformFactory::CreatePlatformWindow() {
#ifdef _WIN32
    return std::make_unique<WindowsPlatformWindow>();
#elif __linux__
    return std::make_unique<LinuxPlatformWindow>();
#else
    return nullptr;
#endif
}

std::unique_ptr<IPlatformFileSystem> PlatformFactory::CreatePlatformFileSystem() {
#ifdef _WIN32
    return std::make_unique<WindowsPlatformFileSystem>();
#elif __linux__
    return std::make_unique<LinuxPlatformFileSystem>();
#else
    return nullptr;
#endif
}

// Platform utilities implementations
namespace PlatformUtils {
    std::string GetPlatformName() {
#ifdef _WIN32
        return "Windows";
#elif __linux__
        return "Linux";
#elif __APPLE__
        return "macOS";
#else
        return "Unknown";
#endif
    }
    
    bool IsWindows() {
#ifdef _WIN32
        return true;
#else
        return false;
#endif
    }
    
    bool IsLinux() {
#ifdef __linux__
        return true;
#else
        return false;
#endif
    }
    
    bool IsMacOS() {
#ifdef __APPLE__
        return true;
#else
        return false;
#endif
    }
    
    std::string GetSystemVersion() {
#ifdef _WIN32
        return "Windows (version detection not implemented)";
#elif __linux__
        return "Linux (version detection not implemented)";
#else
        return "Unknown";
#endif
    }
    
    int GetCPUCoreCount() {
        return std::thread::hardware_concurrency();
    }
    
    size_t GetTotalMemory() {
        // Platform-specific implementation needed
        return 0;
    }
}