#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>
#endif

#include "crash_handler.h"

namespace {

void PrintUsage(const std::string& program_name) {
    std::cerr << "Usage: " << program_name << " [options]\n"
              << "\n"
              << "Crash Report Handler\n"
              << "\n"
              << "Options:\n"
              << "  --database=PATH          Path to crash report database\n"
              << "  --url=URL               URL to upload crash reports\n"
              << "  --annotation=KEY=VALUE  Add annotation to crash reports\n"
              << "  --help                  Show this help message\n"
              << "\n";
}

struct HandlerConfig {
    std::string database_path;
    std::string upload_url;
    std::vector<std::pair<std::string, std::string>> annotations;
};

HandlerConfig ParseArguments(int argc, char* argv[]) {
    HandlerConfig config;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg.substr(0, 11) == "--database=") {
            config.database_path = arg.substr(11);
        } else if (arg.substr(0, 6) == "--url=") {
            config.upload_url = arg.substr(6);
        } else if (arg.substr(0, 13) == "--annotation=") {
            std::string annotation = arg.substr(13);
            size_t eq_pos = annotation.find('=');
            if (eq_pos != std::string::npos) {
                config.annotations.emplace_back(
                    annotation.substr(0, eq_pos),
                    annotation.substr(eq_pos + 1)
                );
            }
        }
    }
    
    return config;
}

} // namespace

int main(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    // Check for help flag
    for (const auto& arg : args) {
        if (arg == "--help" || arg == "-h") {
            PrintUsage(args[0]);
            return 0;
        }
    }

    try {
        HandlerConfig config = ParseArguments(argc, argv);
        
        // Ensure database path is not empty
        if (config.database_path.empty()) {
            config.database_path = "./crashes";
        }
        
        // Initialize crash handler
        CrashHandler handler(config.database_path, config.upload_url);
        
        // Add annotations
        for (const auto& [key, value] : config.annotations) {
            handler.AddAnnotation(key, value);
        }
        
        // Start the handler service
        std::cout << "Crash handler started..." << std::endl;
        return handler.Run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}