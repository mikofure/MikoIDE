#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <direct.h>  // For _getcwd
#include <io.h>     // For _access
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#define VERSION "1.0.0"
#define PROGRAM_NAME "mikoide"
#define MAX_ARGS 256
#define MAX_PATH_LEN 4096
#define MAX_LINE_LEN 1024

// Supported package managers
typedef enum {
    PM_PIP,
    PM_NPM,
    PM_PNPM,
    PM_YARN,
    PM_BUN,
    PM_UNKNOWN
} PackageManager;

// Project types
typedef enum {
    PROJECT_PYTHON,
    PROJECT_NODE,
    PROJECT_UNKNOWN
} ProjectType;

// Command line options
typedef struct {
    bool help;
    bool version;
    bool new_window;
    bool wait;
    bool diff;
    bool add;
    bool reuse_window;
    int goto_line;
    int goto_column;
    PackageManager package_manager;
    bool pm_specified;
    char **files;
    int file_count;
    char **pm_args;
    int pm_arg_count;
} Options;

// Function prototypes
void print_help(void);
void print_version(void);
void print_package_manager_help(void);
PackageManager string_to_package_manager(const char *str);
const char* package_manager_to_string(PackageManager pm);
ProjectType detect_project_type(void);
PackageManager get_default_package_manager(ProjectType type);
PackageManager parse_project_config(void);
bool file_exists(const char *path);
int parse_args(int argc, char *argv[], Options *options);
int handle_package_manager(Options *options);
int launch_mikoide(Options *options);
char* resolve_path(const char *input_path);
void free_options(Options *options);

// Print help message
void print_help(void) {
    printf(
        "MikoIDE CLI - Command Line Interface for MikoIDE\n"
        "\n"
        "Usage: %s [options] [paths...]\n"
        "       %s -pm[=manager] <command> [args...]\n"
        "\n"
        "Options:\n"
        "  -h, --help              Show this help message\n"
        "  -v, --version           Show version information\n"
        "  -n, --new-window        Open a new window\n"
        "  -w, --wait              Wait for the files to be closed before returning\n"
        "  -g, --goto <line:col>   Go to line and column (e.g., -g 10:5)\n"
        "  -d, --diff              Compare files (requires exactly 2 files)\n"
        "  -a, --add               Add folder(s) to the last active window\n"
        "  -r, --reuse-window      Force to open a file or folder in an already opened window\n"
        "  -pm[=manager]           Run package manager command (pip, npm, pnpm, yarn, bun)\n"
        "\n"
        "Examples:\n"
        "  %s .                   Open current directory\n"
        "  %s file.txt            Open file.txt\n"
        "  %s -n project/         Open project/ in new window\n"
        "  %s -g 10:5 file.txt    Open file.txt and go to line 10, column 5\n"
        "  %s -d file1.txt file2.txt  Compare two files\n"
        "  %s -w file.txt         Open file.txt and wait for it to be closed\n"
        "  %s -pm=npm install     Install dependencies using npm\n"
        "  %s -pm install         Auto-detect and install dependencies\n"
        "\n",
        PROGRAM_NAME, PROGRAM_NAME, PROGRAM_NAME, PROGRAM_NAME, PROGRAM_NAME,
        PROGRAM_NAME, PROGRAM_NAME, PROGRAM_NAME, PROGRAM_NAME, PROGRAM_NAME
    );
    
    print_package_manager_help();
    
    printf(
        "For more information, visit: https://github.com/mikofure/mikoide\n"
        "\n"
    );
}

// Print version information
void print_version(void) {
    printf("%s %s\n", PROGRAM_NAME, VERSION);
}

// Print package manager help
void print_package_manager_help(void) {
    printf(
        "Package Manager Options:\n"
        "  -pm[=manager]           Run package manager command\n"
        "                          Supported: pip, npm, pnpm, yarn, bun\n"
        "                          If no manager specified, auto-detect from project\n"
        "\n"
        "Package Manager Examples:\n"
        "  mikoide -pm=npm install         Install dependencies using npm\n"
        "  mikoide -pm=pip install flask   Install flask using pip\n"
        "  mikoide -pm install             Auto-detect and install dependencies\n"
        "  mikoide -pm=yarn add react      Add react using yarn\n"
        "  mikoide -pm=bun install         Install dependencies using bun\n"
        "\n"
        "Project Detection:\n"
        "  The CLI can auto-detect package managers based on:\n"
        "  - Python: requirements.txt, setup.py, pyproject.toml, Pipfile\n"
        "  - Node.js: package.json, package-lock.json, yarn.lock, pnpm-lock.yaml, bun.lockb\n"
        "  - miko.yml: package_manager: <manager>\n"
        "\n"
        "miko.yml Example:\n"
        "  package_manager: npm\n"
        "\n"
    );
}

