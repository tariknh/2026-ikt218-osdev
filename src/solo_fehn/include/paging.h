/*
 * paging.h - Basic paging
 *
 * For this assignment we set up the simplest possible paging:
 *   - one page directory with one entry pointing at one page table
 *   - that page table identity-maps the first 4 MiB of physical memory
 *     (each page is 4 KiB, the table has 1024 entries, 1024 * 4 KiB = 4 MiB)
 *   - load the directory into CR3 and set the PG bit in CR0 to enable paging
 *
 * The kernel and the heap both live well below 4 MiB, so identity mapping
 * is enough to keep everything addressable after paging is enabled.
 */

#pragma once

void init_paging(void);
