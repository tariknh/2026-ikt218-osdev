#include "gdt.h"
#include "terminal.h"
#include "idt.h"
#include "memory.h"    // init_kernel_memory, malloc, free, init_paging, print_memory_layout
 
// Linker symbol — first byte past the kernel image.
extern uint32_t end;
 
// Forward declaration — defined in kernel.cpp
int kernel_main();
 
// ─── Entry point ─────────────────────────────────────────────────────────────
 
void main(uint32_t mb2_magic, uint32_t mb2_info) {
 
    // 1. Hardware descriptor tables
    init_gdt();
    idt_init();
 
    // 2. Screen output
    terminal_initialize();
 
    // 3. Kernel heap — must come before any malloc() or new
    init_kernel_memory(&end);
 
    // 4. Paging — identity-maps first 4 MiB, enables CR0.PG
    init_paging();
 
    // 5. Physical memory map from Multiboot2 tags
    print_memory_layout(mb2_info);
 
    // 6. IRQ handlers
    irq_install_handler(1, keyboard_handler);
 
    // 7. Enable interrupts
    __asm__ __volatile__("sti");
 
    // 8. Hand off to C++ — heap and paging are ready
    kernel_main();
}

