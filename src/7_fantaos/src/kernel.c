#include <libc/stdint.h>
#include <gdt.h>
#include <idt.h>
#include <pic.h>
#include <keyboard.h>
#include <terminal.h>
#include <memory.h>
#include <pit.h>

extern uint32_t end; // Defined in arch/i386/linker.ld, marks where kernel image ends.

void main(uint32_t magic, void *mbi) {
    (void)magic;
    (void)mbi;

    gdt_init();
    terminal_init();
    printf("Launching Fanta OS\n\n");
    idt_init();

    // Test three ISRs (Task 2)
    printf("Testing ISRs...\n");
    asm volatile("int $0x0");  // #DE - Division Error
    asm volatile("int $0x3");  // #BP - Breakpoint
    asm volatile("int $0x6");  // #UD - Invalid Opcode
    printf("ISR test complete.\n");

    // Initialize memory manager and enable paging.
    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();

    void *mem1 = malloc(12345);
    void *mem2 = malloc(54321);
    void *mem3 = malloc(13331);
    printf("malloc test: %x %x %x\n", (uint32_t)mem1, (uint32_t)mem2, (uint32_t)mem3);

    // Remap PIC, register handlers, initialize PIT, then enable interrupts.
    pic_init();
    keyboard_init();
    init_pit();
    asm volatile("sti");

    uint32_t counter = 0;
    for (;;) {
        printf("[%d]: Sleeping with busy-waiting (HIGH CPU).\n", counter);
        sleep_busy(1000);
        printf("[%d]: Slept using busy-waiting.\n", counter++);

        printf("[%d]: Sleeping with interrupts (LOW CPU).\n", counter);
        sleep_interrupt(1000);
        printf("[%d]: Slept using interrupts.\n", counter++);
    }
}
