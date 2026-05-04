#include <libc/stdint.h>
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "kernel/memory.h"
#include "kernel/pit.h"
#include "song/song.h"

extern uint32_t end;

void main(void)
{
    gdt_init();
    idt_init();
    isr_init();
    irq_init();

    printf("GDT, IDT, ISR and IRQ loaded\n");

    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();

    init_pit();

    printf("Memory manager initialized.\n");

    printf("Testing PIT sleep...\n");

    printf("Sleeping busy...\n");
    sleep_busy(1000);
    printf("Busy sleep done.\n");

    printf("Sleeping interrupt...\n");
    sleep_interrupt(1000);
    printf("Interrupt sleep done.\n");

    printf("Testing music player...\n");
    play_music();
    printf("Music test finished.\n");

    __asm__ volatile("int $0x0");
    __asm__ volatile("int $0x1");
    __asm__ volatile("int $0x2");

    while (1)
    {
        __asm__ volatile("hlt");
    }
}