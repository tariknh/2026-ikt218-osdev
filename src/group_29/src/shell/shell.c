#include "shell.h"

static ShellState shell_state;

// static void shell_copy_username(const char* username) {
//     uint16_t i = 0;

//     if (username == 0 || *username == '\0') {
//         username = "user";
//     }

//     while (username[i] != '\0' && i < SHELL_MAX_USERNAME_LENGTH) {
//         shell_state.username[i] = username[i];
//         ++i;
//     }

//     shell_state.username[i] = '\0';
// }

static uint8_t get_shell_prompt_width(void) {
    return (uint8_t)(strlen(user_get_username()) + 3);
}

static uint16_t shell_max_edit_length(void) {
    uint8_t prompt_width = get_shell_prompt_width();

    if (prompt_width >= VGA_TERMINAL_WIDTH) {
        return 0;
    }

    return (uint16_t)(VGA_TERMINAL_WIDTH - prompt_width);
}

static void shell_sync_cursor(void) {
    main_interface.cursor.CalculateRowColFromMemoryPosition(&main_interface.cursor);
    VgaTextModeCursorSyncHardware(&main_interface.cursor);
}

static void shell_copy_current_line_to_submitted(void) {
    uint16_t i;

    for (i = 0; i < shell_state.current_length; ++i) {
        shell_state.submitted_line[i] = shell_state.current_line[i];
    }

    shell_state.submitted_line[i] = '\0';
}

static void shell_print_failure_message(const char* label, const char* value) {
    print_color(label, VgaColor(vga_black, vga_light_red));
    print_color(value, VgaColor(vga_black, vga_light_red));
    print_color("\n", VgaColor(vga_black, vga_light_red));
}

static void shell_begin_prompt(void) {
    char marker_string[2];

    if (main_interface.cursor.col != 0U) {
        print("\n");
    }

    shell_state.current_length = 0;
    shell_state.current_line[0] = '\0';
    marker_string[0] = shell_state.next_prompt_failed ? 'x' : '>';
    marker_string[1] = '\0';

    print(" SH: ");
    print_color(user_get_username(), VgaColor(vga_black, vga_light_green));
    // print();
    print(" ");
    print_color(marker_string, VgaColor(vga_black, shell_state.next_prompt_failed ? vga_light_red : vga_light_green));
    print(" ");
    shell_state.next_prompt_failed = 0U;
}

static void shell_backspace(void) {
    if (shell_state.current_length == 0U) {
        return;
    }

    --shell_state.current_length;
    shell_state.current_line[shell_state.current_length] = '\0';

    --main_interface.cursor.memory_position;
    *main_interface.cursor.memory_position = (uint16_t)(VgaColor(vga_black, vga_white) << 8 | ' ');
    shell_sync_cursor();
}

static void shell_append_char(char value) {
    char output[2] = {value, '\0'};

    if (shell_state.current_length >= shell_max_edit_length()) {
        return;
    }

    shell_state.current_line[shell_state.current_length] = value;
    ++shell_state.current_length;
    shell_state.current_line[shell_state.current_length] = '\0';
    print(output);
}

static void shell_submit_current_line(void) {
    int result;

    print("\n");

    if (shell_state.current_length == 0U) {
        shell_begin_prompt();
        return;
    }

    shell_copy_current_line_to_submitted();
    result = run_command(shell_state.current_line);
    if (result < 0) {
        shell_state.next_prompt_failed = 1U;

        if (result == COMMAND_STATUS_UNKNOWN) {
            shell_print_failure_message(" Error > Unknown command: ", shell_state.submitted_line);
            // shell_print_failure_message("        Failed line: ", shell_state.submitted_line);
        }
    }

    print("\n");

    shell_begin_prompt();
}

static void shell_handle_char(char value) {
    if (value == '\r' || value == '\n') {
        shell_submit_current_line();
        return;
    }

    if (value == '\b') {
        shell_backspace();
        return;
    }

    if (value < ' ' || value > '~') {
        return;
    }

    shell_append_char(value);
}

void shell_init(const char* username) {
    // shell_copy_username(username);
    shell_begin_prompt();
}

// void shell_set_user(const char* username) {
//     shell_copy_username(username);
// }

void shell_process_input(void) {
    while (keyboard_has_char()) {
        shell_handle_char(keyboard_pop_char());
    }
}