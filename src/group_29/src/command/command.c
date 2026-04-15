#include "command.h"

static CommandEntry commands[] = {
    { "echo", command_echo },
    { "setusername", command_set_username }
};

static const int command_count = sizeof(commands) / sizeof(commands[0]);


// Commands

int command_echo(int argument_count, char *arguments[]) {
    print(" ");

    for (int i = 1; i < argument_count; i++) {
        print(arguments[i]);

        if (i + 1 < argument_count) {
            print(" ");
        }
    }

    print("\n");
    return 0;
}

int8_t command_set_username(int argument_count, char* arguments[]) {
    if (argument_count > 2) {
        return -1;
    }

    user_set_username(arguments[1]);
    return 0;
}

// Parsing/handling

int run_command(char* input) {
    if (!input) {
        return COMMAND_STATUS_INVALID;
    }

    char* arguments[MAX_ARGS];

    int argument_count = parse_command_aguments(input, arguments, MAX_ARGS);

    if (argument_count == 0) {
        return COMMAND_STATUS_INVALID;
    }

    return handle_command(argument_count, arguments);
}


int handle_command(int argument_count, char* arguments[]) {
    if (argument_count == 0) {
        return COMMAND_STATUS_INVALID;
    }

    for (int i = 0; i < command_count; i++) {
        if (strcmp(arguments[0], commands[i].name) == 0) {
            return commands[i].command_function(argument_count, arguments);
        }
    }

    return COMMAND_STATUS_UNKNOWN;
}

int parse_command_aguments(char* input, char* arguments[], int max_args) {
    int argument_count = 0;

    while (*input && argument_count < max_args) {

        // skip spaces
        while (*input == ' ') {
            input++;
        }

        if (*input == '\0') {
            break;
        }

        arguments[argument_count++] = input;

        // find end of token
        while (*input && *input != ' ') {
            input++;
        }

        if (*input) {
            *input = '\0';
            input++;
        }
    }

    return argument_count;
}
