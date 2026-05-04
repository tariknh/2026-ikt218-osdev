/*
 * gdt.h - Global Descriptor Table public API
 *
 * The GDT is a table of segment descriptors that the CPU consults whenever
 * it loads a segment register. Even in modern flat-memory operating systems
 * we still have to set up a GDT because the i386 CPU refuses to run
 * without one.
 *
 * For Assignment 2 we install three descriptors:
 *   index 0 - NULL descriptor (required by the CPU)
 *   index 1 - Code segment (also called "Text"), covers all of memory, ring 0
 *   index 2 - Data segment, covers all of memory, ring 0
 *
 * After gdt_install() returns, the segment registers point to our entries
 * and the CPU continues running in our flat 32-bit protected mode.
 */

#pragma once

void gdt_install(void);
