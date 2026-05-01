#!/usr/bin/env bash
set -euo pipefail

# Assignment 4 patch for IKT218 OSDEV group folder.
# You can run this either from the repository root or from src/10_Falcons.

if [ -d "src/10_Falcons" ]; then
  GROUP_DIR="src/10_Falcons"
elif [ -f "CMakeLists.txt" ] && [ -d "src" ] && [ -d "include" ]; then
  GROUP_DIR="."
else
  echo "ERROR: Run from repo root (folder containing src/10_Falcons) or from src/10_Falcons." >&2
  echo "Current directory: $(pwd)" >&2
  exit 1
fi

mkdir -p "$GROUP_DIR/include/kernel" "$GROUP_DIR/src" "$GROUP_DIR/reports"

cat > "$GROUP_DIR/include/kernel/memory.h" <<'EOF'
#ifndef UIAOS_KERNEL_MEMORY_H
#define UIAOS_KERNEL_MEMORY_H

#include <stdint.h>
#include <stddef.h>

typedef struct alloc_header {
    uint32_t magic;
    uint32_t size;
    uint32_t free;
    struct alloc_header *next;
} alloc_header_t;

void init_kernel_memory(uint32_t *kernel_end);
void print_memory_layout(void);

void init_paging(void);
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);

void *malloc(size_t size);
void free(void *ptr);
char *pmalloc(size_t size);

void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *ptr, int value, size_t count);
void *memset16(void *ptr, uint16_t value, size_t count);

#endif
EOF

cat > "$GROUP_DIR/include/kernel/pit.h" <<'EOF'
#ifndef UIAOS_KERNEL_PIT_H
#define UIAOS_KERNEL_PIT_H

#include <stdint.h>

#define PIT_CMD_PORT       0x43
#define PIT_CHANNEL0_PORT  0x40
#define PIT_CHANNEL1_PORT  0x41
#define PIT_CHANNEL2_PORT  0x42
#define PC_SPEAKER_PORT    0x61

#define PIT_BASE_FREQUENCY 1193180u
#define TARGET_FREQUENCY   1000u
#define PIT_DIVIDER        (PIT_BASE_FREQUENCY / TARGET_FREQUENCY)
#define TICKS_PER_MS       1u

#define PIC1_COMMAND_PORT  0x20
#define PIC1_DATA_PORT     0x21
#define PIC2_COMMAND_PORT  0xA0
#define PIC2_DATA_PORT     0xA1
#define PIC_EOI            0x20

void init_pit(void);
void sleep_interrupt(uint32_t milliseconds);
void sleep_busy(uint32_t milliseconds);
uint32_t pit_get_ticks(void);

#endif
EOF

cat > "$GROUP_DIR/src/memory.c" <<'EOF'
#include <kernel/memory.h>
#include <kernel/terminal.h>

#define ALLOC_MAGIC 0xC0FFEE42u
#define HEAP_LIMIT  0x400000u      /* keep the simple heap below 4 MiB */
#define PAGE_SIZE   4096u
#define MAX_PAGE_ALIGNED_ALLOCS 64u

static alloc_header_t *heap_head = 0;
static uint32_t heap_start = 0;
static uint32_t heap_end = HEAP_LIMIT;
static uint32_t memory_used = 0;

static uint32_t pheap_start = 0;
static uint32_t pheap_end = 0;
static uint8_t pheap_bitmap[MAX_PAGE_ALIGNED_ALLOCS];

static uint32_t align_up(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1u) & ~(alignment - 1u);
}

void *memcpy(void *dest, const void *src, size_t count)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}

void *memset(void *ptr, int value, size_t count)
{
    uint8_t *p = (uint8_t *)ptr;
    for (size_t i = 0; i < count; i++) {
        p[i] = (uint8_t)value;
    }
    return ptr;
}

void *memset16(void *ptr, uint16_t value, size_t count)
{
    uint16_t *p = (uint16_t *)ptr;
    for (size_t i = 0; i < count; i++) {
        p[i] = value;
    }
    return ptr;
}

