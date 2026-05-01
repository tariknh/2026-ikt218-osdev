#include "kernel/cli.h"

#include "apps/raycaster/raycaster.h"
#include "apps/song/song.h"
#include "apps/song/song_data.h"
#include "arch/i386/io.h"
#include "boot/splash.h"
#include "memory/heap.h"
#include "kernel/pit.h"
#include "interrupts/isr.h"
#include "libc/stdio.h"

#define HISTORY_CAPACITY 32

static char *history_entries[HISTORY_CAPACITY];
static int history_count = 0;
static int history_start = 0;

// Local strlen to avoid pulling in extra libc helpers here
static size_t cli_strlen(const char *text)
{
    size_t length = 0;

    while (text[length] != '\0') {
        length++;
    }

    return length;
}

// Checks if two strings are exactly equal
static int cli_streq(const char *left, const char *right)
{
    size_t index = 0;

    while (left[index] != '\0' && right[index] != '\0') {
        if (left[index] != right[index]) {
            return 0;
        }

        index++;
    }

    return left[index] == right[index];
}

// Checks if command begins with a given prefix
static int cli_startswith(const char *text, const char *prefix)
{
    size_t index = 0;

    while (prefix[index] != '\0') {
        if (text[index] != prefix[index]) {
            return 0;
        }

        index++;
    }

    return 1;
}

// Skips spaces before parsing command arguments
static const char *skip_spaces(const char *text)
{
    while (*text == ' ') {
        text++;
    }

    return text;
}

// Frees all saved command history entries
static void clear_history(void)
{
    int i;

    for (i = 0; i < history_count; i++) {
        int slot = (history_start + i) % HISTORY_CAPACITY;

        free(history_entries[slot]);
        history_entries[slot] = 0;
    }

    history_count = 0;
    history_start = 0;
}

// Prints command history in oldest to newest order
static void print_history(void)
{
    int i;

    if (history_count == 0) {
        printf("History is empty.\n");
        return;
    }

    printf("Saved entries:\n");
    for (i = 0; i < history_count; i++) {
        int slot = (history_start + i) % HISTORY_CAPACITY;

        printf("%d: %s\n", i + 1, history_entries[slot]);
    }
}

// Displays available commands and keyboard shortcuts
static void print_help(void)
{
    printf("Commands:\n");
    printf("help         Show this help screen\n");
    printf("clear        Clear the display\n");
    printf("meminfo      Show heap and page memory info\n");
    printf("history      Show saved command history\n");
    printf("clearhistory Free saved history entries\n");
    printf("ticks        Show current PIT tick count\n");
    printf("uptime       Show uptime in milliseconds\n");
    printf("music <idx>  Play the song with index (0-%d)\n", available_song_count - 1);
    printf("game         Play the 3D raycaster game\n");
    printf("interrupt    Run CPU exceptions (0x00-0x1F)\n");
    printf("echo <text>  Print text back to the screen\n");
    printf("about        Show kernel feature summary\n");
    printf("Keyboard:\n");
    printf("ESC          Stop playing music or exit game\n");
    printf("M            Show or hide minimap in game\n");
    printf("PgUp/PgDn    Scroll terminal history by pages\n");
    printf("Up/Down      Scroll terminal history line by line\n");
    printf("Home/End     Jump to top or bottom of scrollback\n");
}

// Prints a short summary of current kernel features
static void print_about(void)
{
    terminal_print_logo();
    printf("Interrupts, paging, heap, PIT, keyboard history, scrollback\n");
    printf("History entries are stored on the heap.\n");
}

// Triggers a software exception without causing a real CPU fault
static void show_exception_without_fault(uint32_t interrupt_number)
{
    registers_t regs = {0};
    regs.int_no = interrupt_number;
    isr_handler(&regs);
}

