#include "logger.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

// Static member definition
std::string Logger::currentLogFile = "";

void Logger::Initialize() {
    // Initialize logging system
    EnsureLogDirectoryExists();
    currentLogFile = GetTimestampedLogFileName();
}

void Logger::Shutdown() {
    // Clean up logging system
    // Could flush buffers, close files, etc.
    currentLogFile.clear();
}

void Logger::LogMessage(const std::string& message) {
    // Write to timestamped log file in logs directory
    if (currentLogFile.empty()) {
        Initialize(); // Auto-initialize if not done
    }
    
    std::ofstream logFile(currentLogFile, std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    }
}

std::string Logger::GetTimestampedLogFileName() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << "logs/hyperion_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") << ".log";
    return ss.str();
}

void Logger::EnsureLogDirectoryExists() {
    std::filesystem::create_directories("logs");
}