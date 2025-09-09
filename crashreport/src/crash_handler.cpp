#include "crash_handler.h"
#include "minidump_writer.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>
#include <wininet.h>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "wininet.lib")
#endif

CrashHandler* CrashHandler::instance_ = nullptr;

CrashHandler::CrashHandler(const std::string& database_path, const std::string& upload_url)
    : database_path_(database_path)
    , upload_url_(upload_url)
    , running_(false)
#ifdef _WIN32
    , pipe_handle_(INVALID_HANDLE_VALUE)
#endif
{
    instance_ = this;
    minidump_writer_ = std::make_unique<MinidumpWriter>(database_path_);
}

CrashHandler::~CrashHandler() {
    Stop();
    instance_ = nullptr;
}

void CrashHandler::AddAnnotation(const std::string& key, const std::string& value) {
    annotations_[key] = value;
}

int CrashHandler::Run() {
    if (!Initialize()) {
        std::cerr << "Failed to initialize crash handler" << std::endl;
        return 1;
    }
    
    running_ = true;
    
#ifdef _WIN32
    // Set up unhandled exception filter
    SetUnhandledExceptionFilter(UnhandledExceptionFilter);
    
    // Create service thread
    HANDLE service_thread = CreateThread(
        nullptr,
        0,
        ServiceThreadProc,
        this,
        0,
        nullptr
    );
    
    if (service_thread == nullptr) {
        std::cerr << "Failed to create service thread" << std::endl;
        return 1;
    }
    
    // Wait for service thread
    WaitForSingleObject(service_thread, INFINITE);
    CloseHandle(service_thread);
#endif
    
    return 0;
}

void CrashHandler::Stop() {
    running_ = false;
    
#ifdef _WIN32
    if (pipe_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(pipe_handle_);
        pipe_handle_ = INVALID_HANDLE_VALUE;
    }
#endif
}

bool CrashHandler::Initialize() {
    // Create database directory if it doesn't exist
    if (!database_path_.empty()) {
        std::filesystem::create_directories(database_path_);
    }
    
    std::cout << "Crash handler initialized" << std::endl;
    std::cout << "Database path: " << database_path_ << std::endl;
    std::cout << "Upload URL: " << upload_url_ << std::endl;
    
    return true;
}

bool CrashHandler::ProcessCrashReport(const std::string& crash_data) {
    try {
        // Generate minidump
        std::string minidump_path = minidump_writer_->WriteCrashDump(crash_data, annotations_);
        
        if (minidump_path.empty()) {
            std::cerr << "Failed to write minidump" << std::endl;
            return false;
        }
        
        std::cout << "Minidump written to: " << minidump_path << std::endl;
        
        // Upload crash report if URL is provided
        if (!upload_url_.empty()) {
            return UploadCrashReport(minidump_path);
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error processing crash report: " << e.what() << std::endl;
        return false;
    }
}

bool CrashHandler::UploadCrashReport(const std::string& minidump_path) {
#ifdef _WIN32
    try {
        HINTERNET hInternet = InternetOpenA(
            "CrashHandler/1.0",
            INTERNET_OPEN_TYPE_DIRECT,
            nullptr,
            nullptr,
            0
        );
        
        if (!hInternet) {
            std::cerr << "Failed to initialize WinINet" << std::endl;
            return false;
        }
        
        // TODO: Implement actual HTTP upload
        std::cout << "Would upload " << minidump_path << " to " << upload_url_ << std::endl;
        
        InternetCloseHandle(hInternet);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error uploading crash report: " << e.what() << std::endl;
        return false;
    }
#else
    std::cout << "Upload not implemented for this platform" << std::endl;
    return false;
#endif
}

#ifdef _WIN32
DWORD WINAPI CrashHandler::ServiceThreadProc(LPVOID param) {
    CrashHandler* handler = static_cast<CrashHandler*>(param);
    return handler->ServiceThread();
}

DWORD CrashHandler::ServiceThread() {
    std::cout << "Crash handler service thread started" << std::endl;
    
    while (running_) {
        // Service loop - wait for crash reports
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Crash handler service thread stopped" << std::endl;
    return 0;
}

LONG WINAPI CrashHandler::UnhandledExceptionFilter(EXCEPTION_POINTERS* exception_info) {
    if (instance_) {
        std::cout << "Unhandled exception caught, generating crash report..." << std::endl;
        
        // Generate crash data string
        std::string crash_data = "Exception occurred";
        
        instance_->ProcessCrashReport(crash_data);
    }
    
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif