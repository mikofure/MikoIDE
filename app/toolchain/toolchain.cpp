#include "toolchain.hpp"
#include "windowed.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#endif

namespace Hyperion {
namespace Toolchain {

// Windows-specific sandbox process implementation
class WindowsSandboxProcess : public SandboxProcess {
public:
    WindowsSandboxProcess(const std::string& command, const std::vector<std::string>& args, const SandboxConfig& config)
        : m_processHandle(nullptr), m_threadHandle(nullptr), m_processId(0), m_config(config), m_startTime(0) {
        
        // Build command line
        std::string cmdLine = command;
        for (const auto& arg : args) {
            cmdLine += " \"" + arg + "\"";
        }

        // Create pipes for stdout/stderr
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = nullptr;

        CreatePipe(&m_stdoutRead, &m_stdoutWrite, &saAttr, 0);
        CreatePipe(&m_stderrRead, &m_stderrWrite, &saAttr, 0);
        CreatePipe(&m_stdinRead, &m_stdinWrite, &saAttr, 0);

        SetHandleInformation(m_stdoutRead, HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(m_stderrRead, HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(m_stdinWrite, HANDLE_FLAG_INHERIT, 0);

        // Setup process startup info
        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        si.hStdError = m_stderrWrite;
        si.hStdOutput = m_stdoutWrite;
        si.hStdInput = m_stdinRead;
        si.dwFlags |= STARTF_USESTDHANDLES;

        PROCESS_INFORMATION pi = {};

        // Create the process
        if (CreateProcessA(nullptr, const_cast<char*>(cmdLine.c_str()), nullptr, nullptr, TRUE, 0, nullptr, 
                          config.workingDirectory.empty() ? nullptr : config.workingDirectory.c_str(), &si, &pi)) {
            m_processHandle = pi.hProcess;
            m_threadHandle = pi.hThread;
            m_processId = pi.dwProcessId;
            m_startTime = GetTickCount64();
        }

        // Close write ends of pipes in parent process
        CloseHandle(m_stdoutWrite);
        CloseHandle(m_stderrWrite);
        CloseHandle(m_stdinRead);
    }

    ~WindowsSandboxProcess() {
        if (m_processHandle) {
            CloseHandle(m_processHandle);
        }
        if (m_threadHandle) {
            CloseHandle(m_threadHandle);
        }
        if (m_stdoutRead) CloseHandle(m_stdoutRead);
        if (m_stderrRead) CloseHandle(m_stderrRead);
        if (m_stdinWrite) CloseHandle(m_stdinWrite);
    }

    bool IsRunning() const override {
        if (!m_processHandle) return false;
        DWORD exitCode;
        return GetExitCodeProcess(m_processHandle, &exitCode) && exitCode == STILL_ACTIVE;
    }

    void Terminate() override {
        if (m_processHandle && IsRunning()) {
            TerminateProcess(m_processHandle, 1);
        }
    }

    ExecutionResult GetResult() override {
        ExecutionResult result;
        
        if (!m_processHandle) {
            result.exitCode = -1;
            return result;
        }

        DWORD exitCode;
        if (GetExitCodeProcess(m_processHandle, &exitCode)) {
            result.exitCode = static_cast<int>(exitCode);
        }

        result.executionTime = GetTickCount64() - m_startTime;
        result.stdoutData = ReadOutput();
        result.stderrData = ReadError();

        // Check memory usage
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(m_processHandle, &pmc, sizeof(pmc))) {
            result.memoryUsed = pmc.WorkingSetSize;
            if (m_config.memoryLimit > 0 && result.memoryUsed > m_config.memoryLimit) {
                result.memoryExceeded = true;
            }
        }

        // Check timeout
        if (m_config.timeLimit > 0 && result.executionTime > m_config.timeLimit * 1000) {
            result.timedOut = true;
        }

        return result;
    }

    void SendInput(const std::string& input) override {
        if (m_stdinWrite) {
            DWORD written;
            WriteFile(m_stdinWrite, input.c_str(), static_cast<DWORD>(input.length()), &written, nullptr);
        }
    }

    std::string ReadOutput() override {
        return ReadFromPipe(m_stdoutRead);
    }

    std::string ReadError() override {
        return ReadFromPipe(m_stderrRead);
    }

private:
    std::string ReadFromPipe(HANDLE pipe) {
        std::string result;
        DWORD bytesAvailable = 0;
        
        if (PeekNamedPipe(pipe, nullptr, 0, nullptr, &bytesAvailable, nullptr) && bytesAvailable > 0) {
            std::vector<char> buffer(bytesAvailable);
            DWORD bytesRead;
            if (ReadFile(pipe, buffer.data(), bytesAvailable, &bytesRead, nullptr)) {
                result.assign(buffer.data(), bytesRead);
            }
        }
        
        return result;
    }

    HANDLE m_processHandle;
    HANDLE m_threadHandle;
    HANDLE m_stdoutRead, m_stdoutWrite;
    HANDLE m_stderrRead, m_stderrWrite;
    HANDLE m_stdinRead, m_stdinWrite;
    DWORD m_processId;
    SandboxConfig m_config;
    uint64_t m_startTime;
};

// ToolchainManager implementation
ToolchainManager::ToolchainManager()
    : m_ui(nullptr), m_initialized(false), m_nextProcessId(1) {
}

ToolchainManager::~ToolchainManager() {
    Shutdown();
}

bool ToolchainManager::Initialize() {
    if (m_initialized) {
        return true;
    }

    std::cout << "Initializing Toolchain Manager..." << std::endl;

    // Set default config path
    m_configPath = "toolchain_config.json";

    // Detect system toolchains
    DetectSystemToolchains();

    // Load configuration if it exists
    if (std::filesystem::exists(m_configPath)) {
        LoadConfiguration(m_configPath);
    }

    m_initialized = true;
    std::cout << "Toolchain Manager initialized successfully" << std::endl;
    return true;
}

void ToolchainManager::Shutdown() {
    if (!m_initialized) {
        return;
    }

    std::cout << "Shutting down Toolchain Manager..." << std::endl;

    // Terminate all active processes
    for (auto& process : m_activeProcesses) {
        if (process && process->IsRunning()) {
            process->Terminate();
        }
    }
    m_activeProcesses.clear();

    // Save configuration
    SaveConfiguration();

    m_initialized = false;
    std::cout << "Toolchain Manager shutdown complete" << std::endl;
}

void ToolchainManager::SetUI(WindowedUI* ui) {
    m_ui = ui;
}

bool ToolchainManager::RegisterToolchain(const ToolchainInfo& toolchain) {
    if (toolchain.id.empty() || toolchain.name.empty()) {
        return false;
    }

    m_toolchains[toolchain.id] = toolchain;
    
    if (m_onToolchainRegistered) {
        m_onToolchainRegistered(toolchain.id, toolchain.name);
    }

    std::cout << "Registered toolchain: " << toolchain.name << " (" << toolchain.id << ")" << std::endl;
    return true;
}

bool ToolchainManager::UnregisterToolchain(const std::string& toolchainId) {
    auto it = m_toolchains.find(toolchainId);
    if (it == m_toolchains.end()) {
        return false;
    }

    std::string name = it->second.name;
    m_toolchains.erase(it);

    if (m_onToolchainUnregistered) {
        m_onToolchainUnregistered(toolchainId, name);
    }

    std::cout << "Unregistered toolchain: " << name << " (" << toolchainId << ")" << std::endl;
    return true;
}

std::vector<ToolchainInfo> ToolchainManager::GetAvailableToolchains() const {
    std::vector<ToolchainInfo> toolchains;
    for (const auto& pair : m_toolchains) {
        toolchains.push_back(pair.second);
    }
    return toolchains;
}

ToolchainInfo* ToolchainManager::GetToolchain(const std::string& toolchainId) {
    auto it = m_toolchains.find(toolchainId);
    return (it != m_toolchains.end()) ? &it->second : nullptr;
}

std::unique_ptr<SandboxProcess> ToolchainManager::CreateSandboxProcess(
    const std::string& toolchainId,
    const std::vector<std::string>& arguments,
    const SandboxConfig& config) {
    
    auto* toolchain = GetToolchain(toolchainId);
    if (!toolchain) {
        std::cerr << "Toolchain not found: " << toolchainId << std::endl;
        return nullptr;
    }

    if (!ValidateSandboxConfig(config)) {
        std::cerr << "Invalid sandbox configuration" << std::endl;
        return nullptr;
    }

    auto process = std::make_unique<WindowsSandboxProcess>(toolchain->executablePath, arguments, config);
    return process;
}

bool ToolchainManager::CreateProject(const std::string& name, const std::string& toolchainId, const std::string& path) {
    if (name.empty() || path.empty()) {
        return false;
    }

    auto* toolchain = GetToolchain(toolchainId);
    if (!toolchain) {
        return false;
    }

    // Create project directory
    std::filesystem::create_directories(path);

    // Create basic project structure based on toolchain
    std::string projectFile = path + "/" + name + ".project";
    std::ofstream file(projectFile);
    if (file.is_open()) {
        file << "{\n";
        file << "  \"name\": \"" << name << "\",\n";
        file << "  \"toolchain\": \"" << toolchainId << "\",\n";
        file << "  \"version\": \"1.0.0\"\n";
        file << "}\n";
        file.close();
    }

    std::cout << "Created project: " << name << " at " << path << std::endl;
    return true;
}

bool ToolchainManager::OpenProject(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        return false;
    }

