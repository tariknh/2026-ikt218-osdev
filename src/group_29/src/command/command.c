#include "command.h"

static CommandEntry commands[] = {
    { "help", "[command]", "Display this screen", command_help },
    { "echo", "<content>", "Prints arguments to screen", command_echo },
    { "clear", "[none]", "Clears the shell content", command_clear },
    { "setusername", "<new-username>", "Changes the shell user display name", command_set_username },
    { "heapinfo", "[none]", "Displays information about the heap of memory", command_heapinfo },
    { "disktest", "[read|write]", "Reads or writes one raw disk sector", command_disktest },
    { "format", "[none]", "Formats the tiny persistent filesystem", command_format },
    { "ls", "[none]", "Lists files in the tiny filesystem", command_ls },
    { "write", "<file> <text>", "Creates or overwrites a file", command_write_file },
    { "cat", "<file>", "Prints a file from the tiny filesystem", command_cat }
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

    print_color(" <command>", colors.name);
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

static char* join_arguments(int argument_count, char* arguments[], int start_index) {
    int index;
    int total_length = 0;
    int position = 0;
    char* output;

    for (index = start_index; index < argument_count; ++index) {
        total_length += strlen(arguments[index]);
        if (index + 1 < argument_count) {
            total_length += 1;
        }
    }

    output = (char*)malloc((size_t)total_length + 1U);
    if (output == NULL) {
        return NULL;
    }

    for (index = start_index; index < argument_count; ++index) {
        int char_index = 0;
        while (arguments[index][char_index] != '\0') {
            output[position++] = arguments[index][char_index++];
        }

        if (index + 1 < argument_count) {
            output[position++] = ' ';
        }
    }

    output[position] = '\0';
    return output;
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

int8_t command_heapinfo(int argument_count, char *arguments[]) {
    if (argument_count > 1) {
        return COMMAND_TOO_MANY_ARGUMENTS;
    }

    MemoryDebugData layout = get_memory_layout();
    HeapInformation h = get_heap_stats();
    char* s;

    uint8_t label_color = VgaColor(vga_black, vga_light_gray);
    uint8_t dim_color   = VgaColor(vga_black, vga_dark_gray);
    uint8_t value_color = VgaColor(vga_black, vga_white);
    uint8_t good_color  = VgaColor(vga_black, vga_green);
    uint8_t warn_color  = VgaColor(vga_black, vga_yellow);
    uint8_t bad_color   = VgaColor(vga_black, vga_light_red);
    uint8_t title_color = VgaColor(vga_black, vga_light_gray);

    // fragmentation color logic
    uint8_t frag_color = value_color;
    if (h.fragmentation_per_mille > 700) {
        frag_color = bad_color;
    } else if (h.fragmentation_per_mille > 400) {
        frag_color = warn_color;
    } else {
        frag_color = good_color;
    }

    // header
    print_color(" Heap Information\n", title_color);
    print_color(" ----------------\n", dim_color);

    // --- Kernel ---
    print_color(" General        ", label_color);

    print_color("Kernel End:", dim_color);
    s = format_string("%d ", (int32_t)layout.kernel_end);
    print_color(s, value_color); free(s);

    print_color("Heap Start:", dim_color);
    s = format_string("%d ", (int32_t)layout.heap_start);
    print_color(s, value_color); free(s);

    print_color("Heap End:", dim_color);
    s = format_string("%d\n", (int32_t)layout.heap_end);
    print_color(s, value_color); free(s);



    // memory
    print_color(" Memory (Heap)  ", label_color);

    print_color("Total:", dim_color);
    s = format_string("%d ", (int32_t)h.total_payload_bytes);
    print_color(s, value_color); free(s);

    print_color("Used:", dim_color);
    s = format_string("%d ", (int32_t)h.used_bytes);
    print_color(s, bad_color); free(s);

    print_color("Free:", dim_color);
    s = format_string("%d\n", (int32_t)h.free_bytes);
    print_color(s, good_color); free(s);


    // blocks
    print_color(" Blocks         ", label_color);

    print_color("Total:", dim_color);
    s = format_string("%d ", (int32_t)h.total_blocks);
    print_color(s, value_color); free(s);

    print_color("Used:", dim_color);
    s = format_string("%d ", (int32_t)h.used_blocks);
    print_color(s, bad_color); free(s);

    print_color("Free:", dim_color);
    s = format_string("%d\n", (int32_t)h.free_blocks);
    print_color(s, good_color); free(s);


    // fragmentation
    print_color(" Fragmentation  ", label_color);

    print_color("Largest:", dim_color);
    s = format_string("%d ", (int32_t)h.largest_free_block);
    print_color(s, good_color); free(s);

    print_color("Smallest:", dim_color);
    s = format_string("%d ", (int32_t)h.smallest_free_block);
    print_color(s, warn_color); free(s);

    print_color("Level:", dim_color);
    s = format_string("%d/1000\n", (int32_t)h.fragmentation_per_mille);
    print_color(s, frag_color); free(s);


    // allocation
    print_color(" Allocation     ", label_color);

    print_color("Largest:", dim_color);
    s = format_string("%d ", (int32_t)h.largest_used_block);
    print_color(s, value_color); free(s);

    print_color("Smallest:", dim_color);
    s = format_string("%d ", (int32_t)h.smallest_used_block);
    print_color(s, value_color); free(s);

    print_color("AvgUsed:", dim_color);
    s = format_string("%d ", (int32_t)h.avg_used_block_size);
    print_color(s, value_color); free(s);

    print_color("AvgFree:", dim_color);
    s = format_string("%d\n", (int32_t)h.avg_free_block_size);
    print_color(s, value_color); free(s);


    // metadata
    print_color(" Incl. Metadata ", label_color);

    print_color("Total:", dim_color);
    s = format_string("%d ", (int32_t)h.total_bytes_with_meta);
    print_color(s, value_color); free(s);

    print_color("Used:", dim_color);
    s = format_string("%d ", (int32_t)h.used_bytes_with_meta);
    print_color(s, bad_color); free(s);

    print_color("Free:", dim_color);
    s = format_string("%d\n", (int32_t)h.free_bytes_with_meta);
    print_color(s, good_color); free(s);

    return COMMAND_SUCCESS;
}

int8_t command_disktest(int argument_count, char *arguments[])
{
    static const char test_message[] = "Test text saved on disk";
    uint8_t sector[ATA_SECTOR_SIZE];
    uint32_t index;

    if (argument_count > 2) {
        return COMMAND_TOO_MANY_ARGUMENTS;
    }

    for (index = 0U; index < ATA_SECTOR_SIZE; ++index) {
        sector[index] = 0U;
    }

    if (argument_count == 1 || strcmp(arguments[1], "write") == 0) {
        for (index = 0U; test_message[index] != '\0'; ++index) {
            sector[index] = (uint8_t)test_message[index];
        }

        if (!ata_write_sector(10U, sector)) {
            return TINYFS_STATUS_FAILED_TO_WRITE_SECTOR_10;
        }

        print(" Wrote test message to sector 10\n");
        if (argument_count == 2) {
            return 0;
        }
    }

    if (argument_count == 2 && strcmp(arguments[1], "read") != 0 && strcmp(arguments[1], "write") != 0) {
        return TINYFS_STATUS_DISKTEST_INVALID_USAGE;
    }

    if (!ata_read_sector(10U, sector)) {
        return TINYFS_STATUS_FAILED_TO_READ_SECTOR_10;
    }

    sector[ATA_SECTOR_SIZE - 1U] = 0U;
    print(" Sector 10: ");
    print((char*)sector);
    print("\n");
    return 0;
}

int8_t command_format(int argument_count, char* arguments[]) {
    int8_t status;

    (void)arguments;
    if (argument_count > 1) {
        return COMMAND_TOO_MANY_ARGUMENTS;
    }

    status = tinyfs_format();
    if (status != TINYFS_STATUS_OK) {
        return status;
    }

    print(" TinyFS formatted\n");
    return 0;
}

int8_t command_ls(int argument_count, char* arguments[]) {
    int8_t status;

    (void)arguments;
    if (argument_count > 1) {
        return COMMAND_TOO_MANY_ARGUMENTS;
    }

    status = tinyfs_list();
    if (status != TINYFS_STATUS_OK) {
        return status;
    }

    return 0;
}

int8_t command_write_file(int argument_count, char* arguments[]) {
    char* content;
    int8_t status;

    if (argument_count < 3) {
        return COMMAND_TOO_FEW_ARGUMENTS;
    }

    content = join_arguments(argument_count, arguments, 2);
    if (content == NULL) {
        return TINYFS_STATUS_NOT_ENOUGH_MEMORY_TO_BUILD_FILE_CONTENT;
    }

    status = tinyfs_write_file(arguments[1], content);
    free(content);

    if (status != TINYFS_STATUS_OK) {
        return status;
    }

    print(" File saved\n");
    return 0;
}

int8_t command_cat(int argument_count, char* arguments[]) {
    char* content;
    uint32_t size_bytes;
    int8_t status;

    if (argument_count != 2) {
        return check_argument_count((uint8_t)argument_count, 2U);
    }

    status = tinyfs_read_file(arguments[1], &content, &size_bytes);
    if (status != TINYFS_STATUS_OK) {
        return status;
    }

    (void)size_bytes;
    print(" ");
    print(content);
    print("\n");
    free(content);
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
