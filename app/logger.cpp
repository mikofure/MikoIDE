#include "logger.hpp"
#include <fstream>
#include <iostream>

void Logger::LogMessage(const std::string& message) {
    // Write to a log file instead of console
    std::ofstream logFile("swipeide.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    }
}