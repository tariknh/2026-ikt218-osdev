extern "C" {
    #include "libc/stdint.h"
    #include "libc/stdio.h"
    #include "kernel/memory.h"
    #include "kernel/pit.h"
    #include "song/song.h"
    #include "menu/menu.h"
}

void* operator new(size_t size)    { return malloc(size); }
void* operator new[](size_t size)  { return malloc(size); }
void  operator delete(void* ptr)              noexcept { free(ptr); }
void  operator delete[](void* ptr)            noexcept { free(ptr); }
void  operator delete(void* ptr, size_t)      noexcept { free(ptr); }
void  operator delete[](void* ptr, size_t)    noexcept { free(ptr); }

extern "C" int kernel_main(void);

int kernel_main()
{
    printf("Hello World!\n");

    SongPlayer *player = create_song_player();

    menu_run(player);

    return 0;
}
