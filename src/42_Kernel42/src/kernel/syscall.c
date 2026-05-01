#include "kernel/syscall.h"

#include <drivers/audio/PCSPK.h>
#include <drivers/input/keyboard.h>
#include <kernel/pit.h>
#include <drivers/video/vga_text.h>
#include <kernel/errno.h>
#include <kernel/fcntl.h>
#include <kernel/log.h>
#include <kernel/unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "arch/i386/cpu/isr.h"
#include "arch/i386/cpu/ports.h"
#include "drivers/input/keyboard.h"
#include "shell/shell.h"


void* syscall_table[MAX_SYSCALLS];

/*
 * Process SYScalls
 */

/**
 * terminate the current process.
 * Args:    int status
 * Returns: never
 * Errors:  none (process halts)
 */
uint32_t sys_exit(syscall_args_t* args) {
  (void)args;
  return 0xDEADC0DE;  // Magic return to signal exit
}

/**
 * create a child process (clone).
 * Args:    none
 * Returns: -1 (not implemented)
 * Errors:  ENOSYS
 */
uint32_t sys_fork(syscall_args_t* args) {
  (void)args;
  return -1;
}

/**
 * wait for a child process to exit.
 * Args:    int* status (out)
 * Returns: -1 (not implemented)
 * Errors:  ENOSYS
 */
uint32_t sys_wait(syscall_args_t* args) {
  (void)args;
  return -1;
}

/**
 *replace the current process image with a new program.
 * Args:    const char* path, char* const argv[], char* const envp[]
 * Returns: -1 (not implemented)
 * Errors:  ENOSYS
 */
extern void switch_to_user_mode(uint32_t entry, uint32_t stack_top);
extern int vmm_map_user_page(uint32_t virt, uint32_t phys, uint32_t flags);
extern void* pmm_alloc_frame(void);
extern void* memset(void*, int, unsigned long);

#define PAGE_USER_RW 0x007

uint32_t sys_execve(syscall_args_t* args) {
  uint32_t elf_ptr = args->a;
  (void)args->b;  /* argv - not used yet */
  (void)args->c;  /* envp - not used yet */
  
  if (elf_ptr == 0) {
    return -1;
  }
  
  /* ELF is already loaded at 0x08050000 - entry is just offset 0x72 */
  uint32_t entry = elf_ptr + 0x72;
  
  /* Set up stack - ALLOCATE AND MAP PAGES */
  uint32_t stack_top = 0x08040000;
  uint32_t stack_pages = 4;  /* 16KB stack */
  
  for (uint32_t page = stack_top - (stack_pages * 4096); page < stack_top; page += 4096) {
    uint32_t phys = (uint32_t)pmm_alloc_frame();
    if (!phys) return -1;
    memset((void*)phys, 0, 4096);
    vmm_map_user_page(page, phys, PAGE_USER_RW);
  }
  
  /* Switch to the new program - this doesn't return */
  switch_to_user_mode(entry, stack_top);
  
  return 0;
}

/**
 * get the current process ID.
 * Args:    none
 * Returns: current PID (always 1 for now)
 * Errors:  none
 */
uint32_t sys_getpid(syscall_args_t* args) {
  (void)args;
  return 1;
}

/*
 * I/O SYScalls
 */

/**
 * read from a file descriptor.
 * Args:    int fd, void* buf, size_t count
 * Returns: bytes read, 0 on EOF, -1 on error
 * Errors:  EBADF if fd is invalid or not readable
 *
 * Currently implemented: fd=0 reads from keyboard buffer (blocking).
 * All other fds return -1.
 */
uint32_t sys_read(syscall_args_t* args) {
  int fd = (int)args->a;
  uint32_t buf_ptr = args->b;
  uint32_t count = args->c;

  if (buf_ptr == 0 || count == 0)
    return -EINVAL;

  if (fd == 0) {
    uint32_t bytes_read = 0;
    uint8_t* buf = (uint8_t*)buf_ptr;

    while (bytes_read < count) {
      decode_keyboard();
      if (keyboard_has_key()) {
        if (pop_key(&buf[bytes_read])) {
          bytes_read++;
        }
      } else {
        /* No key available - return what we have */
        break;
      }
    }
    return bytes_read;
  }

  return -EBADF;
}

