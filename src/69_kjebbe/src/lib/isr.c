#include "../../include/isr.h"
#include "../../include/idt.h"
#include "../../include/libc/stdint.h"
#include "../../include/libc/stdio.h"

// Names for the first 3 CPU exceptions we handle
static const char *exception_messages[] = {
    "Division By Zero",      // ISR 0
    "Debug",                 // ISR 1
    "Non-Maskable Interrupt" // ISR 2
};

// Insert the three ISR stubs into the IDT.
// 0x08     = code segment selector, entry 1 in GDT
// 0x8E     = 32-bit interrupt gate, kernel level privelege , present. See the
// idt_entry_t struct in idt.h for more info.
void isr_init(void) {
  idt_set_entry(0, (uint32_t)isr0, 0x08, 0x8E);
  idt_set_entry(1, (uint32_t)isr1, 0x08, 0x8E);
  idt_set_entry(2, (uint32_t)isr2, 0x08, 0x8E);
}

// Called from isr_common_stub with a pointer to the saved register state.
void isr_handler(registers_t *regs) {
  printf("ISR fired: interrupt %d: %s\n", regs->int_no,
         exception_messages[regs->int_no]);
}