// Convert string to package manager enum
PackageManager string_to_package_manager(const char *str) {
    if (strcmp(str, "pip") == 0) return PM_PIP;
    if (strcmp(str, "npm") == 0) return PM_NPM;
    if (strcmp(str, "pnpm") == 0) return PM_PNPM;
    if (strcmp(str, "yarn") == 0) return PM_YARN;
    if (strcmp(str, "bun") == 0) return PM_BUN;
    return PM_UNKNOWN;
}

// Convert package manager enum to string
const char* package_manager_to_string(PackageManager pm) {
    switch (pm) {
        case PM_PIP: return "pip";
        case PM_NPM: return "npm";
        case PM_PNPM: return "pnpm";
        case PM_YARN: return "yarn";
        case PM_BUN: return "bun";
        default: return "unknown";
    }
}

// Check if file exists
bool file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

// Detect project type based on files
ProjectType detect_project_type(void) {
    // Check for Python project files
    if (file_exists("requirements.txt") || 
        file_exists("setup.py") || 
        file_exists("pyproject.toml") || 
        file_exists("Pipfile")) {
        return PROJECT_PYTHON;
    }
    
    // Check for Node.js project files
    if (file_exists("package.json") || 
        file_exists("package-lock.json") || 
        file_exists("yarn.lock") || 
        file_exists("pnpm-lock.yaml") || 
        file_exists("bun.lockb")) {
        return PROJECT_NODE;
    }
    
    return PROJECT_UNKNOWN;
}

// Get default package manager for project type
PackageManager get_default_package_manager(ProjectType type) {
    switch (type) {
        case PROJECT_PYTHON: return PM_PIP;
        case PROJECT_NODE: return PM_NPM;
        default: return PM_UNKNOWN;
    }
}

// Parse miko.yml configuration file
PackageManager parse_project_config(void) {
    FILE *file = fopen("miko.yml", "r");
    if (!file) {
        // No config file, detect project type
        ProjectType type = detect_project_type();
        return get_default_package_manager(type);
    }
    
    char line[MAX_LINE_LEN];
    PackageManager pm = PM_UNKNOWN;
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Simple YAML parsing for package_manager field
        if (strncmp(line, "package_manager:", 16) == 0) {
            char *value = line + 16;
            // Skip whitespace
            while (*value == ' ' || *value == '\t') value++;
            pm = string_to_package_manager(value);
            break;
        }
    }
    
    fclose(file);
    
    // If no package manager specified in config, detect project type
    if (pm == PM_UNKNOWN) {
        ProjectType type = detect_project_type();
        pm = get_default_package_manager(type);
    }
    
    return pm;
}

