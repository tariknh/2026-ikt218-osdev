#include "../include/libc/stdint.h"
#include "../include/libc/stddef.h"
#include "../include/libc/stdio.h"
#include "../include/memory.h"
#include "../include/pit.h"
#include "../include/menu.h"
#include "../include/gdt.h" // or init_gdt
#include "../include/idt.h" // or init_idt
#include "../include/pic.h"

// Requirement: Linker symbol
extern uint32_t end; 
void test_cpp_new(void);

void panic(const char* message) {
    // If the kernel panics, we stop everything
    __asm__ volatile("cli; hlt"); 
    while(1);
}

int main(uint32_t magic, uint32_t mb_info_addr) {
    // ---------------- Assignment Setup ----------------
    gdt_init();  
    idt_init();  
    pic_init();  

    // ---------------- Assignment 4: Memory + PIT Setup ----------------
    init_kernel_memory(&end);
    init_paging();
    init_pit();

    // Assignment 4: verify that C++ new/delete route through malloc/free.
    test_cpp_new();

    // ---------------- Assignment 3: Interrupt Demo ----------------
    // Uncomment one line at a time to test a specific software interrupt.
    // The exception handler is expected to print the interrupt information
    // and halt the kernel after the interrupt is triggered.
    //
    // __asm__ volatile ("int $0x0");
    // __asm__ volatile ("int $0x1");
    // __asm__ volatile ("int $0x2");

    // Enable interrupts so the PIT starts ticking.
    __asm__ volatile ("sti");
    run_menu_loop();
    return 0;
}
