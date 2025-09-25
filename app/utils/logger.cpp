#include "logger.hpp"
#include "../bootstrap/bootstrap.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

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

void Logger::LogMessage(const std::string &message) {
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

  // Get app directory or fallback to current working directory
  std::filesystem::path logDir;
  try {
    logDir = Bootstrap::GetAppDirectory() / "logs";
  } catch (...) {
    // Fallback to current working directory if Bootstrap fails
    logDir = std::filesystem::current_path() / "logs";
  }

  std::stringstream ss;
  ss << logDir.string() << "/hyperion_"
     << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") << ".log";
  return ss.str();
}

void Logger::EnsureLogDirectoryExists() {
  // Get app directory or fallback to current working directory
  std::filesystem::path logDir;
  try {
    logDir = Bootstrap::GetAppDirectory() / "logs";
  } catch (...) {
    // Fallback to current working directory if Bootstrap fails
    logDir = std::filesystem::current_path() / "logs";
  }

  std::error_code ec;
  std::filesystem::create_directories(logDir, ec);
  if (ec) {
    // If we can't create in app directory, try current directory as fallback
    logDir = std::filesystem::current_path() / "logs";
    std::filesystem::create_directories(logDir, ec);
  }
}