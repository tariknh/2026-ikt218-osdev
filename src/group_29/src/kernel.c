#include "libc/stdint.h"
#include "libc/stddef.h"
#include "libc/stdbool.h"
#include <multiboot2.h>
#include "gdt/gdt.h"
#include "interrupts/interrupts.h"
#include "memory/heap.h"
#include "vga_text_mode_interface/vga_text_mode_interface.h"
#include "format/format.h"

extern uint32_t end;

struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};



int kernel_main_c(uint32_t magic, struct multiboot_info* mb_info_addr) {
    (void)magic;
    (void)mb_info_addr;

    gdt_init();
    // char a[]= "Hello World!!";
    // char* vga_text = (char *) 0xb8000;
    // copyZeroTerminatedCharArrayToEvenPositionsInCharArray((char*)&a, vga_text);

    struct VgaTextModeInterface screen = NewVgaTextModeInterface();
    screen.Print(&screen, "GDT loaded successfully!\n\n", VgaColor(vga_cyan, vga_black));

    // GDT Test:
    uint16_t cs, ds, ss;
    __asm__ __volatile__("mov %%cs, %0" : "=r"(cs));
    __asm__ __volatile__("mov %%ds, %0" : "=r"(ds));
    __asm__ __volatile__("mov %%ss, %0" : "=r"(ss));

    if (cs == 0x08 && ds == 0x10 && ss == 0x10) {
        screen.Print(&screen, "GDT OK\n\n", VgaColor(vga_light_green, vga_black));
    } else {
        screen.Print(&screen, "GDT BAD\n\n", VgaColor(vga_light_red, vga_black));
    }

    init_idt();
    screen.Print(&screen, "IDT is initilalized\n\n", VgaColor(vga_black, vga_white));

    init_kernel_memory(&end);

    // Memory test:
    MemoryDebugData debug_data = get_memory_layout();
    screen.Print(&screen, debug_data.formatted, VgaColor(vga_black, vga_light_magenta));
    // print_memory_layout();

    void* first_block = malloc(128U);
    void* second_block = malloc(256U);

    if (first_block != NULL && second_block != NULL) {
        screen.Print(&screen, "Heap allocations OK\n\n", VgaColor(vga_black, vga_light_green));
    } else {
        screen.Print(&screen, "Heap allocations failed\n\n", VgaColor(vga_black, vga_light_red));
    }

    free(first_block);

    void* reused_block = malloc(64U);
    if (reused_block != NULL) {
        screen.Print(&screen, "free() reuse OK\n\n", VgaColor(vga_black, vga_light_green));
    } else {
        screen.Print(&screen, "free() reuse failed\n\n", VgaColor(vga_black, vga_light_red));
    }

    char input = "Testing formatting! -192: %d!\n";

    char* output = format_string(input, 31, -192);
    // screen.Print(&screen, output, VgaColor(vga_black, vga_white));

    free((void*)output);

    // Test how the os handels overflow:
    // while(1){screen.Print(&screen, "aaaaaaaaaaaaaaaaaaaaaa", VgaColor(vga_white, vga_black));}
    
    //Dont let the OS insta-reboot
    while(1){}
    return 0;
}
