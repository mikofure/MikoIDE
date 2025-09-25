#pragma once
#include <fstream>
#include <string>

class Logger {
public:
  static void Initialize();
  static void Shutdown();
  static void LogMessage(const std::string &message);

private:
  static std::string GetTimestampedLogFileName();
  static void EnsureLogDirectoryExists();
  static std::string currentLogFile;
};