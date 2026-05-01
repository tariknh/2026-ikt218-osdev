#include <stdint.h>

/* Syscall numbers - POSIX/Linux */
#define SYS_read     3
#define SYS_write    4
#define SYS_exit    1
#define SYS_execve  11
#define SYS_ioctl   16
#define SYS_nanosleep 35
#define SYS_timer   64

/* Command addresses - where commands are loaded */
#define CMD_ECHO_ADDR  0x08050000
#define CMD_ECHO_VBASE 0x08050000  /* ELF's expected base */

/* Get entry from ELF loaded at addr */
static inline uint32_t get_elf_entry(uint32_t addr) {
    /* Entry is at offset 24 in ELF header */
    return *((uint32_t*)(addr + 24));
}

/* Simple syscall wrapper */
static inline uint32_t syscall(uint32_t num, uint32_t a, uint32_t b, uint32_t c) {
    uint32_t ret;
    __asm__ volatile(
        "movl %1, %%eax\n"
        "movl %2, %%ebx\n"
        "movl %3, %%ecx\n"
        "movl %4, %%edx\n"
        "int $0x80\n"
        "movl %%eax, %0"
        : "=g"(ret)
        : "g"(num), "g"(a), "g"(b), "g"(c)
        : "eax", "ebx", "ecx", "edx"
    );
    return ret;
}

static inline void write_str(const char* s) {
    int len = 0;
    while (s[len]) len++;
    syscall(SYS_write, 1, (uint32_t)s, len);
}

static inline void write_str_n(const char* s, int len) {
    syscall(SYS_write, 1, (uint32_t)s, len);
}

static inline void write_char(char c) {
    syscall(SYS_write, 1, (uint32_t)&c, 1);
}

/* Try reading - returns 1 if character available, 0 otherwise */
static inline int try_read_char(char* c) {
    /* Just call read and return - if no key, it returns 0 */
    return syscall(SYS_read, 0, (uint32_t)c, 1);
}

static inline void exit(int code) {
    write_str("Calling exit syscall\n");
    syscall(SYS_exit, code, 0, 0);
    write_str("Exit returned - looping\n");
    while(1);
}

/* Shell state */
#define BUFFER_SIZE 256
static char buffer[BUFFER_SIZE];
static int buf_pos = 0;

void process_command(void) {
    if (buf_pos == 0) return;

    buffer[buf_pos] = '\0';

    char* cmd = buffer;
    while (*cmd == ' ') cmd++;

    if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p' && cmd[4] == '\0') {
        write_str("Available commands:\n");
        write_str("  help         - Show this help message\n");
        write_str("  clear        - Clear the terminal\n");
        write_str("  echo <args>  - Print arguments\n");
        write_str("  ver          - Show version\n");
        write_str("  test         - Test command\n");
    } else if (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 'e' && cmd[3] == 'a' && cmd[4] == 'r' && cmd[5] == '\0') {
        for (int i = 0; i < 25; i++) write_char('\n');
    } else if (cmd[0] == 'e' && cmd[1] == 'c' && cmd[2] == 'h' && cmd[3] == 'o') {
        char* arg = cmd + 4;
        while (*arg == ' ') arg++;
        write_str(arg);
        write_char('\n');
    } else if (cmd[0] == 't' && cmd[1] == 'e' && cmd[2] == 's' && cmd[3] == 't' && cmd[4] == '\0') {
        write_str("Test command works!\n");
    } else if (cmd[0] == 'v' && cmd[1] == 'e' && cmd[2] == 'r' && cmd[3] == '\0') {
        write_str("Kernel42 OS v0.1\n");
        write_str("Userspace shell\n");
    }
    
    buf_pos = 0;
}

void _start(void) {
    write_str("Welcome to userspace shell!\n");
    write_str("Type 'help' for commands.\n");
    write_str("> ");
    
    while (1) {
        char c;
        int n = try_read_char(&c);
        
        if (n <= 0) continue;
        
        /* Handle Enter key */
        if (c == '\n' || c == '\r') {
            write_char('\n');
            process_command();
            write_str("> ");
            continue;
        }
        
        /* Handle Backspace */
        if (c == '\b' || c == 0x7F) {
            if (buf_pos > 0) {
                buf_pos--;
                write_str("\b \b");
            }
            continue;
        }
        
        /* Regular character */
        if (c >= 32 && c < 127) {
            if (buf_pos < BUFFER_SIZE - 1) {
                buffer[buf_pos++] = c;
                write_char(c);
            }
        }
    }
}