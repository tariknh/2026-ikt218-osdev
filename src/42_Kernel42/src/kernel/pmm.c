#include "kernel/pmm.h"

#include <kernel/log.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


// Physical Memory Manager (PMM) with a bitmap.
// Manages 4 KiB frames in a fixed region.

#define PMM_PGD_BASE       0x100000      // physical base of page directory
#define PMM_PGDIR_PHYS     0x104000      //  pgdir 0x00104000
#define PMM_PGTABLES_PHYS  0x108000      //  pgtables 0x00108000

#define PMM_PD_FRAMES_TO_MARK 5
#define PMM_PGTABLES_FRAMES_TO_MARK 2

// defined in linker.ld
extern uint32_t end;
static pmm_info_t pmm_info;

#define PMM_MAX_MEM (128 * 1024 * 1024)

// mark kernel code/data, bitmap, page directory, and page tables as used
void pmm_init(void* mb_info) {
  (void)mb_info;

  uint32_t max_mem = PMM_MAX_MEM;
  uint32_t num_frames = max_mem / PMM_BLOCK_SIZE;

  pmm_info.total_frames = num_frames;
  pmm_info.bitmap_size = (num_frames + 7) / 8;
  pmm_info.bitmap = (uint8_t*)PMM_BITMAP_PHYS;

  memset(pmm_info.bitmap, 0, pmm_info.bitmap_size);

  uint32_t kernel_end_frame = ((uint32_t)&end + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
  log_info("PMM: kernel ends at 0x%x, frame %u\n", (uint32_t)&end, kernel_end_frame);

  uint32_t used = 0;

  // mark kernel code/data as used (up to &end)
  for (uint32_t f = 0; f < kernel_end_frame; f++) {
    pmm_info.bitmap[f / 8] |= (1 << (f % 8));
    used++;
  }

  // mark PMM bitmap as used
  uint32_t pmm_bitmap_frame = PMM_BITMAP_PHYS / PMM_BLOCK_SIZE;
  uint32_t pmm_bitmap_frames = (pmm_info.bitmap_size + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
  for (uint32_t f = pmm_bitmap_frame; f < pmm_bitmap_frame + pmm_bitmap_frames; f++) {
    if (f < num_frames) {
      pmm_info.bitmap[f / 8] |= (1 << (f % 8));
      used++;
    }
  }

  // mark page directory and related page tables as used
  uint32_t pd_frame = PMM_PGD_BASE / PMM_BLOCK_SIZE;
  for (uint32_t f = pd_frame; f < pd_frame + PMM_PD_FRAMES_TO_MARK; f++) {
    if (f < num_frames) {
      pmm_info.bitmap[f / 8] |= (1 << (f % 8));
      used++;
    }
  }

  uint32_t pgdir_frame = PMM_PGDIR_PHYS / PMM_BLOCK_SIZE;
  uint32_t pgtables_frame = PMM_PGTABLES_PHYS / PMM_BLOCK_SIZE;
  if (pgdir_frame < num_frames) {
    pmm_info.bitmap[pgdir_frame / 8] |= (1 << (pgdir_frame % 8));
    used++;
  }
  for (uint32_t f = pgtables_frame; f < pgtables_frame + PMM_PGTABLES_FRAMES_TO_MARK; f++) {
    if (f < num_frames) {
      pmm_info.bitmap[f / 8] |= (1 << (f % 8));
      used++;
    }
  }

  pmm_info.free_frames = num_frames - used;

  log_info("PMM: %u frames total (%u KB)\n", num_frames, num_frames * 4);
  log_info("PMM: %d used frames, %d free frames (%d KB)\n", used, pmm_info.free_frames,
           pmm_info.free_frames * 4);
}

// allocate a 4KiB a physical frame by finding the first zero bit in bitmap and marking it as used
uint32_t pmm_alloc_frame(void) {
  for (uint32_t i = 0; i < pmm_info.total_frames; i++) {
    if (!(pmm_info.bitmap[i / 8] & (1 << (i % 8)))) {
      pmm_info.bitmap[i / 8] |= (1 << (i % 8));
      pmm_info.free_frames--;
      return i * PMM_BLOCK_SIZE;
    }
  }
  log_info("PMM: OUT OF MEMORY!\n");
  return 0;
}

// free a 4KiB physical frame by marking its bit in the bitmap as unused.
void pmm_free_frame(uint32_t phys_addr) {
  uint32_t frame = phys_addr / PMM_BLOCK_SIZE;
  if (frame >= pmm_info.total_frames) {
    return;
  }
  if (pmm_info.bitmap[frame / 8] & (1 << (frame % 8))) {
    pmm_info.bitmap[frame / 8] &= ~(1 << (frame % 8));
    pmm_info.free_frames++;
  }
}

int pmm_get_free_count(void) {
  return pmm_info.free_frames;
}

int pmm_get_total_count(void) {
  return pmm_info.total_frames;
}
