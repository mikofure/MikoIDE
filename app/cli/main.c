#include "core/cli.h"
#include <signal.h>

// Global flag for graceful shutdown
static volatile bool running = true;

// Signal handler for graceful shutdown
void signal_handler(int signum) {
    printf("\n\n🛑 Received interrupt signal (%d)\n", signum);
    printf("Shutting down Hyperion CLI gracefully...\n");
    running = false;
}

int main(int argc, char* argv[]) {
    // Set up signal handlers for graceful shutdown
#ifndef _WIN32
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
#endif

    // Enable UTF-8 output on Windows
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    // Enable ANSI escape sequences on Windows 10+
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif

    // Display banner for interactive usage
    if (argc == 1) {
        printf("\n");
        printf("🚀 \033[1;36mHyperion CLI\033[0m - Advanced Code Editor Command Line Interface\n");
        printf("   Version 1.0.0 | Built with C for maximum performance\n");
        printf("\n");
        printf("   Usage: \033[1mhyperion [command] [options] [path]\033[0m\n");
        printf("   Type '\033[1mhyperion help\033[0m' for detailed usage information\n");
        printf("\n");
        printf("   Quick examples:\n");
        printf("   • \033[32mhyperion .\033[0m                    # Open current directory\n");
        printf("   • \033[32mhyperion myproject\033[0m            # Open myproject directory\n");
        printf("   • \033[32mhyperion new react-app\033[0m        # Create new React project\n");
        printf("   • \033[32mhyperion serve 3000\033[0m           # Start server on port 3000\n");
        printf("\n");
        return 0;
    }

    // Parse command line arguments
    cli_options_t* options = parse_arguments(argc, argv);
    if (!options) {
        fprintf(stderr, "Error: Failed to parse command line arguments\n");
        return 1;
    }

    // Execute the command
    int result = execute_command(options);

    // Clean up
    free_cli_options(options);

    if (result == 0) {
        if (options && options->verbose) {
            printf("\n✅ Command completed successfully\n");
        }
    } else {
        fprintf(stderr, "\n❌ Command failed with exit code: %d\n", result);
    }

    return result;
}