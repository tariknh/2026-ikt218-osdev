#include <libc/stddef.h>
#include <libc/stdint.h>

#include "terminal.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "kernel_memory.h"
#include "keyboard.h"
#include "paging.h"
#include "pit.h"
#include "cpp_memory_demo.h"
#include "song/song.h"

extern uint32_t end; // Defined in the linker script, marks the end of the kernel

static void print_sleep_test_banner(const char* label, uint32_t iteration, uint32_t total) {
    terminal_write(label);
    terminal_write(" ");
    terminal_write_dec(iteration);
    terminal_write("/");
    terminal_write_dec(total);
    terminal_write("...\n");
}

void main(void) {
    terminal_init();
    terminal_write("Hello, OS World!\n");

    gdt_init();
    terminal_write("GDT initialized successfully.\n");

    idt_init();
    isr_init();
    terminal_write("IDT initialized successfully.\n");

    irq_init();
    terminal_write("IRQ initialized successfully.\n");

    init_kernel_memory(&end);
    terminal_write("Kernel memory allocator initialized successfully.\n");
    print_memory_layout();

    init_paging();

    init_pit();

    keyboard_init();
    terminal_write("Keyboard initialized successfully.\n");

    __asm__ volatile ("sti"); // Enable interrupts after all tables/drivers are ready

    void* some_memory = malloc(1024); // Allocate 1 KB of memory
    void* memory2 = malloc(2048); // Allocate 2 KB of memory
    void* memory3 = malloc(13331); // Allocate 13 KB of memory
    void* cpp_memory = cpp_allocate_demo(); // Allocate memory using C++ demo function

    if (some_memory && memory2 && memory3 && cpp_memory) {
        terminal_write("Memory allocated successfully.\n");
    } else {
        terminal_write("One or more allocations failed.\n");
    }

    terminal_write("Memory layout after allocations:\n");
    print_memory_layout();

    free(memory2); // Free the 2 KB block
    cpp_free_demo(cpp_memory); // Free the C++ demo block
    terminal_write("Memory layout after freeing some blocks:\n");
    print_memory_layout();

    terminal_write("Testing busy sleep (high CPU) for 3 iterations.\n");
    for (uint32_t i = 1; i <= 3; i++) {
        print_sleep_test_banner("Busy sleep", i, 3);
        sleep_busy(1000);
    }
    terminal_write("Busy sleep test done.\n");

    terminal_write("Testing interrupt sleep (low CPU) for 3 iterations.\n");
    for (uint32_t i = 1; i <= 3; i++) {
        print_sleep_test_banner("Interrupt sleep", i, 3);
        sleep_interrupt(1000);
    }
    terminal_write("Interrupt sleep test done.\n");

    Song demo_song = get_demo_song();
    SongPlayer* player = create_song_player();
    if (player != NULL) {
        player->play_song(&demo_song);
        free(player);
    } else {
        terminal_write("Failed to create song player.\n");
    }
    
    terminal_write("Kernel initializaiton complete.\n");
    terminal_write("Type on the keyboard to see output:\n");

    for (;;) {
        // Halt the CPU to save power until the next interrupt
        __asm__ volatile ("hlt");
    }
}