#include "terminal.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "memory.h"
#include "pit.h"

extern uint32_t end;

void main(uint32_t mb_magic, void* mb_info) {
    (void)mb_magic; //stop warnings about unused variables
    (void)mb_info;

    terminal_initialize();
    init_gdt();
    init_idt();
    init_irq();
    init_kernel_memory(&end);  
    init_paging();             
    print_memory_layout();     
    init_pit();                

    __asm__ volatile ("sti"); //enable interrupts after PIC is remapped

    //asm("int $0x1"); //manually trigger interrupt

    printf("Hello World\n");

    uint32_t counter = 0;

    while(1) {
        printf("[%d]: Sleeping with busy-waiting (HIGH CPU).\n", counter);
        sleep_busy(1000);
        printf("[%d]: Slept using busy-waiting.\n", counter++);

        printf("[%d]: Sleeping with interrupts (LOW CPU).\n", counter);
        sleep_interrupt(1000);
        printf("[%d]: Slept using interrupts.\n", counter++);
    };
}