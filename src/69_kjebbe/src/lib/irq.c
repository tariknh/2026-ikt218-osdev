#include "../../include/irq.h"
#include "../../include/idt.h"
#include "../../include/io.h"

// PIC remapping
// The PIC has a has 16 irq lines spread over two chips, a master chip and a
// slave chip. Each of them have a port to send data to and commands to.
static void pic_remap(void) {
  outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); // ICW1: init master PIC
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4); // ICW1: init slave PIC

  outb(PIC1_DATA,
       0x20); // ICW2: master offsets IRQ 0-7 by 32 bits to interrupts 32-39
  outb(PIC2_COMMAND,
       0x28); // ICW2: slave offsets  IRQ 8-15 by 40 bytes to interrupts 40-47

  outb(PIC1_DATA, 1 << CASCADE_IRQ); // ICW3: Bit mask tells master there is
                                     // exactly one slave on irq line 2
  outb(PIC2_DATA, CASCADE_IRQ); // ICW3: Tells slave which irq line it is on in
                                // relation to the master.

  outb(PIC1_DATA, ICW4_8086); // ICW4: 8086 mode on master
  outb(PIC2_DATA, ICW4_8086); // ICW4: 8086 mode on slave

  outb(PIC1_DATA, 0); // unmask all interrupts on master
  outb(PIC2_DATA, 0); // unmask all interrupts on slave
}

// Initialise interrupt requests.
void irq_init(void) {
  pic_remap();

  // Inserts the 16 stubs into IDT.
  // Code segment: 0x08
  // 0x8E Present,Kernel level privelege, interrupt gate.
  idt_set_entry(32, (uint32_t)irq0, 0x08, 0x8E);
  idt_set_entry(33, (uint32_t)irq1, 0x08, 0x8E);
  idt_set_entry(34, (uint32_t)irq2, 0x08, 0x8E);
  idt_set_entry(35, (uint32_t)irq3, 0x08, 0x8E);
  idt_set_entry(36, (uint32_t)irq4, 0x08, 0x8E);
  idt_set_entry(37, (uint32_t)irq5, 0x08, 0x8E);
  idt_set_entry(38, (uint32_t)irq6, 0x08, 0x8E);
  idt_set_entry(39, (uint32_t)irq7, 0x08, 0x8E);
  idt_set_entry(40, (uint32_t)irq8, 0x08, 0x8E);
  idt_set_entry(41, (uint32_t)irq9, 0x08, 0x8E);
  idt_set_entry(42, (uint32_t)irq10, 0x08, 0x8E);
  idt_set_entry(43, (uint32_t)irq11, 0x08, 0x8E);
  idt_set_entry(44, (uint32_t)irq12, 0x08, 0x8E);
  idt_set_entry(45, (uint32_t)irq13, 0x08, 0x8E);
  idt_set_entry(46, (uint32_t)irq14, 0x08, 0x8E);
  idt_set_entry(47, (uint32_t)irq15, 0x08, 0x8E);
}

// IRQ handler table
static void (*irq_handlers[16])(registers_t *) = {0};
// Function which adds IRQ handlers to the IRQ handler table
void irq_register_handler(int irq, void (*handler)(registers_t *)) {
  irq_handlers[irq] = handler;
}

// IRQ handler
void irq_handler(registers_t *regs) {
  int irq = regs->int_no - 32;

  if (irq_handlers[irq]) {
    irq_handlers[irq](regs);
  }
  // IRQ 0-7 are mapped to the master PIC. And IRQ 8-15 are mapped to the slave
  // PIC. Since we have shifted all interrupts up by 32 entries, that means that
  // the the slave PIC uses interrupt numbers starting from 40. Therefore we
  // must send EOI to both the slave and master PIC when the interrupt number is
  // larger or equal to 40.
  if (regs->int_no >= 40) {
    outb(PIC2_COMMAND, 0x20); // Send EOI to the slave PIC
  }
  outb(PIC1_COMMAND, 0x20); // Send EOI to the master PIC
}
