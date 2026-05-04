/*
 * shell.c - Tiny line-buffered shell
 *
 * The keyboard handler calls shell_on_char(c) for every key the user
 * presses.  We echo printable characters to the screen, treat Backspace
 * as line editing, and dispatch the line on Enter.
 */

#include <shell.h>
#include <terminal.h>
#include <rtc.h>
#include <pit.h>
#include <memory.h>
#include <song.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/stdint.h>
#include <libc/stddef.h>

#define SHELL_BUF_SIZE  128

static char   line_buffer[SHELL_BUF_SIZE];
static size_t line_pos = 0;

static void prompt(void) {
    printf("> ");
}

/* Print a non-negative integer with a fixed width, padded with leading
   zeros.  Width must be small (<= 10).  Avoids the lack of %0Nd in our
   minimal printf. */
static void print_padded(uint32_t value, int width) {
    char buf[11];
    int  i = 0;
    if (value == 0) {
        buf[i++] = '0';
    } else {
        while (value > 0 && i < (int)sizeof(buf)) {
            buf[i++] = (char)('0' + (value % 10));
            value /= 10;
        }
    }
    while (i < width) {
        buf[i++] = '0';
    }
    /* Buffer is in reverse; print in correct order. */
    while (i > 0) {
        i--;
        terminal_putchar(buf[i]);
    }
}

/* ---------- Commands ---------- */

static void cmd_time(void) {
    rtc_time_t t;
    rtc_read(&t);
    print_padded(t.hour, 2);
    terminal_putchar(':');
    print_padded(t.minute, 2);
    terminal_putchar(':');
    print_padded(t.second, 2);
    terminal_putchar('\n');
}

static void cmd_date(void) {
    rtc_time_t t;
    rtc_read(&t);
    print_padded(t.day, 2);
    terminal_putchar('-');
    print_padded(t.month, 2);
    terminal_putchar('-');
    print_padded(t.year, 4);
    terminal_putchar('\n');
}

static void cmd_uptime(void) {
    uint32_t ticks   = get_current_tick();
    uint32_t seconds = ticks / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours   = minutes / 60;
    seconds %= 60;
    minutes %= 60;
    print_padded(hours,   2); terminal_putchar(':');
    print_padded(minutes, 2); terminal_putchar(':');
    print_padded(seconds, 2);
    printf("  (%d ticks since boot)\n", (int)ticks);
}

static void cmd_mem(void) {
    print_memory_layout();
}

static void cmd_song(void) {
    Song s = { music_1, music_1_length };
    play_song_impl(&s);
}

static void cmd_clear(void) {
    terminal_initialize();
}

static void cmd_help(void) {
    printf("Available commands:\n");
    printf("  help     show this message\n");
    printf("  time     show wall-clock time (HH:MM:SS)\n");
    printf("  date     show wall-clock date (DD-MM-YYYY)\n");
    printf("  uptime   seconds since boot\n");
    printf("  mem      kernel heap layout\n");
    printf("  song     play Twinkle Twinkle on the PC speaker\n");
    printf("  clear    clear the screen\n");
}

/* ---------- Dispatch ---------- */

typedef struct {
    const char* name;
    void (*fn)(void);
} command_t;

static const command_t commands[] = {
    { "help",   cmd_help   },
    { "time",   cmd_time   },
    { "date",   cmd_date   },
    { "uptime", cmd_uptime },
    { "mem",    cmd_mem    },
    { "song",   cmd_song   },
    { "clear",  cmd_clear  },
};

static void shell_execute(const char* line) {
    if (line[0] == '\0') return;   /* empty line, just show new prompt */

    for (size_t i = 0; i < sizeof(commands) / sizeof(command_t); i++) {
        if (strcmp(line, commands[i].name) == 0) {
            commands[i].fn();
            return;
        }
    }
    printf("Unknown command: %s. Type 'help' for a list.\n", line);
}

/* ---------- Public API ---------- */

void shell_init(void) {
    line_pos = 0;
    line_buffer[0] = '\0';
    printf("UiAOS shell. Type 'help' for the list of commands.\n");
    prompt();
}

void shell_on_char(char c) {
    if (c == '\n') {
        terminal_putchar('\n');
        line_buffer[line_pos] = '\0';
        shell_execute(line_buffer);
        line_pos = 0;
        prompt();
    } else if (c == '\b') {
        if (line_pos > 0) {
            line_pos--;
            terminal_putchar('\b');
        }
    } else if (line_pos < SHELL_BUF_SIZE - 1) {
        line_buffer[line_pos++] = c;
        terminal_putchar(c);
    }
}
