/*
Name: kernel.c
Project: LocOS
Description: This file contains the main entry point for the LocOS kernel. 
             This is a school project, learning about operating system development.
*/

#include <libc/stdint.h> //includes integer types like uint32_t
#include "gdt.h" //Includes a header file for GDT (Global Descriptor Table) initialization
#include "terminal.h" // Includes a header file for terminal output functions like terminal_init, terminal_write.

void kmain(void) { //Kernel entry function kmain, this is where the kernel starts running after bootloader handoff

    terminal_init();  // Calls a function from terminal.c to initialize terminal subsystem so text output works.
    terminal_write("LocOS v0.1\n"); // Calls the terminal_write function to print directly to the memory address to get output directly on screen
    terminal_write("Starting kernel...\n"); 
    terminal_write("Loading GDT...\n");
    gdt_init(); // Calls a function from gdt.c to load the GDT, which is needed for correct segmentation in x86
    terminal_write("GDT loaded successfully.\n");
    terminal_write("\n Hello World! (write)\n"); // Hello World print with terminal_write
    terminal_printf("\n Hello World! (printf)\n"); //Hello World print with printf
    for (;;) { //Starts an infinite loop so the kernel never returns
        __asm__ volatile ("hlt");  // Cpu halt instruction reducing CPU usage waiting for next interrupt
    }
}