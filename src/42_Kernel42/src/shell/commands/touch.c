

#include "stdio.h"
#include "kernel/filesystem/vfs.h"

extern void shell_build_absolute_path(char* out, const char* input);

// Classic touch works by modifiying the last_changed flag of a file. but in our simple ramfs thats not relevant.
int cmd_touch(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: touch <filename>\n");
        return 1;
    }

    char full_path[MAX_PATH_LEN];

    for (int i = 1; i < argc; i++) {
        shell_build_absolute_path(full_path, argv[i]);

        //  Open with O_CREAT
        // This triggers your ramfs_driver_open to allocate a new inode
        const int fd = vfs_open(full_path, O_CREAT | O_WRITE);

        if (fd == VFS_INVALID_FD) {
            printf("touch: cannot touch '%s': Permission denied or path error\n", argv[i]);
            continue;
        }

        //  We don't actually need to write anything, just close it
        vfs_close(fd);
    }

    return 0;
}
