extern "C" {
    #include "terminal.h"
    #include "idt.h"
    #include "memory.h"
}
 
// ─── Global operator new / delete (backed by our kernel malloc) ──────────────
 
void* operator new(size_t size)          { return malloc(size); }
void* operator new[](size_t size)        { return malloc(size); }
 
void  operator delete(void* ptr) noexcept          { free(ptr); }
void  operator delete[](void* ptr) noexcept        { free(ptr); }
void  operator delete(void* ptr, size_t) noexcept  { free(ptr); }
void  operator delete[](void* ptr, size_t) noexcept{ free(ptr); }
 
// ─── Forward declaration expected by kernel.c ────────────────────────────────
extern "C" int kernel_main();
 
// ─── kernel_main ─────────────────────────────────────────────────────────────
// Everything here runs after the heap and paging are already up.
int kernel_main() {
 
    // ── ISR smoke test ───────────────────────────────────────────────────────
    // (sleep() is defined in kernel.c and visible via extern "C" linkage)
 
    __asm__ __volatile__("int $0");
    __asm__ __volatile__("int $1");
    __asm__ __volatile__("int $2");
 
    // ── malloc / new demo ────────────────────────────────────────────────────
 
    // Plain malloc
    char* buf = (char*)malloc(64);
    if (buf) {
        const char* msg = "Heap buffer via malloc()\n";
        int i = 0;
        while (msg[i]) { buf[i] = msg[i]; i++; }
        buf[i] = '\0';
        terminal_write(buf);
        free(buf);
    }
 
    // operator new  (calls malloc under the hood via the overloads above)
    uint32_t* num = new uint32_t(42);
    if (num) {
        terminal_write("Allocated uint32_t via new\n");
        delete num;
    }
 
    // operator new[]
    uint32_t* arr = new uint32_t[8];
    if (arr) {
        for (int i = 0; i < 8; i++) arr[i] = i * i;
        terminal_write("Array via new[]: allocated and filled\n");
        delete[] arr;
    }
 
    // ── Main loop ────────────────────────────────────────────────────────────
 
    terminal_write("Kernel ready - type something:\n\n");
    while (true) {
        __asm__("hlt");
    }
 
    return 0;
}