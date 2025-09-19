#pragma once
#include <string>

class Logger {
public:
    static void Initialize();
    static void Shutdown();
    static void LogMessage(const std::string& message);
};