# Memory Quick Handover

This folder contains the active kernel memory code.

## Files

- `heap.c`: heap allocator, `malloc`, `free`, `pmalloc`, `pfree`, `print_memory_layout`
- `paging.c`: paging setup and identity mapping
- `../../memory/heap.h`: public heap API
- `../../memory/paging.h`: public paging API
- `../libc/memory.c`: `memcpy`, `memset`, `memset16`

## Boot Flow

In `src/kernel.c`, memory starts in this order:

1. `init_kernel_memory(&end)`
2. `init_paging()`
3. `init_pit()`

`end` comes from the linker and marks the end of the kernel image.

## Heap Summary

- allocator type: simple first-fit
- normal allocations: `malloc` / `free`
- page-aligned allocations: `pmalloc` / `pfree`
- C++ also works because `operator new/delete` forward to the heap in `src/kernel.cpp`

## Paging Summary

- page directory address: `0x400000`
- first page table: `0x401000`
- current mapping: identity-mapped first 8 MB

## How To Check That Memory Works

- `meminfo` in the CLI prints memory layout
- `history` and `clearhistory` use heap allocation at runtime
- `src/kernel.cpp` runs a small `new` / `delete` test

## For Assignment 5

Memory is ready to use for the music player.

You can safely use:

- `malloc` / `free` in C
- `new` / `delete` in C++

Important rule:

- do not replace the existing PIT channel 0 timer
- use PIT channel 2 for PC speaker sound

Useful files for next step:

- `include/kernel/pit.h`
- `src/kernel/pit.c`
- `include/common.h`
- `src/common.c`
- `reference/assignment4/apps/song/src/song.cpp` as inspiration only
