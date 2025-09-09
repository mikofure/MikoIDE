#pragma once

#include <string>
#include <map>

class MinidumpWriter {
public:
    explicit MinidumpWriter(const std::string& output_directory);
    ~MinidumpWriter();

    // Write crash dump to file
    std::string WriteCrashDump(
        const std::string& crash_data,
        const std::map<std::string, std::string>& annotations
    );

private:
    std::string output_directory_;

#ifdef _WIN32
    // Windows-specific minidump writing
    std::string WriteWindowsMinidump(
        const std::string& filepath,
        const std::string& crash_data,
        const std::map<std::string, std::string>& annotations
    );
#endif

    // Generic crash dump writing (fallback)
    std::string WriteGenericCrashDump(
        const std::string& filepath,
        const std::string& crash_data,
        const std::map<std::string, std::string>& annotations
    );

    // Write annotations to separate file
    void WriteAnnotationsFile(
        const std::string& filepath,
        const std::string& crash_data,
        const std::map<std::string, std::string>& annotations
    );
};