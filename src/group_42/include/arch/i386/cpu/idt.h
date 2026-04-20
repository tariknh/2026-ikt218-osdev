#pragma once

#define IDT_ENTRIES 256
#define KERNEL_CS 0x08


#include <libc/stdint.h>


/**
 * One element of the idt, called a gate
 */
typedef struct __attribute__((packed)) {
  uint16_t low_offset; // Lower 16 bits of ISR address
  uint16_t kernel_cs;  // GDT segment selector
  uint8_t reserved;    // Set to zero
  uint8_t attributes;
  uint16_t high_offset; // Upper 16 bits of ISR address
} idt_gate_t;

/**
 * The interrupt descriptor table register
 */
typedef struct __attribute__((packed)) {
  uint16_t limit;
  uint32_t base;
} idtr_t;


// Defined in interrupts.asm
extern idt_gate_t idt[IDT_ENTRIES];
extern idtr_t idt_reg;


/**
 * Initializes interrupt descriptor table, interrupt service routines and interrupt requests
 */
void init_idt();

/**
 * Sets a gate in the interrupt descriptor table
 * @param n index of the gate
 * @param handler handler to set
 */
void set_idt_gate(int n, uint32_t handler);
/**
 * Same as set_idt_gate, but allows attributes to be set
 */
void set_idt_gate_with_attrs(int n, uint32_t handler, uint8_t attrs);
