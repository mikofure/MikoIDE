#include "logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

void Logger::Log(Level level, const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    std::string levelStr;
    switch (level) {
        case Level::Debug:   levelStr = "DEBUG"; break;
        case Level::Info:    levelStr = "INFO"; break;
        case Level::Warning: levelStr = "WARNING"; break;
        case Level::Error:   levelStr = "ERROR"; break;
    }
    
    std::cout << "[" << ss.str() << "] [" << levelStr << "] " << message << std::endl;
}

void Logger::Debug(const std::string& message) {
    Log(Level::Debug, message);
}

void Logger::Info(const std::string& message) {
    Log(Level::Info, message);
}

void Logger::Warning(const std::string& message) {
    Log(Level::Warning, message);
}

void Logger::Error(const std::string& message) {
    Log(Level::Error, message);
}

void Logger::LogMessage(const std::string& message) {
    Log(Level::Info, message);
}

void Logger::Initialize() {
    // No-op for backward compatibility
}

void Logger::Shutdown() {
    // No-op for backward compatibility
}