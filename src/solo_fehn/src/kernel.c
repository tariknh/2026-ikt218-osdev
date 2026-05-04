/*
 * kernel.c - Kernel entry point
 *
 * Assignment 6 (Improvisation) wires up a small interactive shell on top
 * of every subsystem we have built so far. Type 'help' at the > prompt
 * to see the available commands.
 */

#include <libc/stdint.h>
#include <libc/stdio.h>
#include <gdt.h>
#include <idt.h>
#include <irq.h>
#include <keyboard.h>
#include <terminal.h>
#include <memory.h>
#include <paging.h>
#include <pit.h>
#include <shell.h>

void main(uint32_t mb_magic, uint32_t mb_info_addr) {
    (void)mb_magic;
    (void)mb_info_addr;

    terminal_initialize();

    printf("UiAOS booting...\n");

    gdt_install();
    idt_install();
    irq_install();
    keyboard_install();
    init_kernel_memory(&end);
    init_paging();
    init_pit();

    printf("[init] All subsystems online.\n\n");

    shell_init();

    __asm__ volatile ("sti");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
