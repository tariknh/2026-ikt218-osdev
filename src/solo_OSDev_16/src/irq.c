#include "terminal.h"
#include "io.h"
#include "pic.h"
#include "keyboard.h"
#include "pit.h"
#include "system_monitor.h"

#include <libc/stdint.h>

// Storing counted keybord interrupts IRQ1
static volatile uint32_t irq1_ticks = 0;
// Storing last used scancode by keybord, used for monitor data
static volatile uint8_t last_keybord_scancode = 0;

// Getters for the monitor
uint32_t get_irq1_count() {
    return irq1_ticks;
}
uint8_t get_last_keyboard_scancode() {
    return last_keybord_scancode;
}

// Handle IRQ0
void irq0_handler(void){
    pit_tick();
    pic_send_eoi(0);

}

// Handle IRQ1
// Read the keyboard scancode from the PS/2 data port
// and pass it to the keyboard handler
void irq1_handler(void){
    irq1_ticks++;

    uint8_t scancode = inb(0x60); 
    last_keybord_scancode = scancode;

    keyboard_handle_scancode(scancode);
    system_monitor_handle_input(scancode);

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