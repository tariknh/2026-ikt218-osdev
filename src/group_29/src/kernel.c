#include "libc/stdint.h"
#include "libc/stddef.h"
#include "libc/stdbool.h"
#include <multiboot2.h>
#include "gdt/gdt.h"
#include "fs/tinyfs.h"
#include "interrupts/interrupts.h"
#include "keyboard/keyboard.h"
#include "memory/heap.h"
#include "pit/pit.h"
#include "shell/shell.h"
#include "sound/song_player.h"
#include "vga_text_mode_interface/vga_text_mode_interface.h"
#include "string/string.h"
#include "printing/printing.h"
#include "user/user.h"

extern uint32_t end;

struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};



int kernel_main_c(uint32_t magic, struct multiboot_info* mb_info_addr) {
    (void)magic;
    (void)mb_info_addr;

    // init vga interface for printing
    init_vga_interface_for_printing();

    // Initialize GDT, keyboard and shell username
    gdt_init();
    init_keyboard();
    init_username();

    print("\n GDT loaded successfully!\n\n");

    // GDT Test:
    uint16_t cs, ds, ss;
    __asm__ __volatile__("mov %%cs, %0" : "=r"(cs));
    __asm__ __volatile__("mov %%ds, %0" : "=r"(ds));
    __asm__ __volatile__("mov %%ss, %0" : "=r"(ss));

    if (cs == 0x08 && ds == 0x10 && ss == 0x10) {
        print(" GDT OK\n\n");
    } else {
        print(" GDT BAD\n\n");
    }

    init_idt();
    print(" IDT is initilalized\n\n");

    init_kernel_memory(&end);
    init_paging();
    // print_memory_layout();
    init_pit();
    tinyfs_init();
    // play_default_song();

    // Memory test:
    MemoryDebugData debug_data = get_memory_layout();
    print(debug_data.formatted);

    void* first_block = malloc(128U);
    void* second_block = malloc(256U);

    if (first_block != NULL && second_block != NULL) {
        print(" Heap allocations OK\n\n");
    } else {
        print(" Heap allocations failed\n\n");
    }

    free(first_block);

    void* reused_block = malloc(64U);
    if (reused_block != NULL) {
        print(" free() reuse OK\n\n");
    } else {
        print(" free() reuse failed\n\n");
    }

    // char* output = format_string(" Testing formatting: The terminal has %d rows\n", VGA_TERMINAL_HEIGHT);
    // print(output);
    // free((void *)output);

    if (tinyfs_is_ready()) {
        print(" TinyFS disk ready\n");
    } else {
        print(" TinyFS disk not formatted. Run format\n");
    }

    print("\n Run 'help' to view commands\n\n");

    shell_init("user");

    // Await for shell user input
    while (true) {
        shell_process_input();
    }

    return 0;
}
