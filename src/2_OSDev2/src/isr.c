#include <libc/stdint.h>
#include "isr.h"
#include "idt.h"
#include "terminal.h"

extern void isr0(void); // ISR for interrupt 0 (divide by zero)
extern void isr6(void); // ISR for interrupt 6 (invalid opcode)
extern void isr13(void); // ISR for interrupt 13 (general protection fault)

// More to come as needed

void isr_init(void) {
    // 0x08 is the code segment selector for kernel code in the GDT
    // 0x8E is the type and attributes for a 32-bit interrupt gate (present, ring 0)
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E); // Set ISR for interrupt 0
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E); // Set ISR for interrupt 6
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E); // Set ISR for interrupt 13
}

static void u32_to_hex(uint32_t value, char* out) { // Convert a 32-bit unsigned integer to a hexadecimal string
    const char* hex_digits = "0123456789ABCDEF";
    for (int i = 0; i < 8; i++) {
        out[2+i] = hex_digits[(value >> (28 - i * 4)) & 0xF]; // Convert each nibble to hex
    }
    out[10] = '\0'; // Null-terminate the string
}

void isr_handler(isr_frame_t* frame) {
    terminal_write("Received interrupt: ");
    char hex_str[11]; // Buffer for hexadecimal string (8 digits + "0x" + null terminator)
    u32_to_hex(frame->int_no, hex_str); // Convert the interrupt number to a hex string
    terminal_write(hex_str); // Print the interrupt number
    terminal_write("\n");
    terminal_write("Error code: ");
    u32_to_hex(frame->err_code, hex_str); // Convert the error code to a hex string
    terminal_write(hex_str); // Print the error code
    terminal_write("\n");
}