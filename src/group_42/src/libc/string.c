#include "libc/string.h"

#include "stddef.h"
#include "libc/stdint.h"

size_t strlen(const char* str) {
  size_t len = 0;
  while (str[len])
    len++;
  return len;
}

int strcmp(const char* l, const char* r) {
  for (; *l == *r && *l; l++, r++)
    ;
  return *(unsigned char*)l - *(unsigned char*)r;
}

int strncmp(const char* l, const char* r, size_t num) {
  if (num == 0) return 0;

  while (num > 0) {
    if (*l != *r || *l == '\0') {
      return *(unsigned char*)l - *(unsigned char*)r;
    }
    l++;
    r++;
    num--;

  }
  return 0;
}

void* memcpy(void* restrict dest, const void* restrict src, size_t count) {
  char* dst8 = (char*)dest;
  char* src8 = (char*)src;

  if (count & 1) {     // If the count is odd
    dst8[0] = src8[0]; // Copy one byte from source to destination
    dst8 += 1;         // Increment destination pointer by one byte
    src8 += 1;         // Increment source pointer by one byte
  }

  count /= 2;          // Divide the count by two to copy 2 bytes at a time
  while (count--) {    // Loop through each pair of 2 bytes
    dst8[0] = src8[0]; // Copy the first byte of the pair from source to destination
    dst8[1] = src8[1]; // Copy the second byte of the pair from source to destination

    dst8 += 2; // Increment destination pointer by 2 bytes
    src8 += 2; // Increment source pointer by 2 bytes
  }

  return (void*)dest; // Return the destination pointer
}

void* memset(void* ptr, int value, size_t num) {
  unsigned char* p = ptr; // Cast the pointer to unsigned char*
  while (num--)
    *p++ = (unsigned char)value; // Set each byte to the given value
  return ptr;                    // Return the pointer to the block of memory
}

char* strcpy(char* dest, const char* src) {
  char* saved = dest;
  while ((*dest++ = *src++));
  return saved;
}

char* strncpy(char* dest, const char* src, size_t n) {
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; i++) {
    dest[i] = src[i];
  }
  // Standard C behavior: pad the rest with null bytes
  for (; i < n; i++) {
    dest[i] = '\0';
  }
  return dest;
}

char* strrchr (const char* s, int c) {
  char* last = NULL;
  unsigned char target = (unsigned char)c;

  // Iterate through the string until the null terminator
  do {
    if ((unsigned char)*s == target) {
      last = (char*)s;
    }
  } while (*s++);

  return last;
}

char* strncat(char* dest, const char* src, size_t n) {
  size_t dest_len = strlen(dest);
  size_t i;

  for (i = 0; i < n && src[i] != '\0'; i++) {
    dest[dest_len + i] = src[i];
  }
  dest[dest_len + i] = '\0';

  return dest;
}