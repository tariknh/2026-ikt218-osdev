#include "isr.h"
#include "terminal.h"

// Handle divide-by-zero interrupt
void isr0_handler(void){
    terminal_write("Interrupt 0: Divide by zero.\n");
}

// Handle breakpoint interrupt
void isr3_handler(void){
    terminal_write("Interrupt 3: Breakpoint.\n");
}

// Handle page fault interrupt
void isr14_handler(void){
    terminal_write("Interrupt 14: Page fault.\n");
}