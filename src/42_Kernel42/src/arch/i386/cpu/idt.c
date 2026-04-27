#include "arch/i386/cpu/idt.h"

#include <kernel/log.h>

#include "arch/i386/cpu/isr.h"

idt_gate_t idt[IDT_ENTRIES];
idtr_t idt_reg;


void set_idt_gate(int n, uint32_t handler) {
  idt[n].low_offset = handler & 0xFFFF;
  idt[n].kernel_cs = KERNEL_CS;
  idt[n].reserved = 0;
  idt[n].attributes = 0x8E;
  idt[n].high_offset = (handler >> 16) & 0xFFFF;
}

void set_idt_gate_with_attrs(int n, uint32_t handler, uint8_t attrs) {
  idt[n].low_offset = handler & 0xFFFF;
  idt[n].kernel_cs = KERNEL_CS;
  idt[n].reserved = 0;
  idt[n].attributes = attrs;
  idt[n].high_offset = (handler >> 16) & 0xFFFF;
}


void init_idt() {
  log_info("Initialising IDT...\n");
  // set idt limit
  idt_reg.base = (uintptr_t)&idt[0];
  idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;

  init_isrs();
  init_irqs();

  __asm__ volatile("lidtl (%0)" : : "r"(&idt_reg)); // load new IDT
}
