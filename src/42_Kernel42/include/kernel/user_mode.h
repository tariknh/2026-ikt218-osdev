#pragma once
#include <stdint.h>
//**FOr simplicity load at set offset
#define USER_PROGRAM_VADDR 0x08000000
#define USER_VIRT_STACK 0x08040000

/** Load an ELF from c array*/
int elf_load(const void* elf_data);
/** Execute an elf*/
void elf_execute(uint32_t entry_point);
/** Create user stack and switch to user mode */
void switch_to_user_mode(uint32_t entry, uint32_t stack_top) __attribute__((naked));

typedef struct {
  uint32_t entry;
  uint32_t phdr;
  uint16_t phnum;
  uint16_t type;
} elf_info_t;

int elf_get_info(const void* elf_data, elf_info_t* info);
