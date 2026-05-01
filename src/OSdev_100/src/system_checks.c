#include "../include/system_checks.h"

#include "../include/keyboard.h"
#include "../include/memory.h"
#include "../include/monitor.h"
#include "../include/pit.h"
#include "../include/isr.h"
#include "../include/libc/stdio.h"

static volatile uint16_t* const system_checks_vga = (volatile uint16_t*)0xB8000;

static void system_checks_write_fixed_text(int row, int column, const char* text, char color) {
    int i = 0;

    while (text[i] != '\0' && (column + i) < 80) {
        system_checks_vga[row * 80 + column + i] =
            (uint16_t)text[i] | ((uint16_t)(uint8_t)color << 8);
        i++;
    }
}

static void system_checks_write_text(int row, int column, const char* text, char color) {
    int i = 0;
    uint8_t old_color;

    monitor_set_cursor((size_t)row, (size_t)column);
    old_color = monitor_get_color();
    monitor_set_color((uint8_t)color);
    while (text[i] != '\0' && (column + i) < 80) {
        monitor_put(text[i]);
        i++;
    }
    monitor_set_color(old_color);
}

static void system_checks_draw_escape_hint(const char* text) {
    int i;
    int column = 76;
    int length = 0;

    while (text[length] != '\0') {
        length++;
    }
    column -= length;
    if (column < 0) {
        column = 0;
    }

    for (i = 47; i < 80; i++) {
        system_checks_vga[4 * 80 + i] =
            (uint16_t)' ' | ((uint16_t)(uint8_t)COLOR_BLACK << 8);
    }
    system_checks_write_fixed_text(4, column, text, COLOR_DARK_GRAY);
}

static void system_checks_draw_basic_hint(void) {
    system_checks_draw_escape_hint("ESC to return");
}

static void system_checks_set_body_color(void) {
    monitor_set_color(COLOR_LIGHT_GRAY);
}

static void system_checks_set_header_color(void) {
    monitor_set_color(COLOR_WHITE);
}

static void system_checks_wait_for_escape_with_hint(void (*draw_hint)(void)) {
    while (1) {
        if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
            monitor_set_overlay(0);
            monitor_set_left_margin(0);
            keyboard_set_mode(KEYBOARD_MODE_COMMAND);
            return;
        }
        sleep_interrupt(60);
    }
}

static void show_system_checks_menu(void) {
    monitor_initialize();
    monitor_hide_cursor();
    keyboard_set_mode(KEYBOARD_MODE_COMMAND);
    monitor_set_overlay(0);

    system_checks_write_text(4, 8, "System Checks", COLOR_YELLOW);
    system_checks_write_text(5, 8, "-------------", COLOR_DARK_GRAY);
    system_checks_write_text(8, 8, "[1] Memory Layout", COLOR_LIGHT_GRAY);
    system_checks_write_text(10, 8, "[2] Allocation Test", COLOR_LIGHT_GRAY);
    system_checks_write_text(12, 8, "[3] Interrupt Checks", COLOR_LIGHT_GRAY);
    system_checks_write_text(14, 8, "[4] PIT Sleep Demo", COLOR_LIGHT_GRAY);
    system_checks_write_text(16, 8, "[5] Run Full Demo", COLOR_LIGHT_GRAY);
    system_checks_write_text(19, 8, "[ESC] Return to menu", COLOR_DARK_GRAY);
}

static void run_memory_layout_check(void) {
    monitor_initialize();
    monitor_hide_cursor();
    keyboard_set_mode(KEYBOARD_MODE_COMMAND);
    monitor_set_left_margin(8);
    monitor_set_overlay(system_checks_draw_basic_hint);

    system_checks_write_text(4, 8, "Memory Layout", COLOR_YELLOW);
    system_checks_write_text(5, 8, "-------------", COLOR_DARK_GRAY);

    monitor_set_cursor(7, 8);
    system_checks_set_body_color();
    print_memory_layout();
    monitor_set_color(COLOR_WHITE);

    system_checks_wait_for_escape_with_hint(system_checks_draw_basic_hint);
}

