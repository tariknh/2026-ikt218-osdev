#include "kernel/memory.h"

void *memcpy(void *dest, const void *src, size_t num) {
    uint8_t *dest_bytes = dest;
    const uint8_t *src_bytes = src;

    for (size_t i = 0; i < num; i++) {
        dest_bytes[i] = src_bytes[i];
    }

    return dest;
}

void *memset(void *ptr, int value, size_t num) {
    uint8_t *bytes = ptr;
    uint8_t byte_value = (uint8_t) value;

    for (size_t i = 0; i < num; i++) {
        bytes[i] = byte_value;
    }

    return ptr;
}

void *memset16(void *ptr, uint16_t value, size_t num) {
    uint16_t *words = ptr;

    for (size_t i = 0; i < num; i++) {
        words[i] = value;
    }

    return ptr;
}
