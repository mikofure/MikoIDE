#include "cli.h"

// Extension registry structure
typedef struct {
    char* name;
    char* version;
    char* description;
    bool enabled;
} extension_info_t;

static char* get_extensions_dir(void) {
    char* extensions_dir = malloc(PATH_MAX);
    if (!extensions_dir) return NULL;

#ifdef _WIN32
    char* appdata = getenv("APPDATA");
    if (appdata) {
        snprintf(extensions_dir, PATH_MAX, "%s\\Hyperion\\extensions", appdata);
    } else {
        snprintf(extensions_dir, PATH_MAX, ".\\extensions");
    }
#else
    char* home = getenv("HOME");
    if (home) {
        snprintf(extensions_dir, PATH_MAX, "%s/.hyperion/extensions", home);
    } else {
        snprintf(extensions_dir, PATH_MAX, "./extensions");
    }
#endif

    return extensions_dir;
}

static void ensure_extensions_dir(const char* dir) {
    if (!file_exists(dir)) {
#ifdef _WIN32
        char command[512];
        snprintf(command, sizeof(command), "mkdir \"%s\"", dir);
        system(command);
#else
        char command[512];
        snprintf(command, sizeof(command), "mkdir -p \"%s\"", dir);
        system(command);
#endif
    }
}

int install_extension(const char* extension_name) {
    if (!extension_name) {
        fprintf(stderr, "Error: No extension name specified\n");
        return 1;
    }

    printf("Installing extension: %s\n", extension_name);

    char* extensions_dir = get_extensions_dir();
    if (!extensions_dir) {
        fprintf(stderr, "Error: Could not determine extensions directory\n");
        return 1;
    }

    ensure_extensions_dir(extensions_dir);

    // Create extension directory
    char ext_path[PATH_MAX];
    snprintf(ext_path, sizeof(ext_path), "%s%c%s", extensions_dir, PATH_SEPARATOR, extension_name);

#ifdef _WIN32
    if (_mkdir(ext_path) != 0 && errno != EEXIST) {
#else
    if (mkdir(ext_path, 0755) != 0 && errno != EEXIST) {
#endif
        fprintf(stderr, "Error: Could not create extension directory: %s\n", ext_path);
        free(extensions_dir);
        return 1;
    }

    // Create package.json for the extension
    char package_path[PATH_MAX];
    snprintf(package_path, sizeof(package_path), "%s%cpackage.json", ext_path, PATH_SEPARATOR);

    FILE* package_file = fopen(package_path, "w");
    if (!package_file) {
        fprintf(stderr, "Error: Could not create extension package file\n");
        free(extensions_dir);
        return 1;
    }

    // Write extension metadata
    fprintf(package_file, "{\n");
    fprintf(package_file, "  \"name\": \"%s\",\n", extension_name);
    fprintf(package_file, "  \"version\": \"1.0.0\",\n");
    fprintf(package_file, "  \"description\": \"Extension installed via Hyperion CLI\",\n");
    fprintf(package_file, "  \"main\": \"extension.js\",\n");
    fprintf(package_file, "  \"engines\": {\n");
    fprintf(package_file, "    \"hyperion\": \"^1.0.0\"\n");
    fprintf(package_file, "  },\n");
    fprintf(package_file, "  \"categories\": [\"Other\"],\n");
    fprintf(package_file, "  \"activationEvents\": [\"*\"],\n");
    fprintf(package_file, "  \"contributes\": {}\n");
    fprintf(package_file, "}\n");

    fclose(package_file);

    // Create basic extension.js
    char extension_js_path[PATH_MAX];
    snprintf(extension_js_path, sizeof(extension_js_path), "%s%cextension.js", ext_path, PATH_SEPARATOR);

    FILE* extension_file = fopen(extension_js_path, "w");
    if (extension_file) {
        fprintf(extension_file, "// %s Extension\n", extension_name);
        fprintf(extension_file, "const hyperion = require('hyperion');\n\n");
        fprintf(extension_file, "function activate(context) {\n");
        fprintf(extension_file, "    console.log('Extension %s is now active!');\n", extension_name);
        fprintf(extension_file, "}\n\n");
        fprintf(extension_file, "function deactivate() {\n");
        fprintf(extension_file, "    console.log('Extension %s is now deactivated!');\n", extension_name);
        fprintf(extension_file, "}\n\n");
        fprintf(extension_file, "module.exports = {\n");
        fprintf(extension_file, "    activate,\n");
        fprintf(extension_file, "    deactivate\n");
        fprintf(extension_file, "};\n");
        fclose(extension_file);
    }

    printf("Extension '%s' installed successfully\n", extension_name);
    free(extensions_dir);
    return 0;
}

int list_extensions(void) {
    char* extensions_dir = get_extensions_dir();
    if (!extensions_dir) {
        fprintf(stderr, "Error: Could not determine extensions directory\n");
        return 1;
    }

    if (!file_exists(extensions_dir)) {
        printf("No extensions installed\n");
        free(extensions_dir);
        return 0;
    }

    printf("Installed Extensions:\n");
    printf("====================\n");

#ifdef _WIN32
    WIN32_FIND_DATAA find_data;
    char search_path[PATH_MAX];
    snprintf(search_path, sizeof(search_path), "%s\\*", extensions_dir);
    
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No extensions found\n");
        free(extensions_dir);
        return 0;
    }
    
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }
        
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            char package_path[PATH_MAX];
            snprintf(package_path, sizeof(package_path), "%s\\%s\\package.json", 
                    extensions_dir, find_data.cFileName);
            
            if (file_exists(package_path)) {
                printf("📦 %s\n", find_data.cFileName);
                
                // Try to read version from package.json
                FILE* package_file = fopen(package_path, "r");
                if (package_file) {
                    char line[256];
                    while (fgets(line, sizeof(line), package_file)) {
                        if (strstr(line, "\"version\"")) {
                            char* version_start = strchr(line, ':');
                            if (version_start) {
                                version_start = strchr(version_start, '"');
                                if (version_start) {
                                    version_start++;
                                    char* version_end = strchr(version_start, '"');
                                    if (version_end) {
                                        *version_end = '\0';
                                        printf("   Version: %s\n", version_start);
                                    }
                                }
                            }
                            break;
                        }
                    }
                    fclose(package_file);
                }
            }
        }
    } while (FindNextFileA(hFind, &find_data));
    
    FindClose(hFind);
