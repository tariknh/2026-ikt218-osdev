Kernel Heap Memory Module
=========================
Overview
--------
We added a simple kernel heap allocator in the memory folder so the kernel can dynamically allocate and free memory without paging.
Files:
- src/group_29/src/memory/heap.h
- src/group_29/src/memory/heap.c
This allocator is meant to solve the assignment focus right now:
- initialize kernel memory using &end
- support malloc()
- support free()
We are not using paging yet. The heap is just a fixed memory region placed directly after the kernel image in memory.
How the heap starts
-------------------
The linker script defines a symbol called end in:
- src/group_29/src/arch/i386/linker.ld:36
That symbol marks the end of the loaded kernel in memory.
In kernel.c, we declare:
- extern uint32_t end;
Then we call:
- init_kernel_memory(&end);
This means:
- take the address right after the kernel
- align it to a safe boundary
- use that address as the start of the heap
In our implementation, the heap then reserves a fixed-size region of 1 MiB.
Public API
----------
Declared in src/group_29/src/memory/heap.h:
- void init_kernel_memory(void* kernel_end);
- void* malloc(size_t size);
- void free(void* ptr);
- void print_memory_layout(void);
What each function does:
1. init_kernel_memory(void* kernel_end)
- called once during kernel startup
- sets up the heap start and heap end
- creates the first free block covering almost the whole heap
2. malloc(size_t size)
- allocates a chunk of memory from the heap
- returns a pointer to usable memory
- returns NULL if allocation fails or size == 0
3. free(void* ptr)
- marks a previously allocated block as free
- merges neighboring free blocks to reduce fragmentation
- does nothing if ptr == NULL
4. print_memory_layout(void)
- prints debug information to VGA
- shows:
  - kernel end
  - heap start
  - heap end
Heap structure
--------------
The allocator uses a linked list of heap blocks.
The metadata struct is declared in heap.h:
- size = usable bytes in the block
- is_free = whether the block is available
- next = pointer to the next block
Conceptually, memory looks like this:
block headerblock headerblock header
Every allocation has a hidden header just before the memory returned to the caller.
Example:
- malloc(128) may internally reserve:
  - one block header
  - then 128 usable bytes
The pointer returned to the caller points only to the usable bytes, not the header.
Initialization flow
-------------------
Inside src/group_29/src/memory/heap.c, init_kernel_memory() does this:
1. store the original kernel end address for debugging
2. align the start address upward
3. define heap end as:
   - heap_start + HEAP_SIZE_BYTES
4. create the first heap block at heap_start
5. mark that first block as:
   - free
   - size = heap size minus one block header
   - next = NULL
So at startup, the heap is one large free block.
Alignment
---------
The allocator uses:
- HEAP_ALIGNMENT = 8
This means requested sizes are rounded up to multiples of 8.
Why this matters:
- improves pointer alignment
- avoids badly aligned returned addresses
- makes block layout cleaner
Example:
- malloc(5) becomes 8 bytes
- malloc(13) becomes 16 bytes
How malloc() works
--------------------
The allocator uses a first-fit strategy.
Steps:
1. reject size 0
2. align the requested size
3. walk the linked list from the first block
4. find the first free block large enough
5. if the block is much larger than needed, split it
6. mark the chosen block as used
7. return the address right after its header
First-fit means:
- the allocator takes the first suitable free block it finds
- it does not search for the “best” size match
This is simple and good enough for an early kernel heap.
Block splitting
---------------
If a free block is larger than needed, the allocator splits it into:
- one allocated block of requested size
- one new free block containing the remaining space
Example:
Before:
- one free block of 1000 bytes
Request:
- malloc(128)
After:
- allocated block of 128 bytes
- new free block with the leftover space
Why split:
- avoids wasting large blocks on small allocations
- preserves remaining heap space for future allocations
How free() works
------------------
When free(ptr) is called:
1. if ptr == NULL, return immediately
2. move backward from ptr to find the block header
3. mark that block as free
4. attempt to merge adjacent free blocks
This does not erase memory contents.
It only marks the block reusable.
Coalescing
----------
After freeing a block, the allocator tries to merge neighboring free blocks.
This is called coalescing.
Why it matters:
- prevents fragmentation
- creates larger free blocks again
Example:
Before:
- free block 128 bytes
- free block 256 bytes
- and they are directly next to each other in memory
After coalescing:
- one free block of combined size
In the code, merging only happens if:
- both blocks are free
- and they are physically adjacent in memory
This is important because blocks that are only neighbors in the linked list but not adjacent in memory must not be merged.
Debug output
------------
print_memory_layout() writes directly to VGA text memory and shows:
- KEND = original kernel end address
- HST = heap start
- HEND = heap end
This helps verify:
- the heap starts after the kernel
- the heap region is initialized correctly
How it is used in the kernel
----------------------------
In src/group_29/src/kernel.c, the current flow is:
1. initialize GDT
2. initialize IDT
3. call init_kernel_memory(&end)
4. call print_memory_layout()
5. test allocations with malloc()
6. test reuse with free()
Current test sequence:
- allocate 128 bytes
- allocate 256 bytes
- free the first block
- allocate 64 bytes
If that works, the kernel prints:
- Heap allocations OK
- free() reuse OK
How to use the allocator
------------------------
Typical usage:
void* ptr = malloc(128);
if (ptr != NULL) {
    /* use memory here */
}
free(ptr);
Rules:
- only free pointers returned by malloc()
- do not free the same pointer twice
- do not use memory after it has been freed
- check for NULL when allocation may fail
Limitations of the current design
---------------------------------
This is a minimal early kernel allocator, so it has some limitations:
1. Fixed heap size
- heap size is currently hardcoded to 1 MiB
- it does not grow dynamically
2. No paging
- memory is assumed to be directly usable after the kernel
- there is no virtual memory management yet
3. No safety checks
- no protection against double free
- no protection against invalid pointers passed to free()
4. No advanced allocation strategy
- first-fit is simple, not optimal
5. No zero-initialization
- malloc() does not clear memory
Why this is still good enough
-----------------------------
For this assignment stage, it is enough because:
- it demonstrates dynamic allocation
- it demonstrates reuse of freed memory
- it is simple to understand and debug
- it is a clean base for later improvements
Possible future improvements
----------------------------
Later, we can extend this design with:
- paging-backed heap growth
- calloc()
- realloc()
- pointer validation in free()
- double-free detection
- better debug output
- overloaded C++ new and delete
Summary
-------
The memory module works by:
- starting the heap right after the kernel image using &end
- managing heap space as a linked list of blocks
- allocating memory with first-fit search
- splitting large free blocks during allocation
- marking blocks free on free()
- merging adjacent free blocks to reduce fragmentation
This gives us a working kernel heap with malloc() and free() without needing paging yet.
If you want, I can also turn this into a shorter “team handoff” version and a separate “technical deep dive” version.
▣  Plan · gpt-5.4 · 35.5s