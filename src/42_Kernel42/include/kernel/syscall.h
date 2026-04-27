#pragma once
#include <arch/i386/cpu/isr.h>
#include <stdint.h>

/*
 * SYSCALL NUMBERS
 * Numbers match Linux/i386 where applicable.
 */

#define SYS_exit 1
#define SYS_fork 2
#define SYS_read 3
#define SYS_write 4
#define SYS_open 5
#define SYS_close 6
#define SYS_wait 12
#define SYS_execve 11
#define SYS_chdir 15
#define SYS_getcwd 17
#define SYS_dup 19
#define SYS_dup2 21
#define SYS_pipe 42
#define SYS_brk 45
#define SYS_lseek 60
#define SYS_getpid 20
#define SYS_getuid 61
#define SYS_getgid 62
#define SYS_ioctl 16
#define SYS_nanosleep 35
#define SYS_timer 64
#define SYS_beep 65
#define SYS_getkey 66

#define MAX_SYSCALLS 64

/**
 *Syscall arguments
 */
typedef struct {
  uint32_t number;
  uint32_t a; /* ebx: arg1 */
  uint32_t b; /* ecx: arg2 */
  uint32_t c; /* edx: arg3 */
  uint32_t d; /* esi: arg4 */
  uint32_t e; /* edi: arg5 */
  uint32_t f; /* ebp: arg6 */
} syscall_args_t;

/**
 * Syscall function type. ALl handlers follow this signature
 */
typedef uint32_t (*syscall_fn_t)(syscall_args_t*);

/**
 * Initialise the syscall table and register interrupt handler 0x80.
 */
void init_syscalls(void);

/**
 *  extract arguments from CPU registers into a
 * syscall_args_t struct, look up the handler, and store the
 * result back into regs->eax.
 *
 * Called from the 0x80 ISR stub in interrupts.asm.
 */
void syscall_handler(registers_t* regs);

/**
 * Pointer to syscall table, userspace code can read to discover syscalls.
 */
extern void* syscall_table[MAX_SYSCALLS];
