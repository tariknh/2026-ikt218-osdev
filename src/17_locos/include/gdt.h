/*
Name: gdt.h
Project: LocOS
Description: This file contains the definitions and function declarations for the Global Descriptor Table (GDT)
*/

#ifndef GDT_H
#define GDT_H

#include <libc/stdint.h> // Includes stdint.h already given in task

typedef struct __attribute__((packed)) { // Defines the structure of a GDT entry packed to prevent padding
    uint16_t limit_low; //Stores low 16 its of segment limit
    uint16_t base_low; // Stores low 16 bits of base address
    uint8_t  base_middle; // Stores bits 16-23 of base address
    uint8_t  access; // Stores access byte 
    uint8_t  granularity; //Stores granularity (low nibble / high nibble)
    uint8_t  base_high; // Stores bits 24-31 of base address
} gdt_entry_t; //end

typedef struct __attribute__((packed)) { 
    uint16_t limit; // stores size of GDT in bytes minus 1
    uint32_t base; //stores memory address of first GDT entry
} gdt_ptr_t;

void gdt_init(void); // Declares the main GDT init function which will be called in kernel.c. The insides of the function is in gdt.c
void gdt_load(uint32_t gdt_ptr_addr); // Executes lgdt and reloads segment registers.

#endif