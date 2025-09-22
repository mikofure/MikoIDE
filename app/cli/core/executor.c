#include "cli.h"

int execute_command(cli_options_t* options) {
    if (!options) {
        fprintf(stderr, "Error: Invalid options\n");
        return 1;
    }

    if (options->verbose) {
        printf("Executing command: %d\n", options->command);
        if (options->target_path) {
            printf("Target path: %s\n", options->target_path);
        }
    }

    switch (options->command) {
        case CMD_HELP:
            print_help();
            return 0;

        case CMD_VERSION:
            print_version();
            return 0;

        case CMD_OPEN:
            if (!options->target_path) {
                fprintf(stderr, "Error: No path specified for open command\n");
                return 1;
            }
            return open_file_or_directory(options->target_path);

        case CMD_NEW:
            if (!options->target_path) {
                fprintf(stderr, "Error: No project name specified for new command\n");
                return 1;
            }
            return create_new_project(options->target_path, "basic");

        case CMD_LIST:
            return list_directory(options->target_path ? options->target_path : ".", 
                                options->recursive);

        case CMD_WORKSPACE:
            if (!options->workspace_name) {
                fprintf(stderr, "Error: No workspace name specified\n");
                return 1;
            }
            
            // Check if workspace already exists
            char workspace_path[PATH_MAX];
            snprintf(workspace_path, sizeof(workspace_path), "%s.hyperion-workspace", 
                    options->workspace_name);
            
            if (file_exists(workspace_path)) {
                return open_workspace(workspace_path);
            } else {
                return create_workspace(options->workspace_name, NULL);
            }

        case CMD_EXTENSION:
            if (!options->extension_name) {
                // List extensions if no specific extension specified
                return list_extensions();
            } else {
                // Check if it's an install or uninstall operation
                // This would be determined by the parser based on subcommand
                return install_extension(options->extension_name);
            }

        case CMD_SERVE:
            return start_dev_server(options->port, options->host);

        case CMD_TUNNEL:
            if (!options->target_path) {
                fprintf(stderr, "Error: No tunnel name specified\n");
                return 1;
            }
            return create_tunnel(options->target_path);

        case CMD_UNKNOWN:
        default:
            fprintf(stderr, "Error: Unknown or unspecified command\n");
            print_help();
            return 1;
    }
}