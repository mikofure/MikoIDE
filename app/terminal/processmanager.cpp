#include "processmanager.hpp"
#include <iostream>
#include <vector>

ProcessManager::ProcessManager()
    : m_hChildStdInRead(nullptr)
    , m_hChildStdInWrite(nullptr)
    , m_hChildStdOutRead(nullptr)
    , m_hChildStdOutWrite(nullptr)
    , m_running(false)
{
    ZeroMemory(&m_processInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&m_startupInfo, sizeof(STARTUPINFOA));
}

ProcessManager::~ProcessManager() {
    Shutdown();
}

bool ProcessManager::Initialize(const std::string& command) {
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    // Create pipes for stdin
    if (!CreatePipe(&m_hChildStdInRead, &m_hChildStdInWrite, &saAttr, 0)) {
        std::cerr << "Failed to create stdin pipe" << std::endl;
        return false;
    }

    // Create pipes for stdout
    if (!CreatePipe(&m_hChildStdOutRead, &m_hChildStdOutWrite, &saAttr, 0)) {
        std::cerr << "Failed to create stdout pipe" << std::endl;
        CloseHandle(m_hChildStdInRead);
        CloseHandle(m_hChildStdInWrite);
        return false;
    }

    // Ensure the read handle to the pipe for stdout is not inherited
    if (!SetHandleInformation(m_hChildStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "Failed to set handle information for stdout read" << std::endl;
        CloseHandle(m_hChildStdInRead);
        CloseHandle(m_hChildStdInWrite);
        CloseHandle(m_hChildStdOutRead);
        CloseHandle(m_hChildStdOutWrite);
        return false;
    }

    // Ensure the write handle to the pipe for stdin is not inherited
    if (!SetHandleInformation(m_hChildStdInWrite, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "Failed to set handle information for stdin write" << std::endl;
        CloseHandle(m_hChildStdInRead);
        CloseHandle(m_hChildStdInWrite);
        CloseHandle(m_hChildStdOutRead);
        CloseHandle(m_hChildStdOutWrite);
        return false;
    }

    // Set up startup info
    m_startupInfo.cb = sizeof(STARTUPINFOA);
    m_startupInfo.hStdError = m_hChildStdOutWrite;
    m_startupInfo.hStdOutput = m_hChildStdOutWrite;
    m_startupInfo.hStdInput = m_hChildStdInRead;
    m_startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process
    std::string cmdLine = command;
    BOOL bSuccess = CreateProcessA(
        nullptr,
        const_cast<char*>(cmdLine.c_str()),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &m_startupInfo,
        &m_processInfo
    );

    if (!bSuccess) {
        std::cerr << "Failed to create process: " << GetLastError() << std::endl;
        CloseHandle(m_hChildStdInRead);
        CloseHandle(m_hChildStdInWrite);
        CloseHandle(m_hChildStdOutRead);
        CloseHandle(m_hChildStdOutWrite);
        return false;
    }

    // Close handles to the pipes no longer needed by the parent process
    CloseHandle(m_hChildStdOutWrite);
    CloseHandle(m_hChildStdInRead);
    m_hChildStdOutWrite = nullptr;
    m_hChildStdInRead = nullptr;

    m_running = true;

    // Start the read thread
    m_readThread = std::thread(&ProcessManager::ReadOutputThread, this);

    return true;
}

void ProcessManager::Shutdown() {
    m_running = false;

    if (m_processInfo.hProcess) {
        TerminateProcess(m_processInfo.hProcess, 0);
        WaitForSingleObject(m_processInfo.hProcess, 1000);
        CloseHandle(m_processInfo.hProcess);
        CloseHandle(m_processInfo.hThread);
        ZeroMemory(&m_processInfo, sizeof(PROCESS_INFORMATION));
    }

    if (m_readThread.joinable()) {
        m_readThread.join();
    }

    if (m_hChildStdInWrite) {
        CloseHandle(m_hChildStdInWrite);
        m_hChildStdInWrite = nullptr;
    }

    if (m_hChildStdOutRead) {
        CloseHandle(m_hChildStdOutRead);
        m_hChildStdOutRead = nullptr;
    }

    if (m_hChildStdInRead) {
        CloseHandle(m_hChildStdInRead);
        m_hChildStdInRead = nullptr;
    }

    if (m_hChildStdOutWrite) {
        CloseHandle(m_hChildStdOutWrite);
        m_hChildStdOutWrite = nullptr;
    }
}

void ProcessManager::SendInput(const std::string& input) {
    if (!m_hChildStdInWrite || !m_running) {
        return;
    }

    DWORD bytesWritten;
    WriteFile(m_hChildStdInWrite, input.c_str(), static_cast<DWORD>(input.length()), &bytesWritten, nullptr);
    FlushFileBuffers(m_hChildStdInWrite);
}

void ProcessManager::Update() {
    // Check if process is still running
    if (m_processInfo.hProcess) {
        DWORD exitCode;
        if (GetExitCodeProcess(m_processInfo.hProcess, &exitCode)) {
            if (exitCode != STILL_ACTIVE) {
                m_running = false;
            }
        }
    }
}

void ProcessManager::SetOutputCallback(OutputCallback callback) {
    m_outputCallback = callback;
}

bool ProcessManager::IsRunning() const {
    return m_running;
}

void ProcessManager::ReadOutputThread() {
    const DWORD bufferSize = 1024; // Smaller buffer for more frequent updates
    std::vector<char> buffer(bufferSize);
    DWORD bytesRead;

    while (m_running) {
        BOOL success = ReadFile(m_hChildStdOutRead, buffer.data(), bufferSize, &bytesRead, nullptr);
        
        if (success && bytesRead > 0) {
            std::string output(buffer.data(), bytesRead);
            ProcessOutput(output);
        } else {
            // Check if the pipe was broken
            DWORD error = GetLastError();
            if (error == ERROR_BROKEN_PIPE || error == ERROR_NO_DATA) {
                m_running = false;
                break;
            }
            
            // Reduced delay for more responsive output processing
            Sleep(1);
        }
    }
}

void ProcessManager::ProcessOutput(const std::string& output) {
    if (m_outputCallback) {
        m_outputCallback(output);
    }
}