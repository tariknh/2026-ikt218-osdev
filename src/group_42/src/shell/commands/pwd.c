#include <stdio.h>

#include "shell/shell.h"

int cmd_pwd(int argc, char** argv) {
    // We ignore argc/argv because pwd doesn't need them
    (void)argc;
    (void)argv;

    // Call the shell's getter
    const char* current_path = shell_get_cwd();

    if (current_path) {
        printf("%s\n", current_path);
    } else {
        printf("Error: Could not retrieve CWD\n");
        return 1;
    }

    return 0;
}
