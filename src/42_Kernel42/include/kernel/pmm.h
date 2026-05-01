#pragma once
#include <stdint.h>

#define PMM_BITMAP_PHYS 0x500000
#define PMM_BLOCK_SIZE 4096

/**
 * Physical memory manger info
 */
typedef struct {
  uint32_t total_frames; // total frames
  uint32_t free_frames;  // total free frames
  uint32_t bitmap_size;  // size of bitmap
  uint8_t* bitmap;       // bitmap array
} pmm_info_t;
/**
 *  Iniitialize the PMM,
 * @param mb_info
 */
void pmm_init(void* mb_info);

/**
 * Allocate a physical frame
 * @return
 */
uint32_t pmm_alloc_frame(void);
/**
 *  Free a physical frame
 * @param phys_addr
 */
void pmm_free_frame(uint32_t phys_addr);
/* Retreive free count*/
int pmm_get_free_count(void);
/***
 *Retrieve ctotal count*/
int pmm_get_total_count(void);