/**
 * write to a file descriptor.
 * Args:    int fd, const void* buf, size_t count
 * Returns: bytes written, -1 on error
 * Errors:  EBADF if fd is invalid or not writable
 *
 * Currently implemented: fd=1 and fd=2 write to VGA terminal.
 * All other fds return -1.
 */
uint32_t sys_write(syscall_args_t* args) {
  int fd = (int)args->a;
  uint32_t buf_ptr = args->b;
  uint32_t count = args->c;

  if (buf_ptr == 0 || count == 0)
    return -EINVAL;

  if (fd == 1 || fd == 2) {
    const char* str = (const char*)buf_ptr;
    for (uint32_t i = 0; i < count && str[i]; i++) {
      vga_text_putchar(str[i]);
    }
    return count;
  }

  return -EBADF;
}

/**
 *  open a file.
 * Args:    const char* pathname, int flags, mode_t mode
 * Returns: file descriptor, -1 on error
 * Errors:  ENOSYS (not implemented)
 */
uint32_t sys_open(syscall_args_t* args) {
  (void)args;
  return -1;
}

/**
 * close a file descriptor.
 * Args:    int fd
 * Returns: 0 on success, -1 on error
 * Errors:  EBADF if fd is invalid
 */
uint32_t sys_close(syscall_args_t* args) {
  (void)args;
  return 0;
}

/**
 * change the read/write file offset.
 * Args:    int fd, int32_t offset, int whence (SEEK_SET/SEEK_CUR/SEEK_END)
 * Returns: new file offset, -1 on error
 * Errors:  ENOSYS (not implemented)
 */
uint32_t sys_lseek(syscall_args_t* args) {
  (void)args;
  return -1;
}

/**
 * duplicate a file descriptor.
 * Args:    int oldfd
 * Returns: new fd, -1 on error
 * Errors:  ENOSYS (not implemented)
 */
uint32_t sys_dup(syscall_args_t* args) {
  (void)args;
  return -1;
}

/**
 * duplicate a file descriptor to a specific new fd.
 * Args:    int oldfd, int newfd
 * Returns: newfd, -1 on error
 * Errors:  ENOSYS (not implemented)
 */
uint32_t sys_dup2(syscall_args_t* args) {
  (void)args;
  return -1;
}

/*
 * MEMORY SYScalls
 */

/**
 *  change the program break (heap end).
 * Args:    void* addr (new break value)
 * Returns: 0 (not implemented — memory layout fixed)
 * Errors:  none
 */
uint32_t sys_brk(syscall_args_t* args) {
  (void)args;
  return 0;
}

/*
 * PATH SYScalls
 */

/**
 *  get the current working directory.
 * Args:    char* buf, size_t size
 * Returns: buf on success, NULL on error
 * Errors:  ENOSYS (not implemented)
 */
uint32_t sys_getcwd(syscall_args_t* args) {
  char* buf = (char*)args->a;
  uint32_t size = args->b;

  if (buf == 0 || size == 0)
    return -EINVAL;

  const char* path = "/";
  size_t len = 2; /* "/" + NUL */

  if (size < len)
    return -EINVAL;

  for (size_t i = 0; i < len; i++) {
    buf[i] = path[i];
  }

  return (uint32_t)buf;
}

/**
 *change the current working directory.
 * Args:    const char* path
 * Returns: 0 on success, -1 on error
 * Errors:  ENOSYS (not implemented)
 */
uint32_t sys_chdir(syscall_args_t* args) {
  (void)args;
  return -1;
}

/*
 * PROCESS INFO SYScalls
 */

/**
 * get the real user ID of the current process.
 * Args:    none
 * Returns: 0 (not implemented — no users)
 * Errors:  none
 */
uint32_t sys_getuid(syscall_args_t* args) {
  (void)args;
  return 0;
}

/**
 *  get the real group ID of the current process.
 * Args:    none
 * Returns: 0 (not implemented — no groups)
 * Errors:  none
 */
uint32_t sys_getgid(syscall_args_t* args) {
  (void)args;
  return 0;
}

/*
 * IPC SYScalls
 */

