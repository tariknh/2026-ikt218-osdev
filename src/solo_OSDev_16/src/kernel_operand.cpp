#include <libc/stddef.h>        // Provides size_t type
#include <libc/stdint.h>        // Privdes  uint32_t type
#include "memory.h"             // Provides malloc() and free()
#include "terminal.h"           // Provides terminal_write()


// Overload global C++ 'new' operator so C++ object allocation uses the kernel heap
void *operator new(size_t size) {
    return malloc((uint32_t)size); // Allocate raw memory using the kernel memory manager
}

// Overload global C++ 'delete' operator so deleted C++ objects are returned to the heap
void operator delete(void *ptr) noexcept {
    free(ptr); // Free memory using the kernel memory manager
}

// Sized delete overload required by some C++ compilers
void operator delete(void *ptr, size_t size) noexcept {
    (void)size; // The size parameter is required, but free() does not need it
    free(ptr);  // Free memory using the kernel memory manager
}

// Test Object to see if the operands work as intended
class TestObject{
    public:
        int value;
};

// C-compatible test function, can be called by kernel.c
extern "C" void kernel_main() {
    TestObject *object = new TestObject; // Allocate a new object using the overloaded new operator
    object->value = 5335;                // Write to the object to verify memory functionality

    terminal_write("C++ 'new' operator working!\n");

    delete object;                       // Delete the object using the overloaded delete operator
}