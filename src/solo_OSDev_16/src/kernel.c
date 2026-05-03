#include "gdt.h"
#include "terminal.h"
#include "idt.h"
#include "irq.h"
#include "pic.h"
#include "memory.h"
#include "pit.h"
#include "system_monitor.h"

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

    system_monitor_screen();

    while (true) {
        __asm__ volatile("hlt");
        system_monitor_update();
    }
}