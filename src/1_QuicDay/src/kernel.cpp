extern "C" {
#include "libc/stdint.h"
#include "libc/stdio.h"

void* malloc(size_t size);
void free(void* ptr);
}

void* operator new(size_t size) {
  return malloc(size);
}

void* operator new[](size_t size) {
  return malloc(size);
}

void operator delete(void* ptr) noexcept {
  free(ptr);
}

void operator delete[](void* ptr) noexcept {
  free(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
  free(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
  free(ptr);
}

extern "C" int kernel_main(void) {
  int* buffer = new int[16];

  if (buffer == 0) {
    printf("C++ new[] failed.\n");
    return -1;
  }

  for (uint32_t i = 0; i < 16; ++i) {
    buffer[i] = (int)(i * i);
  }

  printf("C++ new[] allocation at: %p\n", (void*)buffer);
  delete[] buffer;
  return 0;
}