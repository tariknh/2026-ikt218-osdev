#include "heap.h"



static struct heap_block* heap_head = NULL;
static uint32_t kernel_end_address = 0U;
static uint32_t heap_start_address = 0U;
static uint32_t heap_end_address = 0U;

static size_t align_up(size_t value) {
    return (value + (HEAP_ALIGNMENT - 1U)) & ~(HEAP_ALIGNMENT - 1U);
}

static void split_block(struct heap_block* block, size_t requested_size) {
    size_t minimum_block_size = requested_size + sizeof(struct heap_block) + HEAP_ALIGNMENT;

    if (block->size < minimum_block_size) {
        return;
    }

    struct heap_block* new_block = (struct heap_block*)((uint8_t*)(block + 1) + requested_size);
    new_block->size = block->size - requested_size - sizeof(struct heap_block);
    new_block->is_free = true;
    new_block->next = block->next;

    block->size = requested_size;
    block->next = new_block;
}

static void coalesce_free_blocks(void) {
    struct heap_block* current = heap_head;

    while (current != NULL && current->next != NULL) {
        struct heap_block* next = current->next;
        uint8_t* current_end = (uint8_t*)(current + 1) + current->size;

        /* Merge only blocks that are both free and physically adjacent. */
        if (current->is_free && next->is_free && current_end == (uint8_t*)next) {
            current->size += sizeof(struct heap_block) + next->size;
            current->next = next->next;
            continue;
        }

        current = current->next;
    }
}

static void append_string(char* buffer, size_t* offset, const char* text) {
    while (*text != '\0') {
        buffer[*offset] = *text;
        ++(*offset);
        ++text;
    }
}

static void append_hex32(char* buffer, size_t* offset, uint32_t value) {
    static const char digits[] = "0123456789ABCDEF";
    int shift = 28;

    append_string(buffer, offset, "0x");

    while (shift >= 0) {
        buffer[*offset] = digits[(value >> shift) & 0xFU];
        ++(*offset);
        shift -= 4;
    }
}

void init_kernel_memory(uint32_t* kernel_end) {
    /* Keep the first block aligned so returned allocations stay aligned too. */
    kernel_end_address = (uint32_t)kernel_end;
    heap_start_address = (uint32_t)align_up((size_t)kernel_end);
    heap_end_address = heap_start_address + HEAP_SIZE_BYTES;

    heap_head = (struct heap_block*)heap_start_address;
    heap_head->size = HEAP_SIZE_BYTES - sizeof(struct heap_block);
    heap_head->is_free = true;
    heap_head->next = NULL;
}

char* pmalloc(size_t size) {
    uint32_t raw_address;
    uint32_t aligned_address;
    void* raw_block;

    if (size == 0U) {
        return NULL;
    }

    raw_block = malloc(size + PAGE_SIZE_BYTES + sizeof(uint32_t));
    if (raw_block == NULL) {
        return NULL;
    }

    raw_address = (uint32_t)raw_block + sizeof(uint32_t);
    aligned_address = (uint32_t)align_up((size_t)raw_address);

    if ((aligned_address & (PAGE_SIZE_BYTES - 1U)) != 0U) {
        aligned_address = (aligned_address + PAGE_SIZE_BYTES - 1U) & ~(PAGE_SIZE_BYTES - 1U);
    }

    ((uint32_t*)aligned_address)[-1] = (uint32_t)raw_block;
    return (char*)aligned_address;
}

void* malloc(size_t size) {
    struct heap_block* current;
    size_t requested_size;

    if (size == 0U || heap_head == NULL) {
        return NULL;
    }

    requested_size = align_up(size);
    current = heap_head;

    while (current != NULL) {
        if (current->is_free && current->size >= requested_size) {
            split_block(current, requested_size);
            current->is_free = false;
            return (void*)(current + 1);
        }

        current = current->next;
    }

    return NULL;
}

