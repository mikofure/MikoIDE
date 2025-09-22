#ifndef CLI_H
#define CLI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
    #define PATH_SEPARATOR '\\'
    #define PATH_MAX 260
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #define PATH_SEPARATOR '/'
    #include <limits.h>
#endif

// CLI Command types
typedef enum {
    CMD_HELP,
    CMD_VERSION,
    CMD_OPEN,
    CMD_NEW,
    CMD_LIST,
    CMD_WORKSPACE,
    CMD_EXTENSION,
    CMD_TUNNEL,
    CMD_SERVE,
    CMD_UNKNOWN
} cli_command_t;

// CLI Options structure
typedef struct {
    cli_command_t command;
    char* target_path;
    char* workspace_name;
    char* extension_name;
    bool verbose;
    bool force;
    bool recursive;
    int port;
    char* host;
} cli_options_t;

// Function declarations
cli_options_t* parse_arguments(int argc, char* argv[]);
void free_cli_options(cli_options_t* options);
int execute_command(cli_options_t* options);
void print_help(void);
void print_version(void);

// File operations
int open_file_or_directory(const char* path);
int create_new_project(const char* path, const char* template_name);
int list_directory(const char* path, bool recursive);

// Workspace operations
int create_workspace(const char* name, const char* path);
int open_workspace(const char* path);

// Extension operations
int install_extension(const char* extension_name);
int list_extensions(void);
int uninstall_extension(const char* extension_name);

// Server operations
int start_dev_server(int port, const char* host);
int create_tunnel(const char* name);

// Utility functions
bool file_exists(const char* path);
bool is_directory(const char* path);
char* get_absolute_path(const char* path);
void normalize_path(char* path);

#endif // CLI_H