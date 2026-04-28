#pragma once
#include "stdint.h"
#include "stdbool.h"

#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT 0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT 0xA1
#define PIC_EOI_COMMAND 0x20

/** A pointer to the IDT that will be passed to the LIDT instruction. */
struct idt_pointer {
    //! Size of the IDT-1
    uint16_t size;
    //! Location in memory
    uint32_t offset;
} __attribute__((packed));

/** An entry in the idt table.
 * \note This is for 32-bit ONLY
 * \see https://osdev.wiki/wiki/Interrupt_Descriptor_Table#Gate_Descriptor 
 */
struct idt_gate {
    uint16_t low_offset;
    uint16_t selector;
    uint8_t RESERVED_DO_NOT_USE;
    uint8_t attributes;
    uint16_t hi_offset;
} __attribute__((packed));

enum {
    idt_type_interrupt = 0b1110,
    idt_type_trap = 0b1111,
    idt_type_task_gate = 0b0101
};

struct interrupt_frame {
    uint32_t ip;      //!< Instruction Pointer
    uint32_t cs;      //!< Code Segment
    uint32_t flags;   //!< CPU Flags (EFLAGS)
    uint32_t sp;      //!< Stack Pointer (only if privilege level changed)
    uint32_t ss;      //!< Stack Segment (only if privilege level changed)
} __attribute__((packed));

/** Loads the interrupt descriptor table */
void load_idt(struct idt_pointer idt_pointer);
struct idt_gate create_idt_gate(uint32_t offset, uint16_t selector, uint8_t attributes);
uint8_t create_idt_attributes(bool present, int8_t ring, uint8_t type);
void pic_remap(int offset1, int offset2);
void init_idt();
void trigger_isr_demo(void);
void keyboard_interrupt_handler(struct interrupt_frame* frame);
void keyboard_callback();