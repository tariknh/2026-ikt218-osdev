# Assignment 4 - Memory and PIT

## Introduction

This section extends the kernel with a small memory manager and a Programmable Interval Timer (PIT) driver. The memory manager gives the kernel controlled dynamic allocation, while the PIT gives the kernel a periodic hardware timer that can be used for time measurement and sleep functions.

## Memory Management

The kernel uses the `end` symbol from the linker script as the first safe address after the loaded kernel image. In `kernel.c`, `extern uint32_t end;` is used so the C code can pass this address to `init_kernel_memory(&end)`.

The heap is placed after the kernel image and below the 4 MiB mark. Each allocation has a small header containing a magic value, the block size, a free/used flag, and a pointer to the next block. `malloc()` searches for a free block large enough for the requested allocation, splits it when possible, marks it used, clears the returned memory, and returns a pointer to the usable payload. `free()` marks a block as free and merges neighbouring free blocks.

A small page-aligned allocator, `pmalloc()`, is also included. It reserves page-sized blocks near the top of the first 4 MiB and returns 4096-byte aligned addresses. This is useful for paging structures and later kernel features.

The paging implementation identity maps the first 8 MiB of memory. Identity mapping means that virtual address `x` maps to physical address `x`. This is simple and appropriate at this stage because the kernel, VGA memory, heap, and early paging structures remain accessible after paging is enabled.

```text
Kernel image -> heap start ........ heap end -> page-aligned heap -> 4 MiB
```

## Programmable Interval Timer

The PIT driver programs channel 0 of the 8253/8254 timer. The input frequency is 1,193,180 Hz, and the driver uses a divisor that produces approximately 1000 timer interrupts per second. This gives one PIT tick per millisecond.

IRQ0 is connected to the PIT. After PIC remapping from the interrupt assignment, IRQ0 appears as interrupt vector 32. The PIT driver registers a handler for vector 32. Every time the timer interrupt fires, the handler increments a global tick counter.

Two sleep functions are implemented:

- `sleep_busy(milliseconds)` loops until enough timer ticks have passed. This works, but it wastes CPU time.
- `sleep_interrupt(milliseconds)` enables interrupts and executes `hlt`, allowing the CPU to sleep until the next interrupt. This is more efficient.

## Test

The kernel initializes the memory manager, enables paging, prints the memory layout, allocates and frees test blocks with `malloc()` and `free()`, initializes the PIT, and tests both sleep methods. If the kernel reaches the keyboard prompt after printing the memory and PIT messages, Assignment 4 is working.

## Conclusion

Assignment 4 adds two important operating system foundations: dynamic memory allocation and timer-based sleeping. The memory manager allows the kernel to request memory at runtime, and the PIT gives the kernel a hardware time source needed for scheduling, delays, and future multitasking.

## References

[1] OSDev Wiki, "Memory Allocation". Available: https://wiki.osdev.org/Memory_Allocation

[2] OSDev Wiki, "Paging". Available: https://wiki.osdev.org/Paging

[3] OSDev Wiki, "Programmable Interval Timer". Available: https://wiki.osdev.org/Programmable_Interval_Timer
