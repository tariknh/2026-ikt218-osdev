#include <kernel/panic.h>
#include <stdint.h>
#include <string.h>

#include "arch/i386/cpu/isr.h"
#include "kernel/paging.h"
#include "kernel/pmm.h"
#include "kernel/util.h"

// We just place the user heap at a fixed place
#define USER_HEAP_START 0x10000UL
#define USER_HEAP_END_MAX 0x08000000UL // end

#define USER_HEAP_INVALID_START 0x00001000 // 4 KiB (guard / null region)

#define KERNEL_DIRECT_MAP_START 0x00100000 // 1 MiB

#define USER_FAULT_MAPPABLE_START 0x00000000 // 0 is intentional; we guard 0…0x40000000
#define USER_FAULT_MAPPABLE_END 0x40000000   // 1 GiB (user heap range)


static uint32_t user_heap_end = 0x08000000;


uint32_t get_user_heap_end(void) {
  return user_heap_end;
}

void set_user_heap_end(uint32_t addr) {
  addr = CLAMP(addr, USER_HEAP_START, USER_HEAP_END_MAX);
  user_heap_end = addr;
}

void page_fault_handler(registers_t* regs) {
  uint32_t fault_addr;
  __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));

  uint32_t err = regs->err_code;
  uint32_t present = (err >> 0) & 1;
  bool isPresent = present != 0;
  // uint32_t write = (err >> 1) & 1;
  uint32_t user = (err >> 2) & 1;

  if (!isPresent) {
    if (fault_addr < USER_HEAP_INVALID_START) {
      for (;;) {
        __asm__ volatile("hlt");
      }
    }
    // fault_addr is unsigned so it cant be less than START, so we dont check for it
    bool inDirectMap = fault_addr < KERNEL_DIRECT_MAP_END;
    if (!user && inDirectMap) {
      // Truncate address to the page boundary. clear lower 12 bits so page_start is a multiple of
      // PAGE_SIZE
      uint32_t page_start = fault_addr & PAGE_MASK;
      vmm_map_page(page_start, page_start, PAGE_KERNEL_RW);
      return;
    }

    // same reasoning as before
    bool isUserMappable = fault_addr < USER_FAULT_MAPPABLE_END;

    if (user && isUserMappable) {
      uint32_t page_start = fault_addr & PAGE_MASK;
      uint32_t phys = pmm_alloc_frame();
      // If allocated and nonnegative
      if (phys && phys >= KERNEL_DIRECT_MAP_START) {
        memset((void*)phys, 0, PAGE_SIZE);
        vmm_map_user_page(page_start, phys, PAGE_USER_RW);
        return;
      }
    }
  }

  kernel_panic("Unhandled page fault! addr=0x%x, eip=0x%x, err=0x%x\n", fault_addr, regs->eip,
               regs->err_code);
}
