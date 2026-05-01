/*
Name: kernel.c
Project: LocOS
Description: Main kernel entry file. It starts the system and runs simple tests.
             This is a school project about operating system development.
*/

#include "gdt.h"
#include "idt.h"
#include "memory.h"
#include "paging.h"
#include "pit.h"
#include "terminal.h"
#include "irq.h"
#include "cxx_mem.h"
#include <libc/stdint.h>
#include <libc/stdbool.h>
#include <libc/stddef.h>

extern uint32_t end;

static bool test_hello_write(void) {
    terminal_write("LocOS v0.3\n");
    return true;
}

static bool test_hello_printf(void) {
    terminal_printf("LocOS v0.3\n");
    return true;
}

static bool test_interrupts(void) {
    terminal_printf("Testing software interrupts (0,1,2,0x30)...\n");
    __asm__ volatile ("int $0x0");
    __asm__ volatile ("int $0x1");
    __asm__ volatile ("int $0x2");
    __asm__ volatile ("int $0x30");
    terminal_printf("Software interrupts triggered.\n");
    return true;
}

static bool test_irq_vector(void) {
    terminal_printf("Triggering IRQ vector (int 0x20)...\n");
    __asm__ volatile ("int $0x20");
    terminal_printf("IRQ vector trigger done.\n");
    return true;
}

static bool test_pit(void) {
    terminal_printf("PIT test: busy sleep 500ms...\n");
    sleep_busy(500);
    terminal_printf("PIT test: busy sleep done\n");
    terminal_printf("PIT test: interrupt sleep 500ms...\n");
    sleep_interrupt(500);
    terminal_printf("PIT test: interrupt sleep done\n");
    return true;
}

static bool test_keyboard(void) {
    terminal_printf("Keyboard test: type keys, ESC to exit\n");
    kbd_set_echo(true);
    for (;;) {
        int c = kbd_getchar();
        if (c == 27) { /* ESC */
            terminal_printf("Exit keyboard test\n");
            kbd_set_echo(false);
            return true;
        }
        if (c == '\r' || c == '\n') {
            terminal_printf("\n");
            continue;
        }
        terminal_printf("Key: '%c' (0x%x)\n", (char)c, (uint32_t)(uint8_t)c);
    }
}

static bool test_keyboard_free(void) {
    terminal_printf("Keyboard free input: ESC to exit\n");
    kbd_set_echo(false);
    for (;;) {
        int c = kbd_getchar();
        if (c == 27) { /* ESC */
            terminal_printf("\nExit keyboard free input\n");
            return true;
        }
        if (c == '\r') c = '\n';
        terminal_printf("%c", (char)c);
    }
}

static bool test_cpp_new_delete(void) {
    terminal_printf("C++ new/delete test...\n");
    return cpp_new_delete_test();
}

static bool test_music_demo(void) {
    terminal_printf("PC speaker demo...\n");
    play_demo_song();
    terminal_printf("Music demo done\n");
    return true;
}

static bool test_paging(void) {
    terminal_printf("Paging self-test...\n");
    return paging_self_test();
}

struct menu_item {
    const char *label;
    bool (*fn)(void);
};

static struct menu_item menu[] = {
    { "1) Hello World (write)",  test_hello_write },
    { "2) Hello World (printf)", test_hello_printf },
    { "3) Interrupt test",       test_interrupts },
    { "4) IRQ test",             test_irq_vector },
    { "5) PIT test",             test_pit },
    { "6) Keyboard input (log)", test_keyboard },
    { "7) Keyboard free input",  test_keyboard_free },
    { "8) C++ new/delete test",  test_cpp_new_delete },
    { "9) Paging self-test",     test_paging },
    { "0) Music demo (PCSPK)",   test_music_demo },
};

static const size_t menu_len = sizeof(menu) / sizeof(menu[0]);

static void show_menu(void) {
    terminal_home();
    terminal_printf("LocOS v0.3\n");
    print_memory_layout();
    terminal_printf("\n=== Test Menu ===\n");
    for (size_t i = 0; i < menu_len; i++) {
        terminal_printf("%s\n", menu[i].label);
    }
    terminal_printf("Select: ");
}

static void handle_choice(char key) {
    size_t idx = (size_t)-1;
    
    if (key >= '1' && key <= '9') {
        idx = (size_t)(key - '1');
    } else if (key == '0') {
        idx = 9;  /* '0' maps to index 9 (the 10th item) */
    }
    
    if (idx < menu_len) {
        bool ok = menu[idx].fn();
        terminal_printf("[%s] %s\n", menu[idx].label, ok ? "PASS" : "FAIL");
    } else {
        terminal_printf("Unknown choice: %c\n", key);
    }
}

void kmain(void) {
    gdt_init();
    terminal_init();
    idt_init();
    irq_init();
    kbd_set_echo(false);
    init_pit();

    terminal_printf("LocOS v0.3\n");

    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();

    __asm__ volatile ("sti");

    show_menu();
    for (;;) {
        int c = kbd_getchar();
        if (c < 0) continue;

        if (c == 27) {
            terminal_printf("\n(Return to menu)\n");
            sleep_busy(200);
            terminal_clear();
            terminal_home();
            show_menu();
            continue;
        }

        kbd_set_echo(false); /* ensure clean state before running a test */
        handle_choice((char)c);
        terminal_printf("\n(Press ESC to return to menu)\n");
        /* Wait for ESC explicitly */
        while ((c = kbd_getchar()) != 27) {
            if (c < 0) continue;
        }
        sleep_busy(200);          /* short pause */
        terminal_clear();
        terminal_home();
        show_menu();
    }

    for (;;) { __asm__("hlt"); }
}
