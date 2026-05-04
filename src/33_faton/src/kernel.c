#include <libc/stdint.h>
#include <monitor.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <irq.h>
#include <keyboard.h>
#include <memory.h>
#include <pit.h>
#include <menu.h>

extern uint32_t end;

void main(uint32_t magic, void* mb_info) {
    monitor_initialize();
    init_gdt();
    init_idt();
    init_irq();
    init_keyboard();

    __asm__ __volatile__("sti");

    init_kernel_memory(&end);
    init_paging();
    init_pit();

    run_menu();
}