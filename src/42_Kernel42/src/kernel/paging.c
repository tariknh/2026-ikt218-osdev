#include "kernel/paging.h"

#include <kernel/log.h>
#include <kernel/pmm.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PAGE_TABLE_ENTRIES 1024

static uint32_t* page_directory = NULL;
static uint32_t* page_dir_loc = NULL;

extern uint32_t _page_dir;
extern uint32_t _page_tables;

static uint32_t* get_or_create_pt(uint32_t virt) {
  uint32_t pgd_idx = (virt >> 22) & 0x3FF;
  if (page_directory[pgd_idx] & PAGE_PRESENT) {
    return (uint32_t*)(page_directory[pgd_idx] & ~0xFFF);
  }
  uint32_t pt_phys = pmm_alloc_frame();
  if (!pt_phys) {
    printf("get_or_create_pt: failed to alloc PT for PD[%u]\n", pgd_idx);
    return NULL;
  }
  memset((void*)pt_phys, 0, PAGE_SIZE);
  page_directory[pgd_idx] = pt_phys | PAGE_PRESENT | PAGE_RW;
  return (uint32_t*)pt_phys;
}

void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys) {
  uint32_t* pt = get_or_create_pt(virt);
  if (!pt)
    return;
  uint32_t pte_idx = (virt >> 12) & 0x3FF;
  pt[pte_idx] = phys | PAGE_PRESENT | PAGE_RW;
  invalidate_page(virt);
}

uint32_t paging_get_phys(uint32_t virt) {
  uint32_t pgd_idx = (virt >> 22) & 0x3FF;
  uint32_t pte_idx = (virt >> 12) & 0x3FF;
  if (!(page_directory[pgd_idx] & PAGE_PRESENT)) {
    return 0;
  }
  uint32_t* pt = (uint32_t*)(page_directory[pgd_idx] & ~0xFFF);
  if (!(pt[pte_idx] & PAGE_PRESENT)) {
    return 0;
  }
  return (pt[pte_idx] & ~0xFFF) + (virt & 0xFFF);
}

int vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
  uint32_t* pt = get_or_create_pt(virt);
  if (!pt)
    return -1;
  uint32_t pte_idx = (virt >> 12) & 0x3FF;
  pt[pte_idx] = phys | flags;
  invalidate_page(virt);
  return 0;
}

int vmm_unmap_page(uint32_t virt) {
  uint32_t pgd_idx = (virt >> 22) & 0x3FF;
  uint32_t pte_idx = (virt >> 12) & 0x3FF;
  if (!(page_directory[pgd_idx] & PAGE_PRESENT)) {
    return 0;
  }
  uint32_t* pt = (uint32_t*)(page_directory[pgd_idx] & ~0xFFF);
  pt[pte_idx] = 0;
  invalidate_page(virt);
  return 0;
}

int vmm_map_user_page(uint32_t virt, uint32_t phys, uint32_t flags) {
  uint32_t pgd_idx = (virt >> 22) & 0x3FF;
  if (!(page_directory[pgd_idx] & PAGE_PRESENT)) {
    uint32_t pt_phys = pmm_alloc_frame();
    if (!pt_phys) {
      printf("vmm_map_user_page: failed to alloc PT for PD[%u]\n", pgd_idx);
      return -1;
    }
    memset((void*)pt_phys, 0, PAGE_SIZE);
    page_directory[pgd_idx] = pt_phys | PAGE_PRESENT | PAGE_RW | PAGE_USER;
  }
  uint32_t* pt = (uint32_t*)(page_directory[pgd_idx] & ~0xFFF);
  uint32_t pte_idx = (virt >> 12) & 0x3FF;
  pt[pte_idx] = phys | flags | PAGE_USER;
  invalidate_page(virt);
  return 0;
}

void init_paging(void* mb_info) {
  (void)mb_info;

  page_directory = &_page_dir;
  page_dir_loc = page_directory;

  for (int i = 0; i < 1024; i++) {
    page_directory[i] = 0 | 2;
  }

  uint32_t* first_pt = &_page_tables;
  for (int i = 0; i < 1024; i++) {
    first_pt[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
  }
  page_directory[0] = ((uint32_t)first_pt) | PAGE_PRESENT | PAGE_RW;

  first_pt[0x104] = 0x104000 | PAGE_PRESENT | PAGE_RW;
  first_pt[0x108] = 0x108000 | PAGE_PRESENT | PAGE_RW;
  first_pt[0x109] = 0x109000 | PAGE_PRESENT | PAGE_RW;

  uint32_t* second_pt = &_page_tables + 1024;
  for (int i = 0; i < 1024; i++) {
    second_pt[i] = (0x400000 + i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
  }
  page_directory[1] = ((uint32_t)second_pt) | PAGE_PRESENT | PAGE_RW;

  uint32_t user_pt = pmm_alloc_frame();
  if (user_pt) {
    memset((void*)user_pt, 0, PAGE_SIZE);
    page_directory[32] = user_pt | PAGE_PRESENT | PAGE_RW | PAGE_USER;
  }

  load_cr3((uint32_t)page_dir_loc);
  enable_paging();

  log_info("Paging enabled successfully!\n");
}
