#pragma once

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <windows.h>

class ProcessManager {
public:
  using OutputCallback = std::function<void(const std::string &)>;

  ProcessManager();
  ~ProcessManager();

  bool Initialize(const std::string &command);
  void Shutdown();

  void SendInput(const std::string &input);
  void Update();

  void SetOutputCallback(OutputCallback callback);

  bool IsRunning() const;

private:
  void ReadOutputThread();
  void ProcessOutput(const std::string &output);

  HANDLE m_hChildStdInRead;
  HANDLE m_hChildStdInWrite;
  HANDLE m_hChildStdOutRead;
  HANDLE m_hChildStdOutWrite;

  PROCESS_INFORMATION m_processInfo;
  STARTUPINFOA m_startupInfo;

  std::thread m_readThread;
  std::atomic<bool> m_running;

  OutputCallback m_outputCallback;

  std::string m_outputBuffer;
};