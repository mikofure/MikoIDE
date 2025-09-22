#include "cli.h"

int create_workspace(const char* name, const char* path) {
    if (!name) {
        fprintf(stderr, "Error: No workspace name specified\n");
        return 1;
    }

    char workspace_path[PATH_MAX];
    if (path) {
        snprintf(workspace_path, sizeof(workspace_path), "%s", path);
    } else {
        // Use current directory if no path specified
        snprintf(workspace_path, sizeof(workspace_path), ".%c%s.hyperion-workspace", PATH_SEPARATOR, name);
    }

    printf("Creating workspace: %s\n", name);

    // Create workspace directory if it doesn't exist
    if (!file_exists(workspace_path)) {
#ifdef _WIN32
        if (_mkdir(workspace_path) != 0) {
#else
        if (mkdir(workspace_path, 0755) != 0) {
#endif
            fprintf(stderr, "Error: Could not create workspace directory: %s\n", workspace_path);
            return 1;
        }
    }

    // Create workspace configuration file
    char config_path[PATH_MAX];
    snprintf(config_path, sizeof(config_path), "%s%cworkspace.json", workspace_path, PATH_SEPARATOR);
    
    FILE* config_file = fopen(config_path, "w");
    if (!config_file) {
        fprintf(stderr, "Error: Could not create workspace configuration\n");
        return 1;
    }

    // Write workspace configuration
    fprintf(config_file, "{\n");
    fprintf(config_file, "  \"name\": \"%s\",\n", name);
    fprintf(config_file, "  \"version\": \"1.0.0\",\n");
    fprintf(config_file, "  \"created\": \"%lld\",\n", (long long)time(NULL));
    fprintf(config_file, "  \"folders\": [\n");
    fprintf(config_file, "    {\n");
    fprintf(config_file, "      \"path\": \".\"\n");
    fprintf(config_file, "    }\n");
    fprintf(config_file, "  ],\n");
    fprintf(config_file, "  \"settings\": {\n");
    fprintf(config_file, "    \"editor.fontSize\": 14,\n");
    fprintf(config_file, "    \"editor.tabSize\": 4,\n");
    fprintf(config_file, "    \"editor.insertSpaces\": true\n");
    fprintf(config_file, "  },\n");
    fprintf(config_file, "  \"extensions\": {\n");
    fprintf(config_file, "    \"recommendations\": []\n");
    fprintf(config_file, "  }\n");
    fprintf(config_file, "}\n");

    fclose(config_file);

    // Create .hyperion directory for workspace-specific settings
    char hyperion_dir[PATH_MAX];
    snprintf(hyperion_dir, sizeof(hyperion_dir), "%s%c.hyperion", workspace_path, PATH_SEPARATOR);
    
#ifdef _WIN32
    _mkdir(hyperion_dir);
#else
    mkdir(hyperion_dir, 0755);
#endif

    // Create launch.json for debugging configuration
    char launch_path[PATH_MAX];
    snprintf(launch_path, sizeof(launch_path), "%s%claunch.json", hyperion_dir, PATH_SEPARATOR);
    
    FILE* launch_file = fopen(launch_path, "w");
    if (launch_file) {
        fprintf(launch_file, "{\n");
        fprintf(launch_file, "  \"version\": \"0.2.0\",\n");
        fprintf(launch_file, "  \"configurations\": []\n");
        fprintf(launch_file, "}\n");
        fclose(launch_file);
    }

    // Create tasks.json for build tasks
    char tasks_path[PATH_MAX];
    snprintf(tasks_path, sizeof(tasks_path), "%s%ctasks.json", hyperion_dir, PATH_SEPARATOR);
    
    FILE* tasks_file = fopen(tasks_path, "w");
    if (tasks_file) {
        fprintf(tasks_file, "{\n");
        fprintf(tasks_file, "  \"version\": \"2.0.0\",\n");
        fprintf(tasks_file, "  \"tasks\": []\n");
        fprintf(tasks_file, "}\n");
        fclose(tasks_file);
    }

    printf("Workspace '%s' created successfully at: %s\n", name, workspace_path);
    return 0;
}

int open_workspace(const char* path) {
    if (!path) {
        fprintf(stderr, "Error: No workspace path specified\n");
        return 1;
    }

    char workspace_file[PATH_MAX];
    
    // Check if path is a workspace file or directory
    if (strstr(path, ".hyperion-workspace") != NULL) {
        snprintf(workspace_file, sizeof(workspace_file), "%s", path);
    } else {
        // Look for workspace file in the directory
        snprintf(workspace_file, sizeof(workspace_file), "%s%cworkspace.json", path, PATH_SEPARATOR);
    }

    if (!file_exists(workspace_file)) {
        fprintf(stderr, "Error: Workspace file not found: %s\n", workspace_file);
        return 1;
    }

    printf("Opening workspace: %s\n", workspace_file);

    // Read and validate workspace configuration
    FILE* config_file = fopen(workspace_file, "r");
    if (!config_file) {
        fprintf(stderr, "Error: Could not read workspace configuration\n");
        return 1;
    }

    // Simple validation - check if it's a valid JSON-like structure
    char buffer[1024];
    bool valid_workspace = false;
    while (fgets(buffer, sizeof(buffer), config_file)) {
        if (strstr(buffer, "\"name\"") && strstr(buffer, "\"folders\"")) {
            valid_workspace = true;
            break;
        }
    }
    fclose(config_file);

    if (!valid_workspace) {
        fprintf(stderr, "Error: Invalid workspace configuration\n");
        return 1;
    }

    // Launch Hyperion with workspace
#ifdef _WIN32
    char command[512];
    char* workspace_dir = strdup(workspace_file);
    char* last_sep = strrchr(workspace_dir, PATH_SEPARATOR);
    if (last_sep) *last_sep = '\0';
    
    snprintf(command, sizeof(command), "start \"\" \"%s\"", workspace_dir);
    system(command);
    free(workspace_dir);
#else
    // For Linux/macOS
    char command[512];
    snprintf(command, sizeof(command), "hyperion \"%s\"", workspace_file);
    system(command);
#endif

    printf("Workspace opened successfully\n");
    return 0;
}