static void run_interrupt_checks(void) {
    registers_t snapshot;

    monitor_initialize();
    monitor_hide_cursor();
    keyboard_set_mode(KEYBOARD_MODE_COMMAND);
    monitor_set_left_margin(8);
    monitor_set_overlay(system_checks_draw_basic_hint);
    interrupt_demo_enable();

    system_checks_write_text(4, 8, "Interrupt Checks", COLOR_YELLOW);
    system_checks_write_text(5, 8, "----------------", COLOR_DARK_GRAY);

    monitor_set_cursor(7, 8);
    system_checks_set_header_color();
    printf("What this proves:\n");
    system_checks_set_body_color();
    printf("GDT works because the kernel continues after gdt_init().\n");
    printf("IDT/ISR work if vectors reach exception_handler().\n");
    printf("IRQ0 works because PIT timing and sleep respond.\n");
    printf("IRQ1 works because keyboard input reaches the OS.\n");

    system_checks_set_header_color();
    printf("\nActions:\n");
    system_checks_set_body_color();
    printf("[1] Trigger software int 0\n");
    printf("[2] Trigger software int 1\n");
    printf("[3] Trigger software int 2\n");
    printf("Any other key logs a keyboard IRQ1 event.\n");
    printf("Software tests return here instead of halting the kernel.\n");

    system_checks_set_header_color();
    printf("\nExpected software result:\n");
    system_checks_set_body_color();
    printf("Caught through IDT -> ISR stub -> exception_handler().\n");
    printf("The page will log vector, error code, EIP, CS and EFLAGS.\n");
    system_checks_set_header_color();
    printf("\nLive Log:\n");
    monitor_set_color(COLOR_WHITE);

    while (1) {
        int ch;

        if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
            interrupt_demo_disable();
            monitor_set_overlay(0);
            monitor_set_left_margin(0);
            keyboard_set_mode(KEYBOARD_MODE_COMMAND);
            return;
        }

        ch = keyboard_take_char();
        if (ch == -1) {
            sleep_interrupt(60);
            continue;
        }

        system_checks_set_body_color();
        printf("IRQ1 keyboard interrupt received for key '%c'.\n", (char)ch);

        if (ch == '1') {
            __asm__ volatile ("int $0x0");
        } else if (ch == '2') {
            __asm__ volatile ("int $0x1");
        } else if (ch == '3') {
            __asm__ volatile ("int $0x2");
        } else {
            continue;
        }

        if (interrupt_demo_take_snapshot(&snapshot)) {
            system_checks_set_header_color();
            printf("Software interrupt caught:\n");
            system_checks_set_body_color();
            printf("Vector %d reached exception_handler() through IDT/ISR.\n",
                   (int)snapshot.int_no);
            printf("Error code: %d\n", (int)snapshot.err_code);
            printf("EIP: %x  CS: %x  EFLAGS: %x\n",
                   snapshot.eip, snapshot.cs, snapshot.eflags);
            monitor_set_color(COLOR_WHITE);
        }
    }
}

static void run_allocation_test(void) {
    static int allocations_done = 0;
    static void* some_memory = 0;
    static void* memory2 = 0;
    static void* memory3 = 0;

    monitor_initialize();
    monitor_hide_cursor();
    keyboard_set_mode(KEYBOARD_MODE_COMMAND);
    monitor_set_left_margin(8);
    monitor_set_overlay(system_checks_draw_basic_hint);

    system_checks_write_text(4, 8, "Allocation Test", COLOR_YELLOW);
    system_checks_write_text(5, 8, "---------------", COLOR_DARK_GRAY);

    monitor_set_cursor(7, 8);
    system_checks_set_header_color();
    printf("Memory Layout Before malloc:\n");
    system_checks_set_body_color();
    print_memory_layout();

    if (!allocations_done) {
        some_memory = malloc(12345);
        memory2 = malloc(54321);
        memory3 = malloc(13331);
        allocations_done = 1;
    }

    system_checks_set_header_color();
    printf("\nAllocation Results:\n");
    system_checks_set_body_color();
    printf("malloc(12345) -> %x\n", (uint32_t)some_memory);
    printf("malloc(54321) -> %x\n", (uint32_t)memory2);
    printf("malloc(13331) -> %x\n", (uint32_t)memory3);

    system_checks_set_header_color();
    printf("\nMemory Layout After malloc:\n");
    system_checks_set_body_color();
    print_memory_layout();
    monitor_set_color(COLOR_WHITE);

    system_checks_wait_for_escape_with_hint(system_checks_draw_basic_hint);
}

