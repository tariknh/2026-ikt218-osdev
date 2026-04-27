#include "kernel/user_mode.h"

#include <kernel/log.h>
#include <kernel/paging.h>
#include <kernel/pmm.h>
#include <stdint.h>
#include <string.h>

#define ELF_MAGIC 0x464C457F
#define ELF_TYPE_EXEC 2
#define ELF_PHDR_LOAD 1

typedef struct {
    uint32_t e_magic;
    uint8_t e_class;
    uint8_t e_endian;
    uint8_t e_version;
    uint8_t e_osabi;
    uint8_t e_abiversion;
    uint8_t e_pad[7];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version2;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) elf_header_t;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} __attribute__((packed)) elf_phdr_t;

int elf_get_info(const void* elf_data, elf_info_t* info) {
    const elf_header_t* ehdr = (const elf_header_t*)elf_data;

    if (ehdr->e_magic != ELF_MAGIC) {
        log_info("ELF: invalid magic 0x%x\n", ehdr->e_magic);
        return -1;
    }

    if (ehdr->e_type != ELF_TYPE_EXEC) {
        log_info("ELF: not an executable (type %d)\n", ehdr->e_type);
        return -1;
    }

    if (ehdr->e_machine != 3) {
        log_info("ELF: not i386 (machine %d)\n", ehdr->e_machine);
        return -1;
    }

    info->entry = ehdr->e_entry;
    info->phdr = ehdr->e_phoff;
    info->phnum = ehdr->e_phnum;
    info->type = ehdr->e_type;

    log_info("ELF: entry=0x%x, phdr=0x%x, phnum=%d\n", info->entry, info->phdr, info->phnum);
    return 0;
}

int elf_load(const void* elf_data) {
    elf_info_t info;
    if (elf_get_info(elf_data, &info) < 0) {
        return -1;
    }

    const elf_phdr_t* phdr = (const elf_phdr_t*)((uint32_t)elf_data + info.phdr);

    for (int i = 0; i < info.phnum; i++) {
        if (phdr[i].p_type == ELF_PHDR_LOAD) {
            uint32_t vaddr = phdr[i].p_vaddr;
            uint32_t filesz = phdr[i].p_filesz;
            uint32_t memsz = phdr[i].p_memsz;
            uint32_t offset = phdr[i].p_offset;

            if (vaddr < USER_PROGRAM_VADDR || vaddr >= 0x40000000) {
                log_info("ELF: load segment %d vaddr 0x%x out of user range\n", i, vaddr);
                continue;
            }

            log_info("ELF: loading segment %d: vaddr=0x%x, filesz=%d, memsz=%d\n",
                     i, vaddr, filesz, memsz);

            uint32_t start_page = vaddr & ~0xFFF;
            uint32_t end_page = (vaddr + memsz + 0xFFF) & ~0xFFF;

            for (uint32_t page = start_page; page < end_page; page += 4096) {
                uint32_t phys = pmm_alloc_frame();
                if (!phys) {
                    log_info("ELF: failed to allocate frame\n");
                    return -1;
                }
                memset((void*)phys, 0, 4096);
                vmm_map_user_page(page, phys, PAGE_USER_RW);
            }

            memcpy((void*)vaddr, (const uint8_t*)elf_data + offset, filesz);

            if (memsz > filesz) {
                memset((void*)(vaddr + filesz), 0, memsz - filesz);
            }
        }
    }

    log_info("ELF: load complete\n");
    return 0;
}