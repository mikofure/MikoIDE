#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

class MinidumpWriter;

class CrashHandler {
public:
    CrashHandler(const std::string& database_path, const std::string& upload_url);
    ~CrashHandler();

    // Add annotation to crash reports
    void AddAnnotation(const std::string& key, const std::string& value);
    
    // Start the crash handler service
    int Run();
    
    // Stop the crash handler service
    void Stop();

private:
    std::string database_path_;
    std::string upload_url_;
    std::map<std::string, std::string> annotations_;
    std::unique_ptr<MinidumpWriter> minidump_writer_;
    bool running_;
    
#ifdef _WIN32
    HANDLE pipe_handle_;
    static DWORD WINAPI ServiceThreadProc(LPVOID param);
    DWORD ServiceThread();
    
    // Exception handling
    static LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* exception_info);
    static CrashHandler* instance_;
#endif
    
    // Initialize the crash handler
    bool Initialize();
    
    // Process crash report
    bool ProcessCrashReport(const std::string& crash_data);
    
    // Upload crash report
    bool UploadCrashReport(const std::string& minidump_path);
};