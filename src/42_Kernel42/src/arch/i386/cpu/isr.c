#include "arch/i386/cpu/isr.h"

#include <kernel/pit.h>
#include <stdio.h>

#include "arch/i386/cpu/idt.h"
#include "arch/i386/cpu/ports.h"
#include "drivers/input/keyboard.h"
#include "kernel/util.h"

void page_fault_handler(registers_t* regs);

static isr_t interrupt_handlers[256];

static const char* const exception_names[] = {"Divide Error",
                                              "Debug",
                                              "NMI Interrupt",
                                              "Breakpoint",
                                              "Overflow",
                                              "BOUND Range Exceeded",
                                              "Invalid Opcode",
                                              "Device Not Available",
                                              "Double Fault",
                                              "Coprocessor Segment Overrun",
                                              "Invalid TSS",
                                              "Segment Not Present",
                                              "Stack-Segment Fault",
                                              "General Protection Fault",
                                              "Page Fault",
                                              "x87 Floating-Point",
                                              "Alignment Check",
                                              "Machine Check",
                                              "SIMD Floating-Point",
                                              "Virtualization",
                                              "Reserved 20",
                                              "Reserved 21",
                                              "Reserved 22",
                                              "Reserved 23",
                                              "Reserved 24",
                                              "Reserved 25",
                                              "Reserved 26",
                                              "Reserved 27",
                                              "Reserved 28",
                                              "Reserved 29",
                                              "Reserved 30",
                                              "Reserved 31"};

// Called from assembly stubs
void isr_handler(registers_t* regs) {
  if (regs->int_no < 32u) {
    if (regs->int_no == 14) {
      page_fault_handler(regs);
      return;
    }
    printf(exception_names[regs->int_no]);
    printf("\n");
    __asm__ volatile("hlt");
    return;
  }
  if (interrupt_handlers[regs->int_no] != 0) {
    if (regs->int_no == 128)
      printf("Syscall receaved\n");
    isr_t handler = interrupt_handlers[regs->int_no];
    handler(regs);
  } else {
    printf("Unhandled ISR: ");
    printf(exception_names[regs->int_no]);
    printf("\n");
  }
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
  interrupt_handlers[n] = handler;
}


void irq_handler(registers_t* regs) {
  if (regs->int_no >= 40)
    port_byte_out(0xA0, 0x20); // slave
  port_byte_out(0x20, 0x20);   // master

  if (interrupt_handlers[regs->int_no] != 0) {
    isr_t handler = interrupt_handlers[regs->int_no];
    handler(regs);
  } else {
    printf("Unhandled IRQ: ");
    char s[4];
    int_to_ascii(regs->int_no, s);
    printf(s);
    printf("\n");
  }
}


void init_irqs() {
  __asm__ volatile("sti");
  init_pit();
  init_keyboard();
}

void init_isrs() {
  for (int i = 0; i < 32; i++) {
    uint32_t addr = ((uint32_t*)&isr_stub_table)[i];
    set_idt_gate(i, addr);
  }

  // Remap PIC
  port_byte_out(0x20, 0x11);
  port_byte_out(0xA0, 0x11);
  port_byte_out(0x21, 0x20);
  port_byte_out(0xA1, 0x28);
  port_byte_out(0x21, 0x04);
  port_byte_out(0xA1, 0x02);
  port_byte_out(0x21, 0x01);
  port_byte_out(0xA1, 0x01);
  port_byte_out(0x21, 0x0);
  port_byte_out(0xA1, 0x0);

  // Install IRQs
  for (int i = 0; i < 16; i++) {
    uint32_t addr = ((uint32_t*)&irq_stub_table)[i];
    set_idt_gate(32 + i, addr);
  }

  // syscalls
  set_idt_gate_with_attrs(0x80, (uint32_t)(uintptr_t)syscall_stub, 0xEE);  /* Present=1, DPL=3, 32-bit interrupt gate */
}
