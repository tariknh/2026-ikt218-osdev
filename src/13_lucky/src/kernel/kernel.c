#include "stdbool.h"
#include "stdio.h"
#include "arch/i386/gdt.h"
#include "arch/i386/idt.h"
#include "arch/i386/isr.h"
#include "kernel/apps.h"
#include "kernel/memory.h"
#include "kernel/pit.h"

// This symbol comes from linker.ld and it marks where the kernel ends in memory
extern uint32_t end;

extern void test_new_operator(void);

void print_main_menu(void) {
    terminal_clear();
    printf("=== Lucky 13 OS ===\n");
    printf("1. Music Player\n");
    printf("2. Pong\n");
    printf("3. Matrix rain\n");
    printf("4. Notepad\n");
    printf("5. Shutdown\n\n");
    printf("Select an option: ");
}

void run_main_menu(void) {
    while (true) {
        print_main_menu();
        char choice = keyboard_wait_read();

        switch (choice) {
            case '1':
                run_music_player();
                break;
            case '2':
                run_pong();
                break;
            case '3':
                run_matrix_effect();
                break;
            case '4':
                run_notepad();
                break;
            case '5':
                run_shutdown();
                break;
            default:
                printf("\nUnknown option '%c'\n\n", choice);
                printf("Press any key to return to the menu...");
                keyboard_wait_read();
                break;
        }
    }
}

void run_boot(void) {
    terminal_clear();
    printf("=== Lucky 13 OS boot ===\n\n");

    printf("Hello World\n");
    init_gdt();
    printf("GDT initialized\n");
    init_idt();
    printf("IDT initialized\n");
    init_isr();
    printf("ISR and IRQ handlers initialized\n");
    init_pit();
    printf("PIT initialized at %d Hz\n", PIT_TARGET_FREQUENCY);
    enable_interrupts();
    printf("Keyboard IRQ initialized\n");
    init_kernel_memory(&end);
    printf("Kernel memory initialized\n");
    init_paging();
    printf("Paging initialized\n");

    test_new_operator();
    print_memory_layout();

    printf("\nBoot checks complete. Starting menu in 5 seconds...\n");
    sleep_interrupt(5000);
}

void kernel_main(unsigned long magic, unsigned long multiboot_info) {
    run_boot();
    run_main_menu();
}
