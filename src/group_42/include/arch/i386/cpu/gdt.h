#pragma once

#include <libc/stdbool.h>
#include <stdint.h>

#define GDT_ENTRIES 6

/**
 * Initialises the gdt with null segment, kernel code segment, kernel data segment, user code
 * segment and user data segment
 */
void init_gdt(void);
/**
 *Defined in tss.c
 * Initializes the TSS in the GDT. Must be called after init_gdt and after paging has been setup
 */
void init_tss(void);
/**
 * Updates the stack pointer register for the task state segment. Needed to switch to kernel stack
 * @param esp
 */
void tss_update_esp0(uint32_t esp);
/**
 * Set a gdt entry * @param num number
 * @param base base
 * @param limit limit
 * @param access access flags
 * @param flags DPL/CPL and permissions
 */
void gdt_set_entry(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
/**
 * Reloads the GDT
 */
void gdt_reload(void);
