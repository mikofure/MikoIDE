#include "cli.h"

static void print_usage(void) {
    printf("Hyperion CLI - Advanced Code Editor Command Line Interface\n\n");
    printf("Usage: hyperion [command] [options] [path]\n\n");
    printf("Commands:\n");
    printf("  .                     Open current directory in Hyperion\n");
    printf("  <path>                Open file or directory\n");
    printf("  new <template>        Create new project from template\n");
    printf("  workspace <name>      Create or open workspace\n");
    printf("  list [path]           List directory contents\n");
    printf("  ext install <name>    Install extension\n");
    printf("  ext list              List installed extensions\n");
    printf("  ext uninstall <name>  Uninstall extension\n");
    printf("  serve [port]          Start development server\n");
    printf("  tunnel <name>         Create secure tunnel\n");
    printf("  version               Show version information\n");
    printf("  help                  Show this help message\n\n");
    printf("Options:\n");
    printf("  -v, --verbose         Enable verbose output\n");
    printf("  -f, --force           Force operation\n");
    printf("  -r, --recursive       Recursive operation\n");
    printf("  -p, --port <port>     Specify port number\n");
    printf("  -h, --host <host>     Specify host address\n\n");
    printf("Examples:\n");
    printf("  hyperion .                    # Open current directory\n");
    printf("  hyperion myproject            # Open myproject directory\n");
    printf("  hyperion new react-app        # Create new React project\n");
    printf("  hyperion serve 3000           # Start server on port 3000\n");
    printf("  hyperion ext install prettier # Install Prettier extension\n");
}

cli_options_t* parse_arguments(int argc, char* argv[]) {
    cli_options_t* options = calloc(1, sizeof(cli_options_t));
    if (!options) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    // Default values
    options->command = CMD_UNKNOWN;
    options->port = 8080;
    options->host = strdup("localhost");

    if (argc < 2) {
        options->command = CMD_HELP;
        return options;
    }

    int i = 1;
    while (i < argc) {
        char* arg = argv[i];

        if (strcmp(arg, "help") == 0 || strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            options->command = CMD_HELP;
            break;
        }
        else if (strcmp(arg, "version") == 0 || strcmp(arg, "--version") == 0) {
            options->command = CMD_VERSION;
            break;
        }
        else if (strcmp(arg, ".") == 0) {
            options->command = CMD_OPEN;
            options->target_path = strdup(".");
        }
        else if (strcmp(arg, "new") == 0) {
            options->command = CMD_NEW;
            if (i + 1 < argc) {
                options->target_path = strdup(argv[++i]);
            }
        }
        else if (strcmp(arg, "workspace") == 0) {
            options->command = CMD_WORKSPACE;
            if (i + 1 < argc) {
                options->workspace_name = strdup(argv[++i]);
            }
        }
        else if (strcmp(arg, "list") == 0) {
            options->command = CMD_LIST;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options->target_path = strdup(argv[++i]);
            } else {
                options->target_path = strdup(".");
            }
        }
        else if (strcmp(arg, "ext") == 0) {
            options->command = CMD_EXTENSION;
            if (i + 1 < argc) {
                char* subcommand = argv[++i];
                if (strcmp(subcommand, "install") == 0 && i + 1 < argc) {
                    options->extension_name = strdup(argv[++i]);
                } else if (strcmp(subcommand, "uninstall") == 0 && i + 1 < argc) {
                    options->extension_name = strdup(argv[++i]);
                } else if (strcmp(subcommand, "list") == 0) {
                    // Extension list command
                }
            }
        }
        else if (strcmp(arg, "serve") == 0) {
            options->command = CMD_SERVE;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options->port = atoi(argv[++i]);
            }
        }
        else if (strcmp(arg, "tunnel") == 0) {
            options->command = CMD_TUNNEL;
            if (i + 1 < argc) {
                options->target_path = strdup(argv[++i]);
            }
        }
        else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
            options->verbose = true;
        }
        else if (strcmp(arg, "-f") == 0 || strcmp(arg, "--force") == 0) {
            options->force = true;
        }
        else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--recursive") == 0) {
            options->recursive = true;
        }
        else if (strcmp(arg, "-p") == 0 || strcmp(arg, "--port") == 0) {
            if (i + 1 < argc) {
                options->port = atoi(argv[++i]);
            }
        }
        else if (strcmp(arg, "--host") == 0) {
            if (i + 1 < argc) {
                free(options->host);
                options->host = strdup(argv[++i]);
            }
        }
        else if (arg[0] != '-') {
            // Assume it's a path to open
            if (options->command == CMD_UNKNOWN) {
                options->command = CMD_OPEN;
                options->target_path = strdup(arg);
            }
        }
        else {
            fprintf(stderr, "Unknown option: %s\n", arg);
            print_usage();
            free_cli_options(options);
            return NULL;
        }

        i++;
    }

    return options;
}

void free_cli_options(cli_options_t* options) {
    if (options) {
        free(options->target_path);
        free(options->workspace_name);
        free(options->extension_name);
        free(options->host);
        free(options);
    }
}

void print_help(void) {
    print_usage();
}

void print_version(void) {
    printf("Hyperion CLI v1.0.0\n");
    printf("Advanced Code Editor Command Line Interface\n");
    printf("Built with C for maximum performance\n");
}