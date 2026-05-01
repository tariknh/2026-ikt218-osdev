#include "arch/i386/cpu/gdt.h"

#include <kernel/log.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


/**
 * TSS is used to provide CPL0 stack (esp0) for interrupts and system calls. Guarantee that kernel always uses a
 * dedicated kernel stack when coming from userspace. (we shouldnt trust user stack).
 *
 */
typedef struct __attribute__((packed)) {
    uint16_t prev_task_link;
    uint16_t reserved0;
    uint32_t esp0; // stack pointer for CPL0
    uint16_t ss0; // segment selector for CPL0 stack
    uint16_t reserved1;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved2;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved3;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint16_t es, cs, ss, ds, fs, gs, ldtr, reserved4;
    uint16_t iopb_offset;
    uint32_t esp3;
    uint16_t ss3;
    uint16_t reserved5;
} tss_t;

static tss_t tss;
static uint8_t kernel_stack[8192]; // 8 KiB stack

extern void tss_flush(void); // defined in assembly

void init_tss(void) {
    memset(&tss, 0, sizeof(tss));
    tss.ss0 = 0x10;
    tss.esp0 = (uint32_t) (kernel_stack + sizeof(kernel_stack));

    // CS / SS / DS / ES / FS / GS: kernel segments (0x08 for CS, 0x10 for data)
    tss.cs = 0x08;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x10;


    gdt_set_entry(5, (uint32_t) &tss, sizeof(tss) - 1, 0x89, 0x00);

    gdt_reload();

    // load TR (task register) with tss
    tss_flush();

    log_info("TSS initialized, kernel stack at 0x%x\n", tss.esp0);
}

// update cpl0 stack pointer in tss
void tss_update_esp0(uint32_t esp) {
    tss.esp0 = esp;
}
