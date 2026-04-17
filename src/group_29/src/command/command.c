#include "command.h"

static CommandEntry commands[] = {
    { "help", "[command]", "Display this screen", command_help },
    { "echo", "<content>", "Prints arguments to screen", command_echo },
    { "clear", "[none]", "Clears the shell content", command_clear },
    { "setusername", "<new-username>", "Changes the shell user display name", command_set_username }
};

static const int command_count = sizeof(commands) / sizeof(commands[0]);

// static functions

static CommandColors get_command_colors() {
    CommandColors colors;
    colors.name = VgaColor(vga_black, vga_light_gray);
    colors.options = VgaColor(vga_black, vga_dark_gray);
    colors.description = VgaColor(vga_black, vga_white);

    return colors;
}

static void display_help_header() {
    const CommandColors colors = get_command_colors();

    // print_color(" <command> [options]: <description>");

    print_color(" <command> ", colors.name);
    print_color(" [options]", colors.options);
    print_color(" <description>\n", colors.description);
    print_color(" ----------------------------------------\n", colors.options);
}

static void display_command_info(const char name[], const char options[], const char description[]) {
    const CommandColors colors = get_command_colors();
    
    print(" ");
    print_color(name, colors.name);
    print(" ");
    print_color(options, colors.options);
    print_color(": ", VgaColor(vga_black, vga_white));
    print_color(description, colors.description);
    print("\n");
}

static int8_t check_argument_count(uint8_t actual_count, uint8_t required_count) {
    if (actual_count < required_count) {
        return COMMAND_TOO_FEW_ARGUMENTS;
    }

    if (actual_count > required_count) {
        return COMMAND_TOO_MANY_ARGUMENTS;
    }

    return COMMAND_VALID_ARGUMENTS;
}

// Commands

int8_t command_help(int argument_count, char *arguments[]) {
    if (command_count < 1) {
        return 1;
    }

    if (argument_count > 2) {
        return COMMAND_TOO_MANY_ARGUMENTS;
    }

    uint8_t has_arguments = 0;
    uint8_t command_found = 0;
    uint8_t command_index = 0;

    if (argument_count > 1) {
        has_arguments = 1;
    }

    for (int i = 0; i < command_count; i++) {
        if (strcmp(commands[i].name, arguments[1]) == 0) {
            // display_command_info(commands[i].name, commands[i].options, commands[i].description);
            command_index = i;
            command_found = 1;
        }
    }

    // Only help for one command but failed to find that command
    if (has_arguments && !command_found) {
        print_color(" Could not find command: ", VgaColor(vga_black, vga_light_red));
        print_color(arguments[1], VgaColor(vga_black, vga_light_red));
        print("\n");
        return COMMAND_UNKNOWN_ARGUMENT;
    }

    // No error so we can show help header
    display_help_header();

    // Only help for one command and found the command
    if (has_arguments && command_found) {
        display_command_info(commands[command_index].name, commands[command_index].options, commands[command_index].description);
        return 0;
    }

    // Show all commands
    for (int i = 0; i < command_count; i++) {
        display_command_info(commands[i].name, commands[i].options, commands[i].description);
    }

    return 0;
}

int8_t command_echo(int argument_count, char* arguments[]) {
    if (argument_count < 2) {
        return COMMAND_TOO_FEW_ARGUMENTS;
    }

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

int8_t command_clear(int argument_count, char *arguments[]) {
    if (argument_count > 1) {
        return COMMAND_TOO_MANY_ARGUMENTS;
    }

    clear_screen();

    return COMMAND_SHELL_CLEARED;
}

int8_t command_set_username(int argument_count, char* arguments[]) {
    int8_t argument_result = check_argument_count(argument_count, 2);
    if (argument_result != COMMAND_VALID_ARGUMENTS) return argument_result;

    // uint8_t length = strlen(arguments[1]);
    // if (length > USER_MAX_USERNAME_LENGTH) {
    //     return COMMAND_ARGUMENT_INVALID;
    // }

    int8_t result = user_set_username(arguments[1]);
    if (result == -1) {
        return COMMAND_ARGUMENT_USERNAME_TOO_LONG;
    }

    return 0;
}

// Parsing/handling

int8_t run_command(char* input) {
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


int8_t handle_command(int argument_count, char* arguments[]) {
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

int8_t parse_command_aguments(char* input, char* arguments[], int max_args) {
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
