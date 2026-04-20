#include <boot/multiboot2.h>
#include <drivers/audio/PCSPK.h>
#include <kernel/memory.h>
#include <kernel/panic.h>
#include <kernel/pit.h>
#include <kernel/util.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "arch/i386/cpu/gdt.h"
#include "arch/i386/cpu/idt.h"
#include "drivers/input/keyboard.h"
#include "drivers/video/vga_text.h"
#include "kernel/log.h"
#include "kernel/syscall.h"
#include "kernel/util.h"
#include "kernel/filesystem/ramfs.h"
#include "kernel/filesystem/vfs.h"
#include "shell/shell.h"


struct multiboot_info;
extern uint32_t end; // defined in arch/i386/linker.ld

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

char* strcat(char* dest, const char* src) {
  char* ptr = dest;
  while (*ptr) ptr++;         // Find the end of dest
  while ((*ptr++ = *src++));  // Copy src including null terminator
  return dest;
}

void vfs_debug_tree(const char* path, int level) {
  // Special case: If we are at the very start, print the root marker
  if (level == 0) {
    vga_text_setcolor(VGA_COLOR_LIGHT_GREEN | VGA_COLOR_BLACK << 4);
    printf("/ (first level)\n");
    vga_text_setcolor(VGA_COLOR_WHITE | VGA_COLOR_BLACK << 4);
  }

  int fd = vfs_open(path, O_READ);
  if (fd == VFS_INVALID_FD) return;

  vfs_dirent_t entry;
  while (vfs_read(fd, &entry, 1) > 0) {
    // 1. Indentation
    for (int i = 0; i <= level; i++) printf("  ");

    if (entry.type == VFS_DIRECTORY) {
      // 2. Print Directory with leading slash
      vga_text_setcolor(VGA_COLOR_LIGHT_GREEN | VGA_COLOR_BLACK << 4);
      printf("/%s\n", entry.name);
      vga_text_setcolor(VGA_COLOR_WHITE | VGA_COLOR_BLACK << 4);

      // 3. Build next path manually (Avoids strcat)
      char next_path[MAX_PATH_LEN];
      int len = strlen(path);
      memcpy(next_path, path, len);

      if (path[len - 1] != '/') {
        next_path[len] = '/';
        len++;
      }
      strcpy(next_path + len, entry.name);

      // 4. Recurse
      vfs_debug_tree(next_path, level + 1);
    } else {
      // Print File
      printf("%s\n", entry.name);
    }
  }

  vfs_close(fd);
}

void run_vfs_test_suite() {
  vfs_stat_t st;
  char buffer[64];
  int fd;
  log_info("--- Starting VFS Integration Tests ---\n");

  // 1. Test mkdir & stat
  if (vfs_mkdir("/data") == 0) {
    if (vfs_stat("/data", &st) == 0 && st.type == VFS_DIRECTORY) {
      log_info("[PASS] mkdir and stat (directory)\n");
    }
  } else {
    log_error("[FAIL] mkdir /data\n");
  }

  // 2. Test open (O_CREAT) in a subdirectory
  fd = vfs_open("/data/hello.txt", O_READ | O_WRITE | O_CREAT);
  if (fd != VFS_INVALID_FD) {
    log_info("[PASS] open O_CREAT in subdirectory (FD: %d)\n", fd);

    // 3. Test write & fstat
    vfs_write(fd, "VFS_TEST this is a very long file just for testing ykyk", 50);
    if (vfs_fstat(fd, &st) == 0 && st.size == 8) {
      log_info("[PASS] write and fstat (size: %d)\n", st.size);
    }

    // 4. Test lseek & read
    vfs_lseek(fd, 4, 0); // SEEK_SET to "TEST"
    memset(buffer, 0, 64);
    int bytes = vfs_read(fd, buffer, 4);
    if (bytes == 4 && strcmp(buffer, "TEST") == 0) {
      log_info("[PASS] lseek and read (data: '%s')\n", buffer);
    }

    // 5. Test close
    if (vfs_close(fd) == 0) {
      log_info("[PASS] close\n");
    }
  } else {
    log_error("[FAIL] O_CREAT /data/hello.txt\n");
  }

  // 6. Test stat on the new file (Path Resolution check)
  if (vfs_stat("/data/hello.txt", &st) == 0) {
    log_info("[PASS] stat (file) size: %d, inode: %x\n", st.size, st.inode_num);
  } else {
    log_error("[FAIL] stat /data/hello.txt\n");
  }

  fd = vfs_open("/data/world.txt", O_READ | O_WRITE | O_CREAT);
  if (fd != VFS_INVALID_FD) {
    vfs_write(fd, "FOO", 3);
    vfs_close(fd);
    log_info("[PASS] Created second file /data/world.txt\n");
  }

  vfs_mkdir("/data/dir");
  fd = vfs_open("/data/dir/hello", O_CREAT);
  vfs_close(fd);

  log_info("Testing Directory Read on /data:\n");
  fd = vfs_open("/data", O_READ);
  if (fd != VFS_INVALID_FD) {
    vfs_dirent_t entry;
    int count = 0;

    // Loop until vfs_read returns 0 (EOF)
    // We pass &entry as the 'void* buf' and 1 as 'n' (1 entry)
    while (vfs_read(fd, &entry, 1) > 0) {
      log_info("  - Found: %s (Type: %s, Inode: %x)\n",
               entry.name,
               (entry.type == VFS_DIRECTORY ? "DIR" : "FILE"),
               entry.inode_num);
      count++;
    }

    if (count >= 2) {
      log_info("[PASS] Directory read returned %d entries\n", count);
    } else {
      log_error("[FAIL] Directory read only found %d entries\n", count);
    }

    vfs_close(fd);
  } else {
    log_error("[FAIL] Could not open directory /data for reading\n");
  }

  log_info("--- VFS Tests Complete ---\n");
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
  keyboard_set_scancode_set2();

  init_mm(&end);
  init_paging();

  // PIT is initialized by init_idt();

  log_info("Memory layout before malloc.\n");
  print_memory_layout();

  int* array = (int*)alloc_page();
  if (!array) {
    kernel_panic("Malloc failed!");
  }
  memset(array, 'a', sizeof(int) * 100);

  for (size_t i = 0; i < 100; i++) {
    printf("%c ", array[i]);
  }

  putchar('\n');
  log_info("Memory layout after malloc.\n");
  print_memory_layout();

  free_page(array);
  log_info("Memory layout after free.\n");
  print_memory_layout();

  // test memory
  // TODO: make this work with new MM
  init_syscalls();

  // initialize speaker
  PCSPK_init();

  log_set_min_level(3);
  vfs_init();
  ramfs_init("/");
  run_vfs_test_suite();
  vfs_debug_tree("/", 0);

  log_info("Starting shell...\n");

  shell_init();
  shell_run();
}
