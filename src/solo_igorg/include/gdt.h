#ifndef GDT_H
#define GDT_H

#include <libc/stdint.h>

/*
 * Initializes and loads GDT, which contains:
 *  - null descriptor
 *  - kernel code descriptor
 *  - kernel data descriptor
 */
void gdt_initialize(void);

#endif
