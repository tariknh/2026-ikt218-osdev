#pragma once
#include <stdint.h>


/**
 * Allocates at least the number of bytes requested
 * @param size byte count requested
 * @return memory address of allocated memory
 */
void* malloc(size_t size);

/**
 * Frees already allocated memory
 * @param ptr memory address of memory to free
 */
void free(void* ptr);
