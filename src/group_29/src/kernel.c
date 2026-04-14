#include "libc/stdint.h"
#include "libc/stddef.h"
#include "libc/stdbool.h"
#include <multiboot2.h>
#include "gdt/gdt.h"
#include "interrupts/interrupts.h"
#include "keyboard/keyboard.h"
#include "memory/heap.h"
#include "pit/pit.h"
#include "sound/song_player.h"
#include "vga_text_mode_interface/vga_text_mode_interface.h"
#include "string/string.h"
#include "printing/printing.h"

extern uint32_t end;

struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};



int kernel_main_c(uint32_t magic, struct multiboot_info* mb_info_addr) {
    (void)magic;
    (void)mb_info_addr;
    uint32_t counter = 0U;

    // init vga interface for printing
    init_vga_interface_for_printing();

    // Terminal header and footer
    print_color(" HEADER - This row and the one below will not be printed/scrolled on\n\n", VgaColor(vga_white, vga_black));
    write_text_at(VGA_TERMINAL_HEIGHT - 1, 0, "This row and the one above will not be printed/scrolled on", VgaColor(vga_white, vga_black));

    gdt_init();
    init_keyboard();
    // char a[]= "Hello World!!";
    // char* vga_text = (char *) 0xb8000;
    // copyZeroTerminatedCharArrayToEvenPositionsInCharArray((char*)&a, vga_text);

    // struct VgaTextModeInterface screen = NewVgaTextModeInterface();
    // screen.Print(&screen, "GDT loaded successfully!\n\n", VgaColor(vga_cyan, vga_black));


    print_color("GDT loaded successfully!\n\n", VgaColor(vga_cyan, vga_black));

    // GDT Test:
    uint16_t cs, ds, ss;
    __asm__ __volatile__("mov %%cs, %0" : "=r"(cs));
    __asm__ __volatile__("mov %%ds, %0" : "=r"(ds));
    __asm__ __volatile__("mov %%ss, %0" : "=r"(ss));

    if (cs == 0x08 && ds == 0x10 && ss == 0x10) {
        // screen.Print(&screen, "GDT OK\n\n", VgaColor(vga_light_green, vga_black));
        print_color("GDT OK\n\n", VgaColor(vga_light_green, vga_black));
    } else {
        // screen.Print(&screen, "GDT BAD\n\n", VgaColor(vga_light_red, vga_black));
        print_color("GDT BAD\n\n", VgaColor(vga_light_green, vga_black));
    }

    init_idt();
    // screen.Print(&screen, "IDT is initilalized\n\n", VgaColor(vga_black, vga_white));
    print_color("IDT is initilalized\n\n", VgaColor(vga_black, vga_white));

    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();
    init_pit();
    // play_default_song();

    // Memory test:
    MemoryDebugData debug_data = get_memory_layout();
    // screen.Print(&screen, debug_data.formatted, VgaColor(vga_black, vga_light_magenta));
    print_color(debug_data.formatted, VgaColor(vga_black, vga_light_magenta));

    void* first_block = malloc(128U);
    void* second_block = malloc(256U);

    if (first_block != NULL && second_block != NULL) {
        // screen.Print(&screen, "Heap allocations OK\n\n", VgaColor(vga_black, vga_light_green));
        print_color("Heap allocations OK\n\n", VgaColor(vga_black, vga_light_green));
    } else {
        // screen.Print(&screen, "Heap allocations failed\n\n", VgaColor(vga_black, vga_light_red));
        print_color("Heap allocations failed\n\n", VgaColor(vga_black, vga_light_red));
    }

    free(first_block);

    void* reused_block = malloc(64U);
    if (reused_block != NULL) {
        // screen.Print(&screen, "free() reuse OK\n\n", VgaColor(vga_black, vga_light_green));
        print_color("free() reuse OK\n\n", VgaColor(vga_black, vga_light_green));
    } else {
        // screen.Print(&screen, "free() reuse failed\n\n", VgaColor(vga_black, vga_light_red));
        print_color("free() reuse failed\n\n", VgaColor(vga_black, vga_light_red));
    }

    // char input[] = "Testing formatting: string: -192 | formatted number: %d\n";

    char* output = format_string("Testing formatting: The terminal has %d rows\n", VGA_TERMINAL_HEIGHT);
    // screen.Print(&screen, output, VgaColor(vga_black, vga_white));
    print_color(output, VgaColor(vga_black, vga_white));
    free((void *)output);

    while (true) {
        // print_color("HELLO WORLD\n", VgaColor(vga_dark_gray, vga_light_red));

        // char* busy_start = format_string("[%d]: Sleeping with busy-waiting (HIGH CPU).\n", (int32_t)counter);
        // if (busy_start != NULL) {
        //     // screen.Print(&screen, busy_start, VgaColor(vga_white, vga_black));
        //     print_color(busy_start, VgaColor(vga_white, vga_black));
        //     free((void *)busy_start);
        // }
        // counter++;

        // const char *string = format_string("Counter: %d\n", counter);
        // print_color(string, VgaColor(vga_black, vga_light_magenta));
        // free(string);
        // string = NULL;
        // sleep_busy(500U);

        // char* busy_done = format_string("[%d]: Slept using busy-waiting.\n", (int32_t)counter);
        // if (busy_done != NULL) {
        //     // screen.Print(&screen, busy_done, VgaColor(vga_light_green, vga_black));
        //     print_color(busy_done, VgaColor(vga_light_green, vga_black));
        //     free((void *)busy_done);
        // }
        // ++counter;

        // char* interrupt_start = format_string("[%d]: Sleeping with interrupts (LOW CPU).\n", (int32_t)counter);
        // if (interrupt_start != NULL) {
        //     // screen.Print(&screen, interrupt_start, VgaColor(vga_light_cyan, vga_black));
        //     print_color(interrupt_start, VgaColor(vga_light_cyan, vga_black));
        //     free((void *)interrupt_start);
        // }

        // sleep_interrupt(1000U);

        // char* interrupt_done = format_string("[%d]: Slept using interrupts.\n", (int32_t)counter);
        // if (interrupt_done != NULL) {
        //     // screen.Print(&screen, interrupt_done, VgaColor(vga_light_green, vga_black));
        //     print_color(interrupt_done, VgaColor(vga_light_green, vga_black));
        //     free((void *)interrupt_done);
        // }
        // ++counter;
    }

    return 0;
}