static void run_pit_sleep_demo(void) {
    int counter = 0;

    monitor_initialize();
    monitor_hide_cursor();
    keyboard_set_mode(KEYBOARD_MODE_COMMAND);
    monitor_set_left_margin(8);
    monitor_set_overlay(system_checks_draw_basic_hint);

    system_checks_write_text(4, 8, "PIT Sleep Demo", COLOR_YELLOW);
    system_checks_write_text(5, 8, "--------------", COLOR_DARK_GRAY);

    monitor_set_cursor(7, 8);
    system_checks_set_header_color();
    printf("[PIT Timer]\n");
    system_checks_set_body_color();
    printf("PIT initialized: OK\n");
    printf("busy_wait = high CPU, interrupt_sleep = low CPU\n\n");

    while (1) {
        if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
            monitor_set_overlay(0);
            monitor_set_left_margin(0);
            return;
        }

        printf("[%d]: Sleeping with busy-waiting (HIGH CPU).\n", counter);
        sleep_busy(1000);
        if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
            monitor_set_overlay(0);
            monitor_set_left_margin(0);
            return;
        }
        printf("[%d]: Slept using busy-waiting.\n", counter++);

        printf("[%d]: Sleeping with interrupts (LOW CPU).\n", counter);
        sleep_interrupt(1000);
        if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
            monitor_set_overlay(0);
            monitor_set_left_margin(0);
            return;
        }
        printf("[%d]: Slept using interrupts.\n", counter++);
    }
}

static void run_full_system_demo(void) {
    static int demo_initialized = 0;
    static void* some_memory = 0;
    static void* memory2 = 0;
    static void* memory3 = 0;
    int counter = 0;

    monitor_initialize();
    monitor_hide_cursor();
    keyboard_set_mode(KEYBOARD_MODE_COMMAND);
    monitor_set_left_margin(8);
    monitor_set_overlay(system_checks_draw_basic_hint);

    system_checks_write_text(4, 8, "System Checks Demo", COLOR_YELLOW);
    system_checks_write_text(5, 8, "------------------", COLOR_DARK_GRAY);
    system_checks_write_text(7, 8, "Kernel output test: OK", COLOR_WHITE);

    monitor_set_cursor(9, 8);
    system_checks_set_header_color();
    printf("[Memory Manager]\n");
    system_checks_set_body_color();
    printf("Kernel memory initialized: OK\n");
    printf("Paging initialized: OK\n\n");

    system_checks_set_header_color();
    printf("Memory Layout Before malloc:\n");
    system_checks_set_body_color();
    print_memory_layout();

    if (!demo_initialized) {
        some_memory = malloc(12345);
        memory2 = malloc(54321);
        memory3 = malloc(13331);
        demo_initialized = 1;
    }

    system_checks_set_header_color();
    printf("\nAllocation Results:\n");
    system_checks_set_body_color();
    printf("malloc(12345) -> %x\n", (uint32_t)some_memory);
    printf("malloc(54321) -> %x\n", (uint32_t)memory2);
    printf("malloc(13331) -> %x\n", (uint32_t)memory3);

    system_checks_set_header_color();
    printf("\nMemory Layout After malloc:\n");
    system_checks_set_body_color();
    print_memory_layout();

    system_checks_set_header_color();
    printf("\n[PIT Timer]\n");
    system_checks_set_body_color();
    printf("PIT initialized: OK\n");
    printf("Starting sleep demo...\n");
    printf("busy_wait = high CPU, interrupt_sleep = low CPU\n\n");

    while (1) {
        if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
            monitor_set_overlay(0);
            monitor_set_left_margin(0);
            return;
        }

        printf("[%d]: Sleeping with busy-waiting (HIGH CPU).\n", counter);
        sleep_busy(1000);
        if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
            monitor_set_overlay(0);
            monitor_set_left_margin(0);
            return;
        }
        printf("[%d]: Slept using busy-waiting.\n", counter++);

        printf("[%d]: Sleeping with interrupts (LOW CPU).\n", counter);
        sleep_interrupt(1000);
        if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
            monitor_set_overlay(0);
            monitor_set_left_margin(0);
            return;
        }
        printf("[%d]: Slept using interrupts.\n", counter++);
    }
}

void run_system_checks(void) {
    while (1) {
        int ch;

        show_system_checks_menu();

        while (1) {
            if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
                monitor_set_overlay(0);
                return;
            }

            ch = keyboard_take_char();
            if (ch == -1) {
                sleep_interrupt(60);
                continue;
            }

            if (ch == '1') {
                run_memory_layout_check();
                break;
            } else if (ch == '2') {
                run_allocation_test();
                break;
            } else if (ch == '3') {
                run_interrupt_checks();
                break;
            } else if (ch == '4') {
                run_pit_sleep_demo();
                break;
            } else if (ch == '5') {
                run_full_system_demo();
                break;
            }
        }
    }
}
