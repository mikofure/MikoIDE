#pragma once

#include <string>
#include <iostream>

// Simple logger utility
class Logger {
public:
    enum class Level {
        Debug,
        Info,
        Warning,
        Error
    };

    static void Log(Level level, const std::string& message);
    static void Debug(const std::string& message);
    static void Info(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);
    
    // Legacy methods for backward compatibility
    static void LogMessage(const std::string& message);
    static void Initialize();
    static void Shutdown();
};