// Parse command line arguments
int parse_args(int argc, char *argv[], Options *options) {
    // Initialize options
    memset(options, 0, sizeof(Options));
    options->goto_line = -1;
    options->goto_column = -1;
    options->package_manager = PM_UNKNOWN;
    options->files = malloc(MAX_ARGS * sizeof(char*));
    options->pm_args = malloc(MAX_ARGS * sizeof(char*));
    
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            options->help = true;
        } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) {
            options->version = true;
        } else if (strcmp(arg, "-n") == 0 || strcmp(arg, "--new-window") == 0) {
            options->new_window = true;
        } else if (strcmp(arg, "-w") == 0 || strcmp(arg, "--wait") == 0) {
            options->wait = true;
        } else if (strcmp(arg, "-d") == 0 || strcmp(arg, "--diff") == 0) {
            options->diff = true;
        } else if (strcmp(arg, "-a") == 0 || strcmp(arg, "--add") == 0) {
            options->add = true;
        } else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--reuse-window") == 0) {
            options->reuse_window = true;
        } else if (strcmp(arg, "-g") == 0 || strcmp(arg, "--goto") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --goto requires an argument\n");
                return 1;
            }
            i++;
            char *goto_arg = argv[i];
            char *colon = strchr(goto_arg, ':');
            if (colon) {
                *colon = '\0';
                options->goto_line = atoi(goto_arg);
                options->goto_column = atoi(colon + 1);
            } else {
                options->goto_line = atoi(goto_arg);
            }
        } else if (strcmp(arg, "-pm") == 0) {
            // Package manager without specific manager (auto-detect)
            options->pm_specified = true;
            // Collect remaining arguments as package manager commands
            i++;
            while (i < argc) {
                options->pm_args[options->pm_arg_count++] = argv[i++];
            }
            break;
        } else if (strncmp(arg, "-pm=", 4) == 0) {
            // Package manager with specific manager
            char *pm_value = arg + 4;
            options->package_manager = string_to_package_manager(pm_value);
            options->pm_specified = true;
            // Collect remaining arguments as package manager commands
            i++;
            while (i < argc) {
                options->pm_args[options->pm_arg_count++] = argv[i++];
            }
            break;
        } else if (arg[0] == '-') {
            fprintf(stderr, "Error: Unknown option '%s'\n", arg);
            return 1;
        } else {
            // It's a file or directory path
            options->files[options->file_count++] = arg;
        }
    }
    
    return 0;
}

// Handle package manager commands
int handle_package_manager(Options *options) {
    PackageManager pm = options->package_manager;
    
    if (pm == PM_UNKNOWN) {
        // Auto-detect package manager from project
        pm = parse_project_config();
        if (pm == PM_UNKNOWN) {
            fprintf(stderr, "Error: Could not detect package manager for this project\n");
            fprintf(stderr, "Please specify one using -pm=<manager> or add package_manager to miko.yml\n");
            fprintf(stderr, "Supported package managers: pip, npm, pnpm, yarn, bun\n");
            return 1;
        }
        printf("Detected package manager: %s\n", package_manager_to_string(pm));
    }
    
    if (options->pm_arg_count == 0) {
        fprintf(stderr, "Error: No command specified for package manager\n");
        fprintf(stderr, "Example: mikoide -pm=%s install\n", package_manager_to_string(pm));
        return 1;
    }
    
    // Build command
    char *cmd_args[MAX_ARGS];
    int cmd_argc = 0;
    
    cmd_args[cmd_argc++] = (char*)package_manager_to_string(pm);
    for (int i = 0; i < options->pm_arg_count; i++) {
        cmd_args[cmd_argc++] = options->pm_args[i];
    }
    cmd_args[cmd_argc] = NULL;
    
    printf("Executing: %s", package_manager_to_string(pm));
    for (int i = 0; i < options->pm_arg_count; i++) {
        printf(" %s", options->pm_args[i]);
    }
    printf("\n");
    
#ifdef _WIN32
    int result = _spawnvp(_P_WAIT, package_manager_to_string(pm), cmd_args);
    if (result == -1) {
        perror("Failed to execute command");
        return 1;
    }
    return result;
#else
    pid_t pid = fork();
    if (pid == 0) {
        execvp(package_manager_to_string(pm), cmd_args);
        perror("Failed to execute command");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        perror("Failed to fork");
        return 1;
    }
#endif
}

// Resolve path to absolute path
char* resolve_path(const char *input_path) {
    char *resolved = malloc(MAX_PATH_LEN);
    if (!resolved) return NULL;
    
#ifdef _WIN32
    if (_fullpath(resolved, input_path, MAX_PATH_LEN) == NULL) {
        free(resolved);
        return NULL;
    }
#else
    if (realpath(input_path, resolved) == NULL) {
        free(resolved);
        return NULL;
    }
#endif
    
    return resolved;
}

