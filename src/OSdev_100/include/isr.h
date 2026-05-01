#include "../include/libc/stdint.h"
#pragma once

typedef struct {
   uint32_t ds;                                     
   uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax; 
   uint32_t int_no, err_code;                       
   uint32_t eip, cs, eflags, useresp, ss;           
} __attribute__((packed)) registers_t; 

void exception_handler(registers_t *regs);
void interrupt_demo_enable(void);
void interrupt_demo_disable(void);
int interrupt_demo_take_snapshot(registers_t* out_regs);
