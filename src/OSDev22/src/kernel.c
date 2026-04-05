#include "libc/stddef.h"
#include "libc/stdbool.h"
#include <multiboot2.h>
#include "libc/stdint.h"
#include "gdt.h"
#include "terminal.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "keyboard.h"
#include "memory.h"
#include "pit.h"

extern uint32_t end;

struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};

int main(uint32_t magic, struct multiboot_info* mb_info_addr) {
    (void)magic;
    (void)mb_info_addr;

    /* Sett opp GDT - må gjøres først for riktig segmentering */
    gdt_init();

    /* Klargjør skjermen */
    terminal_initialize();

    /* Sett opp interrupt-systemet */
    idt_init();     /* Opprett tom IDT-tabell */
    isr_init();     /* Registrer CPU-exceptions (0-31) */
    irq_init();     /* Remap PIC + registrer hardware IRQ-er (32-47) + sti */

    /* Initialiser minnehåndtering */
    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();

    /* Initialiser PIT */
    init_pit();

    /* Start tastatur-driver */
    keyboard_init();

    printf("Hello World\n");
    printf("Interrupt-system aktivert!\n");
    printf("Write something on keyboard:\n");

    /* Test malloc */
    void* some_memory = malloc(12345);
    void* memory2 = malloc(54321);
    void* memory3 = malloc(13331);

    printf("Allocated memory blocks:\n");
    printf("some_memory = 0x%x\n", (uint32_t)some_memory);
    printf("memory2     = 0x%x\n", (uint32_t)memory2);
    printf("memory3     = 0x%x\n", (uint32_t)memory3);

    /* Test PIT */
    uint32_t counter = 0;
    while (true) {
        printf("[%d]: Sleeping with busy-waiting (HIGH CPU).\n", counter);
        sleep_busy(1000);
        printf("[%d]: Slept using busy-waiting.\n", counter++);

        printf("[%d]: Sleeping with interrupts (LOW CPU).\n", counter);
        sleep_interrupt(1000);
        printf("[%d]: Slept using interrupts.\n", counter++);
    }

    return 0;
}