#else
    DIR* dir = opendir(extensions_dir);
    if (!dir) {
        printf("No extensions found\n");
        free(extensions_dir);
        return 0;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char ext_path[PATH_MAX];
        snprintf(ext_path, sizeof(ext_path), "%s/%s", extensions_dir, entry->d_name);
        
        if (is_directory(ext_path)) {
            char package_path[PATH_MAX];
            snprintf(package_path, sizeof(package_path), "%s/package.json", ext_path);
            
            if (file_exists(package_path)) {
                printf("📦 %s\n", entry->d_name);
                
                // Try to read version from package.json
                FILE* package_file = fopen(package_path, "r");
                if (package_file) {
                    char line[256];
                    while (fgets(line, sizeof(line), package_file)) {
                        if (strstr(line, "\"version\"")) {
                            char* version_start = strchr(line, ':');
                            if (version_start) {
                                version_start = strchr(version_start, '"');
                                if (version_start) {
                                    version_start++;
                                    char* version_end = strchr(version_start, '"');
                                    if (version_end) {
                                        *version_end = '\0';
                                        printf("   Version: %s\n", version_start);
                                    }
                                }
                            }
                            break;
                        }
                    }
                    fclose(package_file);
                }
            }
        }
    }
    
    closedir(dir);
#endif

    free(extensions_dir);
    return 0;
}

int uninstall_extension(const char* extension_name) {
    if (!extension_name) {
        fprintf(stderr, "Error: No extension name specified\n");
        return 1;
    }

    char* extensions_dir = get_extensions_dir();
    if (!extensions_dir) {
        fprintf(stderr, "Error: Could not determine extensions directory\n");
        return 1;
    }

    char ext_path[PATH_MAX];
    snprintf(ext_path, sizeof(ext_path), "%s%c%s", extensions_dir, PATH_SEPARATOR, extension_name);

    if (!file_exists(ext_path)) {
        fprintf(stderr, "Error: Extension not found: %s\n", extension_name);
        free(extensions_dir);
        return 1;
    }

    printf("Uninstalling extension: %s\n", extension_name);

    // Remove extension directory
#ifdef _WIN32
    char command[512];
    snprintf(command, sizeof(command), "rmdir /s /q \"%s\"", ext_path);
    int result = system(command);
#else
    char command[512];
    snprintf(command, sizeof(command), "rm -rf \"%s\"", ext_path);
    int result = system(command);
#endif

    if (result == 0) {
        printf("Extension '%s' uninstalled successfully\n", extension_name);
    } else {
        fprintf(stderr, "Error: Could not uninstall extension: %s\n", extension_name);
        free(extensions_dir);
        return 1;
    }

    free(extensions_dir);
    return 0;
}