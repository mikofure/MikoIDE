#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Hyperion {
namespace Toolchain {

// Forward declarations
class WindowedUI;

// Sandbox environment configuration
struct SandboxConfig {
  std::string name;
  std::string workingDirectory;
  std::string toolchainPath;
  std::vector<std::string> environmentVariables;
  std::vector<std::string> allowedPaths;
  bool networkAccess = false;
  bool fileSystemAccess = true;
  uint64_t memoryLimit = 0; // 0 = unlimited
  uint32_t timeLimit = 0;   // 0 = unlimited (seconds)
};

// Toolchain information
struct ToolchainInfo {
  std::string id;
  std::string name;
  std::string version;
  std::string description;
  std::string executablePath;
  std::vector<std::string> supportedExtensions;
  SandboxConfig defaultSandbox;
};

// Process execution result
struct ExecutionResult {
  int exitCode = 0;
  std::string stdoutData;
  std::string stderrData;
  uint64_t executionTime = 0; // milliseconds
  uint64_t memoryUsed = 0;    // bytes
  bool timedOut = false;
  bool memoryExceeded = false;
};

// Sandbox process handle
class SandboxProcess {
public:
  virtual ~SandboxProcess() = default;
  virtual bool IsRunning() const = 0;
  virtual void Terminate() = 0;
  virtual ExecutionResult GetResult() = 0;
  virtual void SendInput(const std::string &input) = 0;
  virtual std::string ReadOutput() = 0;
  virtual std::string ReadError() = 0;
};

// Main toolchain manager class
class ToolchainManager {
public:
  ToolchainManager();
  ~ToolchainManager();

  // Initialization and cleanup
  bool Initialize();
  void Shutdown();

  // UI integration
  void SetUI(WindowedUI *ui);
  WindowedUI *GetUI() const { return m_ui; }

  // Toolchain management
  bool RegisterToolchain(const ToolchainInfo &toolchain);
  bool UnregisterToolchain(const std::string &toolchainId);
  std::vector<ToolchainInfo> GetAvailableToolchains() const;
  ToolchainInfo *GetToolchain(const std::string &toolchainId);

  // Sandbox management
  std::unique_ptr<SandboxProcess>
  CreateSandboxProcess(const std::string &toolchainId,
                       const std::vector<std::string> &arguments,
                       const SandboxConfig &config);

  // Project management
  bool CreateProject(const std::string &name, const std::string &toolchainId,
                     const std::string &path);
  bool OpenProject(const std::string &path);
  bool CloseProject();
  std::string GetCurrentProjectPath() const { return m_currentProjectPath; }

  // Build and execution
  std::unique_ptr<SandboxProcess>
  BuildProject(const SandboxConfig &config = {});
  std::unique_ptr<SandboxProcess> RunProject(const SandboxConfig &config = {});
  std::unique_ptr<SandboxProcess>
  ExecuteCommand(const std::string &command,
                 const std::vector<std::string> &arguments,
                 const SandboxConfig &config = {});

  // Configuration
  bool SaveConfiguration(const std::string &path = "");
  bool LoadConfiguration(const std::string &path = "");

  // Event callbacks
  using ToolchainEventCallback =
      std::function<void(const std::string &, const std::string &)>;
  void SetOnToolchainRegistered(ToolchainEventCallback callback) {
    m_onToolchainRegistered = callback;
  }
  void SetOnToolchainUnregistered(ToolchainEventCallback callback) {
    m_onToolchainUnregistered = callback;
  }
  void SetOnProjectOpened(ToolchainEventCallback callback) {
    m_onProjectOpened = callback;
  }
  void SetOnProjectClosed(ToolchainEventCallback callback) {
    m_onProjectClosed = callback;
  }

  // Update loop
  void Update();

private:
  // Internal methods
  void DetectSystemToolchains();
  bool ValidateSandboxConfig(const SandboxConfig &config);
  std::string GenerateProcessId();
  void CleanupFinishedProcesses();

  // Member variables
  WindowedUI *m_ui;
  std::unordered_map<std::string, ToolchainInfo> m_toolchains;
  std::vector<std::unique_ptr<SandboxProcess>> m_activeProcesses;
  std::string m_currentProjectPath;
  std::string m_configPath;

  // Event callbacks
  ToolchainEventCallback m_onToolchainRegistered;
  ToolchainEventCallback m_onToolchainUnregistered;
  ToolchainEventCallback m_onProjectOpened;
  ToolchainEventCallback m_onProjectClosed;

  // Internal state
  bool m_initialized;
  uint32_t m_nextProcessId;
};

} // namespace Toolchain
} // namespace Hyperion