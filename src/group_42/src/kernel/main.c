#include <boot/multiboot2.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>

#include "drivers/video/vga_terminal.h"
#include "libc/stdbool.h"
#include "libc/stddef.h"
#include "libc/stdint.h"
#include "libc/stdio.h"


void kernel_main(uint32_t magic, void* addr);

/// Entrypoint
/// @param magic multiboot magic number
/// @param addr  address of the multiboot_info struct
/// @return shouldn't return
int k_init(uint32_t magic, struct multiboot_info* addr) {
  kernel_main(magic, addr);

  // If kernel_main fails, disable interrupts and halt
  asm volatile("cli");
  while (true) {
    asm volatile("hlt");
  }
}


void kernel_main(uint32_t magic, void* addr) {
  vga_terminal_initialise();

  // NOTE: Verify multiboot magic number, if it fails, then addr pointer is likely corrupted, so we
  // halt.
  if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    printf("Error: Invalid magic number 0x%x. Expected 0x%x\n", magic, MULTIBOOT2_BOOTLOADER_MAGIC);
    printf("Halting system.\n");
    return;
  }
  // parse multiboot info, to get memory map and kernel modules, or other drivers.

  printf("Initializing Group_42 Kernel...\n");
  if (!init_gdt()) {
    printf("Failed to initialize GDT. Halting...\n");
    return;
  }
  // init_idt();

  printf("Kernel initialised successfully.\n");
  printf("Welcome to Group_42 Kernel!\n\n\n");

  // Assignment 2
  printf("Hello World!\n\n\n");


  while (true) {
    __asm__ volatile("hlt");
  }
}
