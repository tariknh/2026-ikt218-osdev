#include <libc/stddef.h>
#include <libc/stdint.h>

#include "terminal.h"
#include "shell.h"
#include "snake.h"
#include "song/song.h"
#include "kernel_memory.h"

#define SHELL_INPUT_MAX 128

static char input_buffer[SHELL_INPUT_MAX];
static size_t input_length = 0;
static char pending_command[SHELL_INPUT_MAX];
static volatile int pending_command_ready = 0;

static int streq(const char* a, const char* b) {
    size_t i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) {
            return 0;
        }
        i++;
    }
    return a[i] == '\0' && b[i] == '\0';
}

static void shell_prompt(void) {
    terminal_write("> ");
}

static void shell_queue_command(void) {
    size_t i = 0;
    while (i < SHELL_INPUT_MAX - 1 && input_buffer[i] != '\0') {
        pending_command[i] = input_buffer[i];
        i++;
    }
    pending_command[i] = '\0';
    pending_command_ready = 1;
}

static void shell_play_march(void) {
    Song song = get_demo_song();
    SongPlayer* player = create_song_player();

    if (player == NULL) {
        terminal_write("Failed to create song player.\n");
        return;
    }

    player->play_song(&song);
    free(player);
}

static void shell_execute_command(const char* cmd) {
    // Ignore empty input
    if (cmd[0] == '\0') {
        return;
    }

    // Placeholder for future commands
    if (streq(cmd, "help")) {
        terminal_write("Available commands:\n");
        terminal_write("  help - Show this message\n");
        terminal_write("  march - Play the stored imperial march song\n");
        terminal_write("  snake - Play the terminal snake game\n");
    } else if (streq(cmd, "march")) {
        shell_play_march();
    } else if (streq(cmd, "snake")) {
        snake_run();
    } else {
        terminal_write("Unknown command: ");
        terminal_write(cmd);
        terminal_write("\n");
    }
}

void shell_init(void) {
    input_length = 0;
    input_buffer[0] = '\0';
    pending_command_ready = 0;

    terminal_write("Welcome to the OSDev Shell!\n");
    shell_prompt();
}

void shell_handle_char(char c) {
    if (snake_is_running()) {
        snake_handle_input(c);
        return;
    }

    if (c == '\n') {
        terminal_write("\n");
        input_buffer[input_length] = '\0';
        shell_queue_command();
        input_length = 0;
        input_buffer[0] = '\0';
        return;
    }

    if(c== '\b') {
        if (input_length > 0) {
            input_length--;
            input_buffer[input_length] = '\0';
            terminal_write("\b \b");
        }
        return;
    }

    if (c>=32 && c <=126) {
        if (input_length < SHELL_INPUT_MAX - 1) {
            input_buffer[input_length++] = c;
            input_buffer[input_length] = '\0';

            char out[2] = {c, '\0'};
            terminal_write(out);
        }
    }
}

void shell_process_pending(void) {
    if (!pending_command_ready) {
        return;
    }

    pending_command_ready = 0;
    shell_execute_command(pending_command);
    shell_prompt();
}