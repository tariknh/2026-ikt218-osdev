#include <boot/multiboot2.h>
#include <drivers/audio/PCSPK.h>
#include <kernel/paging.h>
#include <kernel/panic.h>
#include <kernel/pit.h>
#include <kernel/pmm.h>
#include <kernel/util.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "arch/i386/cpu/gdt.h"
#include "arch/i386/cpu/idt.h"
#include "drivers/input/keyboard.h"
#include "drivers/video/vga_text.h"
#include "kernel/filesystem/ramfs.h"
#include "kernel/filesystem/vfs.h"
#include "kernel/log.h"
#include "kernel/syscall.h"
#include "shell/shell.h"

extern const uint8_t shell_elf[];
extern const uint32_t shell_elf_size;


extern uint32_t end;

void kernel_main(uint32_t magic, void* addr);

/// Entrypoint
/// @param magic multiboot magic number
/// @param addr  address of the multiboot_info struct
/// @return shouldn't return
int k_init(uint32_t magic, struct multiboot_info* addr) {
  kernel_main(magic, addr);

  // If kernel_main fails, disable interrupts and halt

  __asm__ volatile("cli");
  while (true) {
    __asm__ volatile("hlt");
  }
}

void kernel_main(uint32_t magic, void* addr) {
  vga_text_initialise();
  log_init();

  log_info("Initializing Group_42 Kernel...\n");

  log_info("TSC: 0x%llx (%llu cycles)\n", rdtsc(), rdtsc());

  if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    kernel_panic("Invalid magic number 0x%x. Expected 0x%x\n", magic, MULTIBOOT2_BOOTLOADER_MAGIC);
  }

  init_gdt();
  init_idt();
  init_pit();
  keyboard_set_scancode_set2();

  pmm_init(addr);
  init_paging(addr);
  // This populates and reloads another segment in GDT, but needs paging to be setup first
  init_tss();
  init_syscalls();
  PCSPK_init();

  log_info("PMM: %d free frames (%d KB)\n", pmm_get_free_count(), pmm_get_free_count() * 4);
  printf("\nStarting kernel shell...\n");
  printf("Type 'help' for commands.\n");
  printf("Use 'run_userspace' to launch userspace shell.\n\n");

  // Start virtual file systems
  vfs_init();
  ramfs_init("/");

  vga_text_enable_debug_serial(true);
  log_info("Starting shell...\n");
  shell_run();

  // If shell ever exits, main will kernel_main will return to k_init which will disable interrupts
  // and halt the CPU
}