void init_kernel_memory(uint32_t *kernel_end)
{
    heap_start = align_up((uint32_t)kernel_end, 16u);

    /* Reserve the last part of the first 4 MiB for page-aligned allocations. */
    pheap_end = HEAP_LIMIT;
    pheap_start = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * PAGE_SIZE);
    heap_end = pheap_start;

    if (heap_start >= heap_end) {
        printf("[MEM] ERROR: kernel image is too large for the simple heap.\n");
        return;
    }

    heap_head = (alloc_header_t *)heap_start;
    heap_head->magic = ALLOC_MAGIC;
    heap_head->size = heap_end - heap_start - sizeof(alloc_header_t);
    heap_head->free = 1;
    heap_head->next = 0;

    memory_used = 0;
    memset(pheap_bitmap, 0, sizeof(pheap_bitmap));

    printf("[MEM] Kernel end: 0x%x\n", (uint32_t)kernel_end);
    printf("[MEM] Heap initialized: 0x%x - 0x%x\n", heap_start, heap_end);
}

static void split_block(alloc_header_t *block, uint32_t requested_size)
{
    uint32_t remaining = block->size - requested_size;
    if (remaining <= sizeof(alloc_header_t) + 16u) {
        return;
    }

    alloc_header_t *new_block = (alloc_header_t *)((uint8_t *)block + sizeof(alloc_header_t) + requested_size);
    new_block->magic = ALLOC_MAGIC;
    new_block->size = remaining - sizeof(alloc_header_t);
    new_block->free = 1;
    new_block->next = block->next;

    block->size = requested_size;
    block->next = new_block;
}

void *malloc(size_t size)
{
    if (size == 0 || heap_head == 0) {
        return 0;
    }

    uint32_t requested_size = align_up((uint32_t)size, 8u);
    alloc_header_t *current = heap_head;

    while (current) {
        if (current->magic != ALLOC_MAGIC) {
            printf("[MEM] Heap corruption detected at 0x%x\n", (uint32_t)current);
            return 0;
        }

        if (current->free && current->size >= requested_size) {
            split_block(current, requested_size);
            current->free = 0;
            memory_used += current->size;
            void *payload = (void *)((uint8_t *)current + sizeof(alloc_header_t));
            memset(payload, 0, current->size);
            return payload;
        }
        current = current->next;
    }

    printf("[MEM] malloc failed: %d bytes requested\n", (uint32_t)size);
    return 0;
}

