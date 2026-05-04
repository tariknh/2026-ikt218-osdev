extern "C" {
    void* malloc(long unsigned int size);
    void  free(void* ptr);
}

void* operator new(long unsigned int size)                          { return malloc(size); }
void* operator new[](long unsigned int size)                        { return malloc(size); }
void  operator delete(void* ptr) noexcept                           { free(ptr); }
void  operator delete[](void* ptr) noexcept                         { free(ptr); }
void  operator delete(void* ptr, long unsigned int) noexcept        { free(ptr); }
void  operator delete[](void* ptr, long unsigned int) noexcept      { free(ptr); }
