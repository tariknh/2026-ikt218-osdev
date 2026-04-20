

#include "stdio.h"
#include "kernel/filesystem/vfs.h"

extern void shell_build_absolute_path(char* out, const char* input);


int cmd_mkdir(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: mkdir <directory name>\n");
        return 1;
    }

    char full_path[MAX_PATH_LEN];

    for (int i = 1; i < argc; i++) {
        // 1. Resolve relative vs absolute
        shell_build_absolute_path(full_path, argv[i]);

        // 2. Open with O_CREAT
        // This triggers your ramfs_driver_open to allocate a new inode
        const int return_code = vfs_mkdir(full_path);

        if (return_code == VFS_INVALID_FD) {
            printf("mkdir: cannot create '%s': Path error\n", argv[i]);
        }
    }

    return 0;
}
