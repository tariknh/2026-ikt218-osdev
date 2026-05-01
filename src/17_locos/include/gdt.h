/*
Name: gdt.h
Project: LocOS
Description: This file contains the definitions and function declarations for the Global Descriptor Table (GDT)
*/

#ifndef GDT_H
#define GDT_H

#include <libc/stdint.h>

void gdt_init(void);

#endif
