#pragma once
#include "stdint.h"
#include "stdbool.h"
#include "../io/io.h"

#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT 0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT 0xA1
#define PIC_EOI_COMMAND 0x20

/** \brief A pointer to the IDT that will be passed to the LIDT instruction. */
struct idt_pointer {
    //! Size of the IDT-1
    uint16_t size;
    //! Location in memory
    uint32_t offset;
} __attribute__((packed));

/** \brief An entry in the idt table.
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

enum idt_types {
    idt_type_interrupt = 0b1110,
    idt_type_trap = 0b1111,
    idt_type_task_gate = 0b0101
};

/** */
struct interrupt_frame {
    uint32_t ip;      //!< Instruction Pointer
    uint32_t cs;      //!< Code Segment
    uint32_t flags;   //!< CPU Flags (EFLAGS)
    uint32_t sp;      //!< Stack Pointer (only if privilege level changed)
    uint32_t ss;      //!< Stack Segment (only if privilege level changed)
} __attribute__((packed));

/// @brief Tell the CPU to load the IDT
/// @param idt_pointer A pointer structure that the CPU can read
void load_idt(struct idt_pointer idt_pointer);

/**
 * \param attributes Use create_idt_attributes()
 */
struct idt_gate create_idt_gate(uint32_t offset, uint16_t selector, uint8_t attributes);

/**
 * \param present If it's present in memory
 * \param ring Between 0 and 3, specifies the privelege of the ISR
 * \param type one from idt_types
 */
uint8_t create_idt_attributes(bool present, int8_t ring, uint8_t type);

/** \brief Needed to make the programmable interrupt controller work
 * \see https://wiki.osdev.org/8259_PIC#Initialisation
 */
void pic_remap(int offset1, int offset2);

/// @brief Sets up the IDT with needed interrupt for system functionality
void init_idt();

void trigger_isr_demo(void);
void keyboard_interrupt_handler(struct interrupt_frame* frame);

/// @brief Called by keyboard_interrupt_handler
void keyboard_callback();