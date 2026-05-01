#include "gdt.h"
#include "terminal.h"
#include "idt.h"
#include "irq.h"
#include "pic.h"
#include "memory.h"
#include "pit.h"
#include "song/song.h"

#include <libc/stdint.h>

extern uint32_t end;

void kmain(void) {
    gdt_initialize();
    terminal_initialize();
    idt_init();
    pic_remap();
    init_pit();
    __asm__ volatile("sti");

    init_kernel_memory(&end);

    init_paging();

    print_memory_layout();

    terminal_write("Hello World!\n");

terminal_write("Playing music_1...\n");
play_song(&music_1);
terminal_write("Finished music_1.\n");

terminal_write("Playing music_2...\n");
play_song(&music_2);
terminal_write("Finished music_2.\n");

terminal_write("Playing music_3...\n");
play_song(&music_3);
terminal_write("Finished music_3.\n");

terminal_write("Playing music_4...\n");
play_song(&music_4);
terminal_write("Finished music_4.\n");

    void* some_memory = malloc(12345);
    void* memory2 = malloc(54321);
    void* memory3 = malloc(13331);

    print_memory_layout();

    // kernel_main();

    while (true) {
        terminal_write("Sleeping with busy-waiting (HIGH CPU).\n");
        sleep_busy(1000);
        terminal_write("Slept using busy-waiting.\n");

        terminal_write("Sleeping with interrupts (LOW CPU).\n");
        sleep_interrupt(1000);
        terminal_write("Slept using interrupts.\n");
    }
    
}
