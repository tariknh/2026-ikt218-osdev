#include "terminal.h"
#include "io.h"
#include "pic.h"
#include "keyboard.h"
#include "pit.h"
#include <libc/stdint.h>

// Handle IRQ0
void irq0_handler(void){
    pit_tick();
    pic_send_eoi(0);

}

// Handle IRQ1
// Read the keyboard scancode from the PS/2 data port
// and pass it to the keyboard handler
void irq1_handler(void){
    uint8_t scancode = inb(0x60); 
    keyboard_handle_scancode(scancode);
    pic_send_eoi(1);
}

// Handle IRQ2
void irq2_handler(void){
    terminal_write("IRQ 2 triggered.\n");
}

// Handle IRQ3
void irq3_handler(void){
    terminal_write("IRQ 3 triggered.\n");
}

// Handle IRQ4
void irq4_handler(void){
    terminal_write("IRQ 4 triggered.\n");
}

// Handle IRQ5
void irq5_handler(void){
    terminal_write("IRQ 5 triggered.\n");
}

// Handle IRQ6
void irq6_handler(void){
    terminal_write("IRQ 6 triggered.\n");
}

// Handle IRQ7
void irq7_handler(void){
    terminal_write("IRQ 7 triggered.\n");
}

// Handle IRQ8
void irq8_handler(void){
    terminal_write("IRQ 8 triggered.\n");
}

// Handle IRQ9
void irq9_handler(void){
    terminal_write("IRQ 9 triggered.\n");
}

// Handle IRQ10
void irq10_handler(void){
    terminal_write("IRQ 10 triggered.\n");
}

// Handle IRQ11
void irq11_handler(void){
    terminal_write("IRQ 11 triggered.\n");
}

// Handle IRQ12
void irq12_handler(void){
    terminal_write("IRQ 12 triggered.\n");
}

// Handle IRQ13
void irq13_handler(void){
    terminal_write("IRQ 13 triggered.\n");
}

// Handle IRQ14
void irq14_handler(void){
    terminal_write("IRQ 14 triggered.\n");
}

// Handle IRQ15
void irq15_handler(void){
    terminal_write("IRQ 15 triggered.\n");
}