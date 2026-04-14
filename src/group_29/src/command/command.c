#include "command.h"

void run_shell_command(int argument_count, char* arguments[]) {
    if (argument_count == 0) {
        return -1;
    }

    // echo command
    if (strcmp(arguments[0], "echo") == 0) {
        for (int i = 1; i < argument_count; i++) {
            // assume you have some print function
            print(arguments[i]);

            if (i + 1 < argument_count) {
                print(" ");
            }
        }

        print("\n");
        return 0;
    }

    // unknown command
    print("Unknown command\n");
    return -1;
}