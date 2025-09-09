#include "minidump_writer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <iomanip>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")
#endif

MinidumpWriter::MinidumpWriter(const std::string& output_directory)
    : output_directory_(output_directory.empty() ? "./crashes" : output_directory) {
    // Ensure output directory exists
    if (!output_directory_.empty()) {
        std::filesystem::create_directories(output_directory_);
    }
}

MinidumpWriter::~MinidumpWriter() = default;

std::string MinidumpWriter::WriteCrashDump(
    const std::string& crash_data,
    const std::map<std::string, std::string>& annotations) {
    
    try {
        // Generate timestamp-based filename
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
        ss << "_" << std::setfill('0') << std::setw(3) << ms.count();
        
        std::string filename = "crash_" + ss.str() + ".dmp";
        std::string filepath = (std::filesystem::path(output_directory_) / filename).string();
        
#ifdef _WIN32
        return WriteWindowsMinidump(filepath, crash_data, annotations);
#else
        return WriteGenericCrashDump(filepath, crash_data, annotations);
#endif
    } catch (const std::exception& e) {
        std::cerr << "Error writing crash dump: " << e.what() << std::endl;
        return "";
    }
}

#ifdef _WIN32
std::string MinidumpWriter::WriteWindowsMinidump(
    const std::string& filepath,
    const std::string& crash_data,
    const std::map<std::string, std::string>& annotations) {
    
    HANDLE hFile = CreateFileA(
        filepath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create minidump file: " << filepath << std::endl;
        return "";
    }
    
    // Get current process and thread
    HANDLE hProcess = GetCurrentProcess();
    DWORD processId = GetCurrentProcessId();
    DWORD threadId = GetCurrentThreadId();
    
    // Create exception information (simulated)
    EXCEPTION_POINTERS exceptionPointers = {};
    EXCEPTION_RECORD exceptionRecord = {};
    CONTEXT context = {};
    
    exceptionRecord.ExceptionCode = EXCEPTION_ACCESS_VIOLATION;
    exceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    exceptionPointers.ExceptionRecord = &exceptionRecord;
    exceptionPointers.ContextRecord = &context;
    
    MINIDUMP_EXCEPTION_INFORMATION exceptionInfo = {};
    exceptionInfo.ThreadId = threadId;
    exceptionInfo.ExceptionPointers = &exceptionPointers;
    exceptionInfo.ClientPointers = FALSE;
    
    // Write minidump
    BOOL result = MiniDumpWriteDump(
        hProcess,
        processId,
        hFile,
        MiniDumpNormal,
        &exceptionInfo,
        nullptr,
        nullptr
    );
    
    CloseHandle(hFile);
    
    if (!result) {
        std::cerr << "MiniDumpWriteDump failed with error: " << GetLastError() << std::endl;
        std::filesystem::remove(filepath);
        return "";
    }
    
    // Write annotations file
    WriteAnnotationsFile(filepath + ".txt", crash_data, annotations);
    
    return filepath;
}
#endif

std::string MinidumpWriter::WriteGenericCrashDump(
    const std::string& filepath,
    const std::string& crash_data,
    const std::map<std::string, std::string>& annotations) {
    
    try {
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to create crash dump file: " << filepath << std::endl;
            return "";
        }
        
        // Write crash data
        file << "CRASH DUMP\n";
        file << "Timestamp: " << std::chrono::system_clock::now().time_since_epoch().count() << "\n";
        file << "Data: " << crash_data << "\n";
        
        file.close();
        
        // Write annotations file
        WriteAnnotationsFile(filepath + ".txt", crash_data, annotations);
        
        return filepath;
    } catch (const std::exception& e) {
        std::cerr << "Error writing generic crash dump: " << e.what() << std::endl;
        return "";
    }
}

void MinidumpWriter::WriteAnnotationsFile(
    const std::string& filepath,
    const std::string& crash_data,
    const std::map<std::string, std::string>& annotations) {
    
    try {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to create annotations file: " << filepath << std::endl;
            return;
        }
        
        file << "Crash Report Annotations\n";
        file << "========================\n\n";
        
        file << "Crash Data: " << crash_data << "\n\n";
        
        file << "Annotations:\n";
        for (const auto& [key, value] : annotations) {
            file << "  " << key << ": " << value << "\n";
        }
        
        file << "\nSystem Information:\n";
#ifdef _WIN32
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        file << "  Processor Architecture: " << sysInfo.wProcessorArchitecture << "\n";
        file << "  Number of Processors: " << sysInfo.dwNumberOfProcessors << "\n";
        
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        file << "  Total Physical Memory: " << memInfo.ullTotalPhys / (1024 * 1024) << " MB\n";
        file << "  Available Physical Memory: " << memInfo.ullAvailPhys / (1024 * 1024) << " MB\n";
#endif
        
        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Error writing annotations file: " << e.what() << std::endl;
    }
}