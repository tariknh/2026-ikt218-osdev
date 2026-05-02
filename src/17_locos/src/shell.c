#include "shell.h"
#include "irq.h"
#include "memory.h"
#include "pcspk.h"
#include "piano.h"
#include "pit.h"
#include "terminal.h"
#include "wire3d.h"
#include <libc/stdint.h>
#include <libc/stddef.h>

#define SHELL_LINE_MAX 128

// Write one byte to an I/O port
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Compare two strings exactly
static int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

// Check if one string starts with another string
static int str_starts(const char *s, const char *prefix) {
    while (*prefix) {
        if (*s++ != *prefix++) return 0;
    }
    return 1;
}

// Skip spaces at the front of a line
static char *skip_spaces(char *s) {
    while (*s == ' ') s++;
    return s;
}

// Read a positive number from text
static uint32_t parse_u32(const char *s, int *ok) {
    uint32_t v = 0;
    *ok = 0;
    if (!s || *s == '\0') return 0;
    while (*s == ' ') s++;
    if (*s < '0' || *s > '9') return 0;

    while (*s >= '0' && *s <= '9') {
        v = v * 10u + (uint32_t)(*s - '0');
        s++;
    }
    *ok = 1;
    return v;
}

// Print the shell help text
static void shell_help(void) {
    terminal_printf("Commands:\n");
    terminal_printf("  help              Show this help\n");
    terminal_printf("  clear             Clear the screen\n");
    terminal_printf("  mem               Print memory layout\n");
    terminal_printf("  ticks             Print PIT tick counter\n");
    terminal_printf("  sleep <ms>        Sleep using interrupts\n");
    terminal_printf("  beep <hz> <ms>    Play tone\n");
    terminal_printf("  piano             Interactive piano (1..8)\n");
    terminal_printf("  music             Play demo melody\n");
    terminal_printf("  cube              Run ASCII 3D cube demo\n");
    terminal_printf("  reboot            Reboot machine\n");
    terminal_printf("  exit              Return to menu\n");
}

// Run one shell command line
static int shell_exec(char *line) {
    // Remove leading spaces first
    char *cmd = skip_spaces(line);
    if (*cmd == '\0') return 1;

    // Built in help command
    if (str_eq(cmd, "help")) {
        shell_help();
        return 1;
    }
    // Clear the screen and go back to the top
    if (str_eq(cmd, "clear")) {
        terminal_clear();
        terminal_home();
        return 1;
    }
    // Print the current heap layout
    if (str_eq(cmd, "mem")) {
        print_memory_layout();
        return 1;
    }
    // Show PIT tick count
    if (str_eq(cmd, "ticks")) {
        terminal_printf("ticks=%u\n", pit_get_ticks());
        return 1;
    }
    // Play the demo song
    if (str_eq(cmd, "music")) {
        play_demo_song();
        return 1;
    }
    // Open the piano app
    if (str_eq(cmd, "piano")) {
        piano_run();
        return 1;
    }
    // Open the cube demo
    if (str_eq(cmd, "cube")) {
        wire3d_run_demo();
        return 1;
    }
    // Reboot the machine through the keyboard controller
    if (str_eq(cmd, "reboot")) {
        terminal_printf("Rebooting...\n");
        outb(0x64, 0xFE);
        for (;;) { __asm__ volatile ("hlt"); }
    }
    // Leave the shell and go back to the menu
    if (str_eq(cmd, "exit")) {
        return 0;
    }

    // Sleep for a number of milliseconds
    if (str_starts(cmd, "sleep ")) {
        int ok = 0;
        uint32_t ms = parse_u32(cmd + 6, &ok);
        if (!ok) {
            terminal_printf("Usage: sleep <ms>\n");
            return 1;
        }
        sleep_interrupt(ms);
        return 1;
    }

    // Play one tone for a chosen time
    if (str_starts(cmd, "beep ")) {
        char *args = cmd + 5;
        int ok_hz = 0, ok_ms = 0;
        uint32_t hz = parse_u32(args, &ok_hz);

        while (*args && *args != ' ') args++;
        args = skip_spaces(args);
        uint32_t ms = parse_u32(args, &ok_ms);

        if (!ok_hz || !ok_ms || hz == 0) {
            terminal_printf("Usage: beep <hz> <ms>\n");
            return 1;
        }

        play_sound(hz);
        sleep_interrupt(ms);
        stop_sound();
        disable_speaker();
        return 1;
    }

    // Unknown command path
    terminal_printf("Unknown command: %s\n", cmd);
    terminal_printf("Type 'help'\n");
    return 1;
}

// Main shell loop
bool shell_run(void) {
    char line[SHELL_LINE_MAX];
    size_t len = 0;

    // Turn off echo so the shell can print input itself
    kbd_set_echo(false);
    terminal_printf("LocOS shell started. Type 'help'.\n");

    for (;;) {
        // Print the prompt for one command
        terminal_printf("LocOS> ");
        len = 0;

        for (;;) {
            // Wait for one key
            int c = kbd_getchar();
            if (c < 0) continue;

            // ENTER finishes the current line
            if (c == '\r' || c == '\n') {
                terminal_printf("\n");
                line[len] = '\0';
                break;
            }

            // ESC leaves the shell and returns to the menu
            if (c == 27) { /* ESC exits shell */
                terminal_printf("\nExit shell\n");
                return true;
            }

            // Backspace removes one character from the line
            if (c == '\b' || c == 127) {
                if (len > 0) {
                    len--;
                    terminal_printf("\b");
                }
                continue;
            }

            // Store normal printable input in the line buffer
            if (c >= 32 && c <= 255 && c != 127) {
                if (len + 1 < SHELL_LINE_MAX) {
                    line[len++] = (char)c;
                    terminal_printf("%c", (char)c);
                }
            }
        }

        // Run the command line and stop on exit
        if (!shell_exec(line)) {
            return true;
        }
    }
}