static void coalesce_free_blocks(void)
{
    alloc_header_t *current = heap_head;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(alloc_header_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void free(void *ptr)
{
    if (!ptr) {
        return;
    }

    alloc_header_t *block = (alloc_header_t *)((uint8_t *)ptr - sizeof(alloc_header_t));
    if (block->magic != ALLOC_MAGIC) {
        printf("[MEM] free ignored invalid pointer 0x%x\n", (uint32_t)ptr);
        return;
    }

    if (!block->free) {
        block->free = 1;
        if (memory_used >= block->size) {
            memory_used -= block->size;
        }
        coalesce_free_blocks();
    }
}

char *pmalloc(size_t size)
{
    (void)size;
    for (uint32_t i = 0; i < MAX_PAGE_ALIGNED_ALLOCS; i++) {
        if (!pheap_bitmap[i]) {
            pheap_bitmap[i] = 1;
            uint32_t address = pheap_start + (i * PAGE_SIZE);
            memset((void *)address, 0, PAGE_SIZE);
            return (char *)address;
        }
    }
    printf("[MEM] pmalloc failed\n");
    return 0;
}

void print_memory_layout(void)
{
    uint32_t free_bytes = 0;
    alloc_header_t *current = heap_head;
    while (current) {
        if (current->free) {
            free_bytes += current->size;
        }
        current = current->next;
    }

    printf("[MEM] Layout:\n");
    printf("      heap start : 0x%x\n", heap_start);
    printf("      heap end   : 0x%x\n", heap_end);
    printf("      heap used  : %d bytes\n", memory_used);
    printf("      heap free  : %d bytes\n", free_bytes);
    printf("      pheap      : 0x%x - 0x%x\n", pheap_start, pheap_end);
}
EOF

cat > "$GROUP_DIR/src/paging.c" <<'EOF'
#include <kernel/memory.h>
#include <kernel/terminal.h>

#define PAGE_PRESENT 0x1u
#define PAGE_RW      0x2u
#define PAGE_SIZE    4096u

static uint32_t page_directory[1024] __attribute__((aligned(PAGE_SIZE)));
static uint32_t first_page_table[1024] __attribute__((aligned(PAGE_SIZE)));
static uint32_t second_page_table[1024] __attribute__((aligned(PAGE_SIZE)));

void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys)
{
    uint32_t directory_index = virt >> 22;
    uint32_t table_index = (virt >> 12) & 0x3FFu;
    uint32_t *table = 0;

    if (directory_index == 0) {
        table = first_page_table;
    } else if (directory_index == 1) {
        table = second_page_table;
    } else {
        return;
    }

    table[table_index] = (phys & 0xFFFFF000u) | PAGE_PRESENT | PAGE_RW;
    page_directory[directory_index] = ((uint32_t)table) | PAGE_PRESENT | PAGE_RW;
}

void init_paging(void)
{
    printf("[PAGING] Identity mapping first 8 MiB...\n");

    for (uint32_t i = 0; i < 1024; i++) {
        page_directory[i] = PAGE_RW;
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
        second_page_table[i] = ((1024u + i) * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
    }

    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW;
    page_directory[1] = ((uint32_t)second_page_table) | PAGE_PRESENT | PAGE_RW;

    __asm__ volatile ("mov %0, %%cr3" :: "r"((uint32_t)page_directory));

    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u;
    __asm__ volatile ("mov %0, %%cr0" :: "r"(cr0));

    printf("[PAGING] Paging enabled.\n");
}
EOF

cat > "$GROUP_DIR/src/pit.c" <<'EOF'
#include <kernel/pit.h>
#include <kernel/io.h>
#include <kernel/idt.h>
#include <kernel/terminal.h>

static volatile uint32_t pit_ticks = 0;

/* Compatibility with both common course skeleton styles. */
extern void register_interrupt_handler(uint8_t interrupt_number, isr_t handler) __attribute__((weak));
extern void irq_install_handler(int irq, isr_t handler) __attribute__((weak));

static void pit_callback(registers_t *regs)
{
    (void)regs;
    pit_ticks++;
}

uint32_t pit_get_ticks(void)
{
    return pit_ticks;
}

void init_pit(void)
{
    pit_ticks = 0;

    uint16_t divisor = (uint16_t)PIT_DIVIDER;
    outb(PIT_CMD_PORT, 0x36);                       /* channel 0, lobyte/hibyte, mode 3 */
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    if (register_interrupt_handler) {
        register_interrupt_handler(32, pit_callback); /* IRQ0 after PIC remap */
    } else if (irq_install_handler) {
        irq_install_handler(0, pit_callback);         /* older skeleton API */
    }

    /* Unmask IRQ0 on the master PIC. */
    uint8_t mask = inb(PIC1_DATA_PORT);
    outb(PIC1_DATA_PORT, mask & (uint8_t)~0x01u);

    printf("[PIT] Initialized at %d Hz.\n", TARGET_FREQUENCY);
}

void sleep_busy(uint32_t milliseconds)
{
    uint32_t start = pit_get_ticks();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    while ((pit_get_ticks() - start) < ticks_to_wait) {
        /* busy wait */
    }
}

void sleep_interrupt(uint32_t milliseconds)
{
    uint32_t end_tick = pit_get_ticks() + (milliseconds * TICKS_PER_MS);
    while ((int32_t)(end_tick - pit_get_ticks()) > 0) {
        __asm__ volatile ("sti; hlt");
    }
}
EOF

cat > "$GROUP_DIR/reports/assignment_4_memory_and_pit.md" <<'EOF'
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
EOF

# Patch kernel.c safely: add includes, extern end, and an Assignment 4 test block.
KERNEL_C="$GROUP_DIR/src/kernel.c"
if [ ! -f "$KERNEL_C" ]; then
  echo "ERROR: Cannot find $KERNEL_C" >&2
  exit 1
fi

# Add required includes and the linker symbol if they are not already there.
if ! grep -q "kernel/memory.h" "$KERNEL_C"; then
  tmp="$(mktemp)"
  {
    echo '#include <stdint.h>'
    echo '#include <kernel/memory.h>'
    echo '#include <kernel/pit.h>'
    echo ''
    cat "$KERNEL_C"
  } > "$tmp"
  mv "$tmp" "$KERNEL_C"
elif ! grep -q "kernel/pit.h" "$KERNEL_C"; then
  tmp="$(mktemp)"
  {
    echo '#include <kernel/pit.h>'
    cat "$KERNEL_C"
  } > "$tmp"
  mv "$tmp" "$KERNEL_C"
fi

if ! grep -q "extern uint32_t end;" "$KERNEL_C"; then
  tmp="$(mktemp)"
  awk '
    BEGIN { inserted = 0 }
    /^#include/ { print; next }
    inserted == 0 { print ""; print "extern uint32_t end;"; inserted = 1 }
    { print }
  ' "$KERNEL_C" > "$tmp"
  mv "$tmp" "$KERNEL_C"
fi

ASSIGNMENT4_BLOCK="$(mktemp)"
cat > "$ASSIGNMENT4_BLOCK" <<'EOF'

    /* Assignment 4: memory manager, paging, malloc/free and PIT. */
    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();

    void *mem1 = malloc(12345);
    void *mem2 = malloc(54321);
    void *mem3 = malloc(13331);
    printf("[MEM] malloc test: mem1=0x%x mem2=0x%x mem3=0x%x\n", (uint32_t)mem1, (uint32_t)mem2, (uint32_t)mem3);
    free(mem2);
    void *mem4 = malloc(1000);
    printf("[MEM] free/reuse test: mem4=0x%x\n", (uint32_t)mem4);

    init_pit();
    printf("[PIT] Testing busy sleep 250 ms...\n");
    sleep_busy(250);
    printf("[PIT] Busy sleep OK. Testing interrupt sleep 250 ms...\n");
    sleep_interrupt(250);
    printf("[PIT] Interrupt sleep OK.\n");
EOF

if ! grep -q "Assignment 4: memory manager" "$KERNEL_C"; then
  tmp="$(mktemp)"
  awk -v blockfile="$ASSIGNMENT4_BLOCK" '
    function print_block() {
      while ((getline line < blockfile) > 0) print line;
      close(blockfile);
    }
    BEGIN { inserted = 0 }
    inserted == 0 && $0 ~ /Keyboard ready\. Type something/ { print_block(); inserted = 1 }
    inserted == 0 && $0 ~ /Hardware interrupts enabled/ { print_block(); inserted = 1 }
    inserted == 0 && $0 ~ /__asm__.*volatile.*"sti"/ { print_block(); inserted = 1 }
    inserted == 0 && $0 ~ /asm.*volatile.*"sti"/ { print_block(); inserted = 1 }
    inserted == 0 && $0 ~ /while[[:space:]]*\([[:space:]]*(1|true)[[:space:]]*\)/ { print_block(); inserted = 1 }
    { print }
    END { if (inserted == 0) print "WARNING: Could not automatically insert Assignment 4 block into kernel.c" > "/dev/stderr" }
  ' "$KERNEL_C" > "$tmp"
  mv "$tmp" "$KERNEL_C"
fi
rm -f "$ASSIGNMENT4_BLOCK"

# CMake: make sure new C files are part of the kernel target when the project does not use GLOB_RECURSE.
CMAKE="$GROUP_DIR/CMakeLists.txt"
if [ -f "$CMAKE" ] && ! grep -q "src/pit.c" "$CMAKE"; then
  cat >> "$CMAKE" <<'EOF'

# Assignment 4 sources: memory manager, paging and PIT.
target_sources(uiaos-kernel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/memory.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/paging.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pit.c
)
EOF
fi

cat <<'EOF'
Done: Assignment 4 files were added.

Now test from src/10_Falcons:
  rm -rf build
  cmake -S . -B build
  cmake --build build
  cmake --build build --target uiaos-create-image
  qemu-system-i386 -cdrom build/kernel.iso

Expected QEMU output should include:
  [MEM] Heap initialized
  [PAGING] Paging enabled
  [MEM] malloc test
  [PIT] Initialized at 1000 Hz
  [PIT] Interrupt sleep OK

After it works:
  git status
  git add .
  git commit -m "Implement assignment 4 memory management and PIT"
  git push origin master
EOF
