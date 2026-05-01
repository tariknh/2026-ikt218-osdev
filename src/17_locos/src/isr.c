/*
Name: isr.c
Project: LocOS
Description: This file contains the implementation of the ISR (Interrupt Service Routine) handler for CPU exceptions
*/

#include <libc/stdint.h>        
#include "terminal.h" // Print to the screen

void isr_handler_c(uint32_t int_no) {  //Handle CPU exceptions and software interrupts.
    switch (int_no) { //Check which interrupt happened.
    case 0x00:      // Divide by zero
        terminal_write("ISR 0x00 triggered (Divide Error)\n");
        break;
    case 0x01: // Debug exception
        terminal_write("ISR 0x01 triggered (Debug Exception)\n");
        break;
    case 0x02: // NMI (non-maskable interrupt)
        terminal_write("ISR 0x02 triggered (NMI)\n");
        break;
    case 0x30:// Test software interrupt
        terminal_write("Hey! Don't interrupt me!\n");
        break;
    default:// Any other exception.
        terminal_printf("ISR 0x%x triggered\n", int_no);
        break;
    }
}