// Runs through CPU exceptions to test ISR output
static void run_interrupt_demo(void)
{
    terminal_initialize();
    printf("Interrupt test page\n");
    printf("Running CPU exceptions 0x00-0x1F...\n\n");

    asm volatile("int $0x00");
    asm volatile("int $0x01");
    asm volatile("int $0x02");
    asm volatile("int $0x03");
    asm volatile("int $0x04");
    asm volatile("int $0x05");
    asm volatile("int $0x06");
    asm volatile("int $0x07");
    show_exception_without_fault(8);
    asm volatile("int $0x09");
    show_exception_without_fault(10);
    show_exception_without_fault(11);
    show_exception_without_fault(12);
    show_exception_without_fault(13);
    show_exception_without_fault(14);
    asm volatile("int $0x0F");
    asm volatile("int $0x10");
    asm volatile("int $0x11");
    asm volatile("int $0x12");
    asm volatile("int $0x13");
    asm volatile("int $0x14");
    asm volatile("int $0x15");
    asm volatile("int $0x16");
    asm volatile("int $0x17");
    asm volatile("int $0x18");
    asm volatile("int $0x19");
    asm volatile("int $0x1A");
    asm volatile("int $0x1B");
    asm volatile("int $0x1C");
    asm volatile("int $0x1D");
    asm volatile("int $0x1E");
    asm volatile("int $0x1F");

    printf("\nDone triggering CPU exceptions 0x00-0x1F.\n");
}

// Stores a command line in fixed-size circular history
static void save_history_entry(const char *line)
{
    size_t length = cli_strlen(line);
    char *entry;
    int slot;
    size_t i;

    if (length == 0) {
        return;
    }

    entry = (char *)malloc(length + 1);
    if (entry == 0) {
        printf("History allocation failed.\n");
        return;
    }

    for (i = 0; i <= length; i++) {
        entry[i] = line[i];
    }

    if (history_count == HISTORY_CAPACITY) {
        slot = history_start;
        free(history_entries[slot]);
        history_start = (history_start + 1) % HISTORY_CAPACITY;
    } else {
        slot = (history_start + history_count) % HISTORY_CAPACITY;
        history_count++;
    }

    history_entries[slot] = entry;
}

// Main command dispatcher for the terminal
static void execute_command(const char *command)
{
    const char *argument;

    if (cli_streq(command, "help")) {
        print_help();
        return;
    }

    if (cli_streq(command, "clear")) {
        terminal_initialize();
        return;
    }

    if (cli_streq(command, "meminfo")) {
        print_memory_layout();
        printf("History entries: %d of %d\n", history_count, HISTORY_CAPACITY);
        printf("Ticks: %d\n", (int)pit_get_ticks());
        return;
    }

    if (cli_streq(command, "history")) {
        print_history();
        return;
    }

    if (cli_streq(command, "clearhistory")) {
        clear_history();
        printf("History cleared.\n");
        return;
    }

    if (cli_streq(command, "ticks")) {
        printf("Ticks: %d\n", (int)pit_get_ticks());
        return;
    }

    if (cli_streq(command, "uptime")) {
        printf("Uptime: %d ms\n", (int)pit_get_ticks());
        return;
    }

    if (cli_streq(command, "about")) {
        print_about();
        return;
    }

    if (cli_startswith(command, "music")) {
        const char *arg = skip_spaces(command + 5);
        int song_index = 0;

        if (*arg == '\0') {
            printf("Usage: music <song_index>\nAvailable songs: 0-%d\n", available_song_count - 1);
            return;
        }

        while (*arg >= '0' && *arg <= '9') {
            song_index = song_index * 10 + (*arg - '0');
            arg++;
        }

        if (*arg != '\0') {
            printf("Invalid song index: %s\n", skip_spaces(command + 5));
            return;
        }

        play_music(song_index);
        return;
    }

    if (cli_startswith(command, "echo")) {
        argument = skip_spaces(command + 4);
        printf("%s\n", argument);
        return;
    }

    if (cli_streq(command, "game")) {
        raycaster_input_request_launch();
        printf("Starting game...\n");
        return;
    }

    if (cli_streq(command, "interrupt")) {
        run_interrupt_demo();
        return;
    }

    printf("Unknown command: %s\n", command);
    printf("Type help to see available commands.\n");
}

void cli_print_prompt(void)
{
    printf("> ");
}

void cli_submit_line(const char *line)
{
    printf("\n");

    if (line[0] == '\0') {
        cli_print_prompt();
        return;
    }

    // Save then execute so user can review previous commands
    save_history_entry(line);
    execute_command(line);
    cli_print_prompt();
}
