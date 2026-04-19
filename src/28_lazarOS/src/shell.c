#include <shell.h>
#include <input.h>
#include <pit.h>
#include <pic.h>
#include <memory.h>
#include <libc/stdint.h>
#include <libc/stdio.h>
#include <song.h>

/* ── VGA helpers (defined in kernel.c) ── */
extern void terminal_clear(void);

/* ── Memory globals (defined in memory.c) ── */
extern uint32_t heap_begin;
extern uint32_t heap_end;
extern uint32_t memory_used;
extern uint32_t pheap_begin;
extern uint32_t pheap_end;

/* ── Simple string helpers (no libc available) ── */
static int str_equal(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static int str_starts_with(const char *str, const char *prefix)
{
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++; prefix++;
    }
    return 1;
}

static int str_len(const char *s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

/* ── Shutdown via ACPI / QEMU ── */
static void shutdown(void)
{
    printf("\nShutting down");
    for (int i = 3; i > 0; i--) {
        printf(" %d...", i);
        sleep_interrupt(1000);
    }
    printf("\nGoodbye!\n");
    sleep_interrupt(500);

    /* QEMU shutdown port */
    outb(0xF4, 0x00);
    /* Bochs / older QEMU */
    __asm__ volatile("outw %%ax, %%dx" : : "a"(0x2000), "d"(0x604));
    /* If nothing worked, halt */
    __asm__ volatile("cli");
    while (1) __asm__ volatile("hlt");
}

/* ── Beep using PC speaker ── */
static void cmd_beep(void)
{
    uint16_t div = (uint16_t)(1193180 / 1000);  /* 1 kHz tone */
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(div & 0xFF));
    outb(0x42, (uint8_t)((div >> 8) & 0xFF));

    uint8_t tmp = inb(0x61);
    if ((tmp & 3) != 3)
        outb(0x61, tmp | 3);

    sleep_interrupt(200);

    outb(0x61, inb(0x61) & (uint8_t)~3);
    printf("Beep!\n");
}

/* ── System information display ── */
static void cmd_sysinfo(void)
{
    uint32_t ticks = get_current_tick();
    uint32_t seconds = ticks / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;

    printf("========== lazarOS System Info ==========\n");
    printf("  Uptime:       %d h  %d m  %d s\n",
           hours, minutes % 60, seconds % 60);
    printf("  PIT ticks:    %u\n", ticks);
    printf("  PIT freq:     %d Hz\n", TARGET_FREQUENCY);
    printf("\n");
    printf("  Heap start:   0x%x\n", heap_begin);
    printf("  Heap end:     0x%x\n", heap_end);
    printf("  Heap size:    %d bytes\n", heap_end - heap_begin);
    printf("  Memory used:  %d bytes\n", memory_used);
    printf("  Memory free:  %d bytes\n",
           heap_end - heap_begin - memory_used);
    printf("\n");
    printf("  PHeap start:  0x%x\n", pheap_begin);
    printf("  PHeap end:    0x%x\n", pheap_end);
    printf("\n");
    printf("  VGA buffer:   0xB8000\n");
    printf("  Architecture: i386 (32-bit protected mode)\n");
    printf("  Paging:       enabled (identity-mapped)\n");
    printf("=========================================\n");
}

/* ── Uptime command ── */
static void cmd_uptime(void)
{
    uint32_t ticks = get_current_tick();
    uint32_t seconds = ticks / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;

    printf("Up %d h %d m %d s (%u ticks)\n",
           hours, minutes % 60, seconds % 60, ticks);
}

/* ── Help ── */
static void cmd_help(void)
{
    printf("Available commands:\n");
    printf("  help      - Show this message\n");
    printf("  clear     - Clear the screen\n");
    printf("  sysinfo   - Display system information\n");
    printf("  uptime    - Show time since boot\n");
    printf("  meminfo   - Show memory layout\n");
    printf("  beep      - Play a short beep\n");
    printf("  echo <text> - Print text to screen\n");
    printf("  shutdown  - Power off the system\n");
    printf("  exit      - Return to the main menu\n");
}

/* ── Main shell loop ── */
void shell_run(void)
{
    char line[128];

    terminal_clear();
    printf("lazarOS Shell v1.0\n");
    printf("Type 'help' for a list of commands.\n\n");

    while (1) {
        printf("lazarOS> ");
        getline(line, 127);

        /* Skip empty lines */
        if (line[0] == '\0')
            continue;

        if (str_equal(line, "help")) {
            cmd_help();
        } else if (str_equal(line, "clear")) {
            terminal_clear();
        } else if (str_equal(line, "sysinfo")) {
            cmd_sysinfo();
        } else if (str_equal(line, "uptime")) {
            cmd_uptime();
        } else if (str_equal(line, "meminfo")) {
            print_memory_layout();
        } else if (str_equal(line, "beep")) {
            cmd_beep();
        } else if (str_starts_with(line, "echo ")) {
            printf("%s\n", line + 5);
        } else if (str_equal(line, "shutdown")) {
            shutdown();
        } else if (str_equal(line, "exit")) {
            printf("Returning to menu...\n");
            sleep_interrupt(500);
            return;
        } else {
            printf("Unknown command: '%s'. Type 'help' for commands.\n", line);
        }
    }
}