void free(void* ptr) {
    struct heap_block* block;

    if (ptr == NULL) {
        return;
    }

    block = ((struct heap_block*)ptr) - 1;
    block->is_free = true;
    coalesce_free_blocks();
}

void print_memory_layout(void) {
    char buffer[80];
    size_t offset = 0U;

    append_string(buffer, &offset, "KEND ");
    append_hex32(buffer, &offset, kernel_end_address);
    append_string(buffer, &offset, " HST ");
    append_hex32(buffer, &offset, heap_start_address);
    append_string(buffer, &offset, " HEND ");
    append_hex32(buffer, &offset, heap_end_address);
    buffer[offset] = '\0';

    write_text_at(2U, 0U, "                                                                                ", VgaColor(vga_black, vga_black));
    write_text_at(3U, 0U, "                                                                                ", VgaColor(vga_black, vga_black));
    write_text_at(2U, 0U, "Memory layout", VgaColor(vga_black, vga_light_green));
    write_text_at(3U, 0U, buffer, VgaColor(vga_black, vga_white));
}

MemoryDebugData get_memory_layout(void) {
    MemoryDebugData data;
    size_t offset = 0U;

    data.kernel_end = kernel_end_address;
    data.heap_start = heap_start_address;
    data.heap_end = heap_end_address;

    append_string(data.formatted, &offset, " KEND: ");
    append_hex32(data.formatted, &offset, data.kernel_end);
    append_string(data.formatted, &offset, "\n HST:  ");
    append_hex32(data.formatted, &offset, data.heap_start);
    append_string(data.formatted, &offset, "\n HEND: ");
    append_hex32(data.formatted, &offset, data.heap_end);
    
    data.formatted[offset] = '\n';
    data.formatted[offset+1] = '\n';
    data.formatted[offset+2] = '\0';

    return data;
}

HeapInformation get_heap_stats(void) {
    HeapInformation stats = {0};
    struct heap_block* current = heap_head;

    size_t free_block_size_sum = 0;
    size_t used_block_size_sum = 0;

    // initialize min values
    stats.smallest_free_block = (size_t)-1;
    stats.smallest_used_block = (size_t)-1;

    while (current != NULL) {
        size_t payload = current->size;
        size_t total = payload + sizeof(struct heap_block);

        stats.total_blocks++;
        stats.total_payload_bytes += payload;
        stats.total_bytes_with_meta += total;

        if (current->is_free) {
            stats.free_blocks++;
            stats.free_bytes += payload;
            stats.free_bytes_with_meta += total;

            free_block_size_sum += payload;

            if (payload > stats.largest_free_block) {
                stats.largest_free_block = payload;
            }
            if (payload < stats.smallest_free_block) {
                stats.smallest_free_block = payload;
            }

        } else {
            stats.used_blocks++;
            stats.used_bytes += payload;
            stats.used_bytes_with_meta += total;

            used_block_size_sum += payload;

            if (payload > stats.largest_used_block) {
                stats.largest_used_block = payload;
            }
            if (payload < stats.smallest_used_block) {
                stats.smallest_used_block = payload;
            }
        }

        current = current->next;
    }

    // fix edge cases (no blocks of a type)
    if (stats.free_blocks == 0) {
        stats.smallest_free_block = 0;
    }
    if (stats.used_blocks == 0) {
        stats.smallest_used_block = 0;
    }

    // averages
    if (stats.free_blocks > 0) {
        stats.avg_free_block_size = free_block_size_sum / stats.free_blocks;
    }
    if (stats.used_blocks > 0) {
        stats.avg_used_block_size = used_block_size_sum / stats.used_blocks;
    }

    // fragmentation metric: (1 - largest_free / total_free) * 1000
    if (stats.free_bytes > 0 && stats.largest_free_block > 0) {
        stats.fragmentation_per_mille = (size_t)((1000U * (stats.free_bytes - stats.largest_free_block)) / stats.free_bytes);
    } else {
        stats.fragmentation_per_mille = 0;
    }

    return stats;
}