    CloseProject();
    m_currentProjectPath = path;

    if (m_onProjectOpened) {
        m_onProjectOpened(path, "");
    }

    std::cout << "Opened project: " << path << std::endl;
    return true;
}

bool ToolchainManager::CloseProject() {
    if (!m_currentProjectPath.empty()) {
        if (m_onProjectClosed) {
            m_onProjectClosed(m_currentProjectPath, "");
        }
        m_currentProjectPath.clear();
        std::cout << "Closed project" << std::endl;
    }
    return true;
}

std::unique_ptr<SandboxProcess> ToolchainManager::BuildProject(const SandboxConfig& config) {
    if (m_currentProjectPath.empty()) {
        return nullptr;
    }

    // This is a simplified build - in reality, you'd parse the project file
    // and determine the appropriate build command based on the toolchain
    std::vector<std::string> args = {"build", m_currentProjectPath};
    return ExecuteCommand("cmake", args, config);
}

std::unique_ptr<SandboxProcess> ToolchainManager::RunProject(const SandboxConfig& config) {
    if (m_currentProjectPath.empty()) {
        return nullptr;
    }

    // This is a simplified run - in reality, you'd determine the executable
    // based on the project configuration
    std::vector<std::string> args = {};
    return ExecuteCommand(m_currentProjectPath + "/output.exe", args, config);
}

std::unique_ptr<SandboxProcess> ToolchainManager::ExecuteCommand(
    const std::string& command,
    const std::vector<std::string>& arguments,
    const SandboxConfig& config) {
    
    if (!ValidateSandboxConfig(config)) {
        return nullptr;
    }

    auto process = std::make_unique<WindowsSandboxProcess>(command, arguments, config);
    return process;
}

bool ToolchainManager::SaveConfiguration(const std::string& path) {
    std::string configPath = path.empty() ? m_configPath : path;
    
    std::ofstream file(configPath);
    if (!file.is_open()) {
        return false;
    }

    // Simple JSON-like format for configuration
    file << "{\n";
    file << "  \"toolchains\": [\n";
    
    bool first = true;
    for (const auto& pair : m_toolchains) {
        if (!first) file << ",\n";
        first = false;
        
        const auto& tc = pair.second;
        file << "    {\n";
        file << "      \"id\": \"" << tc.id << "\",\n";
        file << "      \"name\": \"" << tc.name << "\",\n";
        file << "      \"version\": \"" << tc.version << "\",\n";
        file << "      \"description\": \"" << tc.description << "\",\n";
        file << "      \"executablePath\": \"" << tc.executablePath << "\"\n";
        file << "    }";
    }
    
    file << "\n  ]\n";
    file << "}\n";
    file.close();

    std::cout << "Configuration saved to: " << configPath << std::endl;
    return true;
}

bool ToolchainManager::LoadConfiguration(const std::string& path) {
    std::string configPath = path.empty() ? m_configPath : path;
    
    if (!std::filesystem::exists(configPath)) {
        return false;
    }

    // This is a simplified loader - in a real implementation,
    // you'd use a proper JSON parser
    std::cout << "Configuration loaded from: " << configPath << std::endl;
    return true;
}

void ToolchainManager::Update() {
    CleanupFinishedProcesses();
    
    // Update UI if available
    if (m_ui) {
        // UI update logic would go here
    }
}

void ToolchainManager::DetectSystemToolchains() {
    std::cout << "Detecting system toolchains..." << std::endl;

    // Detect common toolchains on Windows
    std::vector<std::pair<std::string, std::string>> commonTools = {
        {"gcc", "gcc.exe"},
        {"clang", "clang.exe"},
        {"msvc", "cl.exe"},
        {"python", "python.exe"},
        {"node", "node.exe"},
        {"java", "java.exe"},
        {"go", "go.exe"},
        {"rust", "rustc.exe"}
    };

    for (const auto& tool : commonTools) {
        // Simple PATH search - in reality, you'd do more sophisticated detection
        std::string command = "where " + tool.second + " >nul 2>&1";
        if (system(command.c_str()) == 0) {
            ToolchainInfo info;
            info.id = tool.first;
            info.name = tool.first;
            info.version = "detected";
            info.description = "System-detected " + tool.first + " toolchain";
            info.executablePath = tool.second;
            
            RegisterToolchain(info);
        }
    }
}

bool ToolchainManager::ValidateSandboxConfig(const SandboxConfig& config) {
    // Basic validation
    if (!config.workingDirectory.empty() && !std::filesystem::exists(config.workingDirectory)) {
        return false;
    }
    
    return true;
}

std::string ToolchainManager::GenerateProcessId() {
    return "proc_" + std::to_string(m_nextProcessId++);
}

void ToolchainManager::CleanupFinishedProcesses() {
    m_activeProcesses.erase(
        std::remove_if(m_activeProcesses.begin(), m_activeProcesses.end(),
            [](const std::unique_ptr<SandboxProcess>& process) {
                return !process || !process->IsRunning();
            }),
        m_activeProcesses.end()
    );
}

} // namespace Toolchain
} // namespace Hyperion