/**
 *  create a pipe.
 * Args:    int fds[2] (out: [0]=read end, [1]=write end)
 * Returns: 0 on success, -1 on error
 * Errors:  ENOSYS (not implemented)
 */
uint32_t sys_pipe(syscall_args_t* args) {
  (void)args;
  return -1;
}

/**
 * add a handler to the syscall table.
 */
static void syscall_register(uint32_t num, syscall_fn_t fn) {
  if (num < MAX_SYSCALLS) {
    syscall_table[num] = (void*)fn;
  }
}

/**
 *  extract arguments from CPU registers into a
 * syscall_args_t struct, look up the handler, call it, and store the
 * result back into regs->eax.
 *
 * Called from the 0x80 ISR stub in interrupts.asm.
 */
void syscall_handler(registers_t* regs) {
  uint32_t snum = regs->eax;

  syscall_args_t args;
  args.number = snum;
  args.a = regs->ebx;
  args.b = regs->ecx;
  args.c = regs->edx;
  args.d = regs->esi;
  args.e = regs->edi;
  args.f = regs->esp_pusha;

  uint32_t result = -1;

  if (snum < MAX_SYSCALLS && syscall_table[snum] != NULL) {
    result = ((syscall_fn_t)syscall_table[snum])(&args);
  }

  regs->eax = result;
}

/**
 *  zero the syscall table and register all handlers.
 */
void init_syscalls(void) {
  log_info("Initialising syscalls...\n");

  for (int i = 0; i < MAX_SYSCALLS; i++) {
    syscall_table[i] = NULL;
  }

  syscall_register(SYS_exit, sys_exit);
  syscall_register(SYS_fork, sys_fork);
  syscall_register(SYS_wait, sys_wait);
  syscall_register(SYS_execve, sys_execve);
  syscall_register(SYS_getpid, sys_getpid);

  syscall_register(SYS_read, sys_read);
  syscall_register(SYS_write, sys_write);
  syscall_register(SYS_open, sys_open);
  syscall_register(SYS_close, sys_close);
  syscall_register(SYS_lseek, sys_lseek);
  syscall_register(SYS_dup, sys_dup);
  syscall_register(SYS_dup2, sys_dup2);

  syscall_register(SYS_brk, sys_brk);

  syscall_register(SYS_getcwd, sys_getcwd);
  syscall_register(SYS_chdir, sys_chdir);

  syscall_register(SYS_getuid, sys_getuid);
  syscall_register(SYS_getgid, sys_getgid);

  syscall_register(SYS_pipe, sys_pipe);

uint32_t sys_timer(syscall_args_t* args) {
  uint32_t ms = args->a;
  sleep_interrupt(ms);
  return 0;
}

uint32_t sys_beep(syscall_args_t* args) {
  uint32_t freq = args->a;
  if (freq == 0) {
    PCSPK_STOP();
  } else {
    PCSPK_PLAY(freq);
  }
  return 0;
}

uint32_t sys_getkey(syscall_args_t* args) {
  (void)args;
  decode_keyboard();
  if (keyboard_has_key()) {
    uint8_t key;
    if (pop_key(&key)) {
      return key;
    }
  }
  return 0;
}

uint32_t sys_nanosleep(syscall_args_t* args) {
  uint32_t ms = args->a;
  sleep_interrupt(ms);
  return 0;
}

uint32_t sys_ioctl(syscall_args_t* args) {
  uint32_t fd = args->a;
  uint32_t cmd = args->b;
  uint32_t val = args->c;
  (void)fd;
  (void)val;
  if (cmd == 0) {
    PCSPK_STOP();
  } else if (cmd == 1) {
    PCSPK_PLAY(val);
  }
  return 0;
}

  syscall_register(SYS_timer, sys_timer);
  syscall_register(SYS_beep, sys_beep);
  syscall_register(SYS_getkey, sys_getkey);
  syscall_register(SYS_nanosleep, sys_nanosleep);
  syscall_register(SYS_ioctl, sys_ioctl);

  register_interrupt_handler(0x80, (isr_t)syscall_stub);

  log_info("Syscall table ready (%d entries)\n", MAX_SYSCALLS);
}
