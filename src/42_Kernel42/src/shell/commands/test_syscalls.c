#include <drivers/video/vga_text.h>
#include <kernel/errno.h>
#include <kernel/fcntl.h>
#include <kernel/pit.h>
#include <kernel/syscall.h>
#include <kernel/unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "shell/shell_command.h"

static uint32_t syscall_via_int0x80(uint32_t num, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    uint32_t ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(a), "c"(b), "d"(c), "S"(d), "D"(e)
        : "memory");
    return ret;
}

int cmd_test_syscalls(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("=== TESTING int 0x80 SYSCALL MECHANISM ===\n\n");

    printf("--- SYS_GETPID via int 0x80 ---\n");
    uint32_t pid = syscall_via_int0x80(SYS_getpid, 0, 0, 0, 0, 0);
    printf("Result: %u (expected 1)\n", pid);
    printf("Status: %s\n\n", pid == 1 ? "PASS" : "FAIL");
    sleep_interrupt(300);

    printf("--- SYS_GETUID via int 0x80 ---\n");
    uint32_t uid = syscall_via_int0x80(SYS_getuid, 0, 0, 0, 0, 0);
    printf("Result: %u (expected 0)\n", uid);
    printf("Status: %s\n\n", uid == 0 ? "PASS" : "FAIL");
    sleep_interrupt(300);

    printf("--- SYS_GETGID via int 0x80 ---\n");
    uint32_t gid = syscall_via_int0x80(SYS_getgid, 0, 0, 0, 0, 0);
    printf("Result: %u (expected 0)\n", gid);
    printf("Status: %s\n\n", gid == 0 ? "PASS" : "FAIL");
    sleep_interrupt(300);

    printf("--- SYS_WRITE (fd=1) via int 0x80 ---\n");
    const char* msg = "Hello from int 0x80 syscall!\n";
    uint32_t written = syscall_via_int0x80(SYS_write, 1, (uint32_t)msg, strlen(msg), 0, 0);
    printf("Result: %u (expected %u)\n", written, (uint32_t)strlen(msg));
    printf("Status: %s\n\n", written == strlen(msg) ? "PASS" : "FAIL");
    sleep_interrupt(300);

    printf("--- SYS_WRITE (fd=99 invalid) via int 0x80 ---\n");
    written = syscall_via_int0x80(SYS_write, 99, (uint32_t)"x", 1, 0, 0);
    printf("Result: %d (expected -EBADF=%d)\n", written, -EBADF);
    printf("Status: %s\n\n", (int32_t)written == -EBADF ? "PASS" : "FAIL");
    sleep_interrupt(300);

    printf("--- SYS_GETCWD via int 0x80 ---\n");
    char cwd_buf[32] = {0};
    uint32_t cwd_ret = syscall_via_int0x80(SYS_getcwd, (uint32_t)cwd_buf, sizeof(cwd_buf), 0, 0, 0);
    printf("Result: 0x%x, buffer: \"%s\"\n", cwd_ret, cwd_buf);
    printf("Status: %s\n\n", (cwd_ret == (uint32_t)cwd_buf && cwd_buf[0] == '/') ? "PASS" : "FAIL");
    sleep_interrupt(300);

    printf("--- SYS_CLOSE (fd=0 valid) via int 0x80 ---\n");
    uint32_t close_ret = syscall_via_int0x80(SYS_close, 0, 0, 0, 0, 0);
    printf("Result: %u (expected 0)\n", close_ret);
    printf("Status: %s\n\n", close_ret == 0 ? "PASS" : "FAIL");
    sleep_interrupt(300);

    printf("--- SYS_EXIT via int 0x80 ---\n");
    printf("Note: SYS_EXIT will halt the CPU, test skipped\n\n");
    sleep_interrupt(300);

    printf("=== STUB SYSCALLS (all return -1) ===\n");
    printf("sys_fork: %d (expected -1)\n", syscall_via_int0x80(SYS_fork, 0, 0, 0, 0, 0));
    sleep_interrupt(300);
    printf("sys_execve: %d (expected -1)\n", syscall_via_int0x80(SYS_execve, 0, 0, 0, 0, 0));
    sleep_interrupt(300);
    printf("sys_wait: %d (expected -1)\n", syscall_via_int0x80(SYS_wait, 0, 0, 0, 0, 0));
    sleep_interrupt(300);
    printf("sys_open: %d (expected -1)\n", syscall_via_int0x80(SYS_open, 0, 0, 0, 0, 0));
    sleep_interrupt(300);
    printf("sys_lseek: %d (expected -1)\n", syscall_via_int0x80(SYS_lseek, 0, 0, 0, 0, 0));
    sleep_interrupt(300);
    printf("sys_dup: %d (expected -1)\n", syscall_via_int0x80(SYS_dup, 0, 0, 0, 0, 0));
    sleep_interrupt(300);
    printf("sys_dup2: %d (expected -1)\n", syscall_via_int0x80(SYS_dup2, 0, 0, 0, 0, 0));
    sleep_interrupt(300);
    printf("sys_pipe: %d (expected -1)\n", syscall_via_int0x80(SYS_pipe, 0, 0, 0, 0, 0));
    sleep_interrupt(300);
    printf("sys_chdir: %d (expected -1)\n", syscall_via_int0x80(SYS_chdir, 0, 0, 0, 0, 0));
    sleep_interrupt(300);
    printf("sys_brk: %d (expected 0)\n", syscall_via_int0x80(SYS_brk, 0, 0, 0, 0, 0));

    printf("\n=== ALL TESTS COMPLETE ===\n");
    return 0;
}
