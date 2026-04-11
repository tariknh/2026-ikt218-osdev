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

static void write_text_at(uint8_t row, uint8_t col, const char* text, uint8_t color) {
    uint16_t* cursor = VGA_MEMORY + (row * VGA_WIDTH) + col;

    while (*text != '\0' && col < VGA_WIDTH) {
        *cursor = (uint16_t)((color << 8) | (uint8_t)*text);
        ++cursor;
        ++col;
        ++text;
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

void init_kernel_memory(void* kernel_end) {
    /* Keep the first block aligned so returned allocations stay aligned too. */
    kernel_end_address = (uint32_t)kernel_end;
    heap_start_address = (uint32_t)align_up((size_t)kernel_end);
    heap_end_address = heap_start_address + HEAP_SIZE_BYTES;

    heap_head = (struct heap_block*)heap_start_address;
    heap_head->size = HEAP_SIZE_BYTES - sizeof(struct heap_block);
    heap_head->is_free = true;
    heap_head->next = NULL;
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

    append_string(data.formatted, &offset, "KEND: ");
    append_hex32(data.formatted, &offset, data.kernel_end);
    append_string(data.formatted, &offset, "\nHST:  ");
    append_hex32(data.formatted, &offset, data.heap_start);
    append_string(data.formatted, &offset, "\nHEND: ");
    append_hex32(data.formatted, &offset, data.heap_end);
    
    data.formatted[offset] = '\n';
    data.formatted[offset+1] = '\n';
    data.formatted[offset+2] = '\0';

    return data;
}