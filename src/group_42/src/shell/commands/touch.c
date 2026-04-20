

#include "stdio.h"
#include "kernel/filesystem/vfs.h"

extern void shell_build_absolute_path(char* out, const char* input);


int cmd_touch(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: touch <filename>\n");
        return 1;
    }

    char full_path[MAX_PATH_LEN];

    for (int i = 1; i < argc; i++) {
        // 1. Resolve relative vs absolute
        shell_build_absolute_path(full_path, argv[i]);

        // 2. Open with O_CREAT
        // This triggers your ramfs_driver_open to allocate a new inode
        const int fd = vfs_open(full_path, O_CREAT | O_WRITE);

        if (fd == VFS_INVALID_FD) {
            printf("touch: cannot touch '%s': Permission denied or path error\n", argv[i]);
            continue;
        }

        // 3. We don't actually need to write anything, just close it
        vfs_close(fd);
    }

    return 0;
}
