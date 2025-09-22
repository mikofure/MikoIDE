#include "cli.h"

bool file_exists(const char* path) {
    if (!path) return false;
    
#ifdef _WIN32
    return _access(path, 0) == 0;
#else
    return access(path, F_OK) == 0;
#endif
}

bool is_directory(const char* path) {
    if (!path) return false;
    
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return (stat(path, &st) == 0) && S_ISDIR(st.st_mode);
#endif
}

char* get_absolute_path(const char* path) {
    if (!path) return NULL;
    
#ifdef _WIN32
    char* abs_path = malloc(PATH_MAX);
    if (!abs_path) return NULL;
    
    if (_fullpath(abs_path, path, PATH_MAX) == NULL) {
        free(abs_path);
        return NULL;
    }
    return abs_path;
#else
    return realpath(path, NULL);
#endif
}

void normalize_path(char* path) {
    if (!path) return;
    
    for (char* p = path; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = PATH_SEPARATOR;
        }
    }
}

int open_file_or_directory(const char* path) {
    if (!path) {
        fprintf(stderr, "Error: No path specified\n");
        return 1;
    }

    char* abs_path = get_absolute_path(path);
    if (!abs_path) {
        fprintf(stderr, "Error: Could not resolve path: %s\n", path);
        return 1;
    }

    if (!file_exists(abs_path)) {
        fprintf(stderr, "Error: Path does not exist: %s\n", abs_path);
        free(abs_path);
        return 1;
    }

    char hyperion_path[PATH_MAX];
    char exe_path[PATH_MAX];

#ifdef _WIN32
    // หาตำแหน่งจริงของ hyprn.exe
    DWORD result = GetModuleFileNameA(NULL, exe_path, sizeof(exe_path));
    if (result == 0 || result >= sizeof(exe_path)) {
        fprintf(stderr, "Error: Could not get executable path\n");
        free(abs_path);
        return 1;
    }

    // exe_path = ...\bin\hyprn.exe
    char* last_sep = strrchr(exe_path, PATH_SEPARATOR);
    if (last_sep) *last_sep = '\0'; // => ...\bin

    // ขึ้นไป parent directory (root ที่มี Hyperion.exe)
    last_sep = strrchr(exe_path, PATH_SEPARATOR);
    if (last_sep) *last_sep = '\0'; // => root

    snprintf(hyperion_path, sizeof(hyperion_path), "%s%cHyperion.exe", exe_path, PATH_SEPARATOR);

#else
    // Unix-like systems
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1) {
        fprintf(stderr, "Error: Could not get executable path\n");
        free(abs_path);
        return 1;
    }
    exe_path[len] = '\0';

    char* last_sep = strrchr(exe_path, PATH_SEPARATOR);
    if (last_sep) *last_sep = '\0'; // bin
    last_sep = strrchr(exe_path, PATH_SEPARATOR);
    if (last_sep) *last_sep = '\0'; // root

    snprintf(hyperion_path, sizeof(hyperion_path), "%s%cHyperion", exe_path, PATH_SEPARATOR);
#endif

    printf("Launching Hyperion at: %s\n", hyperion_path);

    char command[2048];
    snprintf(command, sizeof(command), "\"%s\" \"%s\"", hyperion_path, abs_path);

    int result_code = system(command);
    if (result_code != 0) {
        fprintf(stderr, "Error: Failed to launch Hyperion (code %d)\n", result_code);
    }

    free(abs_path);
    return result_code;
}

int list_directory(const char* path, bool recursive) {
    if (!path) path = ".";
    
    if (!file_exists(path)) {
        fprintf(stderr, "Error: Directory does not exist: %s\n", path);
        return 1;
    }
    
    if (!is_directory(path)) {
        fprintf(stderr, "Error: Not a directory: %s\n", path);
        return 1;
    }

    printf("Contents of %s:\n", path);

#ifdef _WIN32
    WIN32_FIND_DATAA find_data;
    char search_path[PATH_MAX];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);
    
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Could not read directory: %s\n", path);
        return 1;
    }
    
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }
        
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s\\%s", path, find_data.cFileName);
        
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            printf("  📁 %s/\n", find_data.cFileName);
            if (recursive) {
                list_directory(full_path, recursive);
            }
        } else {
            printf("  📄 %s\n", find_data.cFileName);
        }
    } while (FindNextFileA(hFind, &find_data));
    
    FindClose(hFind);
#else
    DIR* dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "Error: Could not read directory: %s\n", path);
        return 1;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (is_directory(full_path)) {
            printf("  📁 %s/\n", entry->d_name);
            if (recursive) {
                list_directory(full_path, recursive);
            }
        } else {
            printf("  📄 %s\n", entry->d_name);
        }
    }
    
    closedir(dir);
#endif

    return 0;
}

int create_new_project(const char* path, const char* template_name) {
    if (!path) {
        fprintf(stderr, "Error: No project path specified\n");
        return 1;
    }

    printf("Creating new project: %s\n", path);
    
    if (file_exists(path)) {
        fprintf(stderr, "Error: Path already exists: %s\n", path);
        return 1;
    }

    // Create directory
#ifdef _WIN32
    if (_mkdir(path) != 0) {
#else
    if (mkdir(path, 0755) != 0) {
#endif
        fprintf(stderr, "Error: Could not create directory: %s\n", path);
        return 1;
    }

    char file_path[PATH_MAX];
    
    if (template_name && strcmp(template_name, "react-app") == 0) {
        snprintf(file_path, sizeof(file_path), "%s%cpackage.json", path, PATH_SEPARATOR);
        FILE* f = fopen(file_path, "w");
        if (f) {
            fprintf(f, "{\n");
            fprintf(f, "  \"name\": \"%s\",\n", path);
            fprintf(f, "  \"version\": \"1.0.0\",\n");
            fprintf(f, "  \"dependencies\": {\n");
            fprintf(f, "    \"react\": \"^18.0.0\",\n");
            fprintf(f, "    \"react-dom\": \"^18.0.0\"\n");
            fprintf(f, "  }\n");
            fprintf(f, "}\n");
            fclose(f);
        }
        
        snprintf(file_path, sizeof(file_path), "%s%cindex.html", path, PATH_SEPARATOR);
        f = fopen(file_path, "w");
        if (f) {
            fprintf(f, "<!DOCTYPE html>\n");
            fprintf(f, "<html>\n<head>\n  <title>%s</title>\n</head>\n", path);
            fprintf(f, "<body>\n  <div id=\"root\"></div>\n</body>\n</html>\n");
            fclose(f);
        }
    } else {
        snprintf(file_path, sizeof(file_path), "%s%cREADME.md", path, PATH_SEPARATOR);
        FILE* f = fopen(file_path, "w");
        if (f) {
            fprintf(f, "# %s\n\n", path);
            fprintf(f, "A new project created with Hyperion CLI.\n");
            fclose(f);
        }
    }

    printf("Project created successfully: %s\n", path);
    return 0;
}