// Launch MikoIDE with arguments
int launch_mikoide(Options *options) {
    char *cmd_args[MAX_ARGS];
    int cmd_argc = 0;
    
    // Try to find MikoIDE executable in common locations
    const char *possible_paths[] = {
        "../MikoIDE.exe",
        "build/Release/MikoIDE.exe",
        "build/Debug/MikoIDE.exe",
        "../build/Release/MikoIDE.exe",
        "../build/Debug/MikoIDE.exe",
        NULL
    };
    
    const char *exe_path = NULL;
    for (int i = 0; possible_paths[i]; i++) {
        if (file_exists(possible_paths[i])) {
            exe_path = possible_paths[i];
            break;
        }
    }
    
    if (!exe_path) {
        fprintf(stderr, "Error: Could not find MikoIDE executable\n");
        fprintf(stderr, "Please ensure MikoIDE is built and available in the expected location\n");
        return 1;
    }
    
    cmd_args[cmd_argc++] = (char*)exe_path;
    
    // Add command line arguments based on options
    if (options->new_window) {
        cmd_args[cmd_argc++] = "--new-window";
    }
    if (options->wait) {
        cmd_args[cmd_argc++] = "--wait";
    }
    if (options->reuse_window) {
        cmd_args[cmd_argc++] = "--reuse-window";
    }
    if (options->add) {
        cmd_args[cmd_argc++] = "--add";
    }
    if (options->diff) {
        cmd_args[cmd_argc++] = "--diff";
    }
    if (options->goto_line >= 0) {
        char *goto_arg = malloc(64);
        if (options->goto_column >= 0) {
            snprintf(goto_arg, 64, "--goto=%d:%d", options->goto_line, options->goto_column);
        } else {
            snprintf(goto_arg, 64, "--goto=%d", options->goto_line);
        }
        cmd_args[cmd_argc++] = goto_arg;
    }
    
    // Add file/directory paths
    for (int i = 0; i < options->file_count; i++) {
        char *resolved_path = resolve_path(options->files[i]);
        if (resolved_path) {
            if (!file_exists(resolved_path)) {
                printf("Warning: Path '%s' does not exist\n", options->files[i]);
            }
            cmd_args[cmd_argc++] = resolved_path;
        }
    }
    
    // If no files specified, open current directory
    if (options->file_count == 0) {
        char *cwd = malloc(MAX_PATH_LEN);
        if (getcwd(cwd, MAX_PATH_LEN)) {
            cmd_args[cmd_argc++] = cwd;
        }
    }
    
    cmd_args[cmd_argc] = NULL;
    
#ifdef _WIN32
    if (options->wait) {
        int result = _spawnv(_P_WAIT, exe_path, cmd_args);
        if (result == -1) {
            perror("Failed to launch MikoIDE");
            return 1;
        }
        return result;
    } else {
        int result = _spawnv(_P_NOWAIT, exe_path, cmd_args);
        if (result == -1) {
            perror("Failed to launch MikoIDE");
            return 1;
        }
        printf("MikoIDE launched successfully\n");
        return 0;
    }
#else
    pid_t pid = fork();
    if (pid == 0) {
        execv(exe_path, cmd_args);
        perror("Failed to launch MikoIDE");
        exit(1);
    } else if (pid > 0) {
        if (options->wait) {
            int status;
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        } else {
            printf("MikoIDE launched successfully\n");
            return 0;
        }
    } else {
        perror("Failed to fork");
        return 1;
    }
#endif
}

// Free allocated memory in options
void free_options(Options *options) {
    if (options->files) {
        free(options->files);
    }
    if (options->pm_args) {
        free(options->pm_args);
    }
}

// Main function
int main(int argc, char *argv[]) {
    Options options;
    
    if (parse_args(argc, argv, &options) != 0) {
        fprintf(stderr, "\nUse '%s --help' for usage information\n", PROGRAM_NAME);
        free_options(&options);
        return 1;
    }
    
    // Handle help and version first
    if (options.help) {
        print_help();
        free_options(&options);
        return 0;
    }
    
    if (options.version) {
        print_version();
        free_options(&options);
        return 0;
    }
    
    // Handle package manager commands
    if (options.pm_specified) {
        int result = handle_package_manager(&options);
        free_options(&options);
        return result;
    }
    
    // Validate options
    if (options.diff && options.file_count != 2) {
        fprintf(stderr, "Error: --diff requires exactly 2 files\n");
        free_options(&options);
        return 1;
    }
    
    // Launch MikoIDE
    int result = launch_mikoide(&options);
    free_options(&options);
    return result;
}