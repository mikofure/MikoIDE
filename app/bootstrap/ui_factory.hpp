#pragma once

#include "ui_interface.hpp"
#include <memory>

class UIFactory {
public:
    static std::unique_ptr<ISplashScreen> CreateSplashScreen();
    static std::unique_ptr<IModernDialog> CreateModernDialog();
    
    static bool InitializePlatform();
    static void ShutdownPlatform();
};