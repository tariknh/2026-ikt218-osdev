#include "stdio.h"
#include "arch/i386/gdt.h"

void kernel_main(unsigned long magic, unsigned long multiboot_info) {
    gdt_init();

    printf("Hello World\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
