
#include "stdio.h"
#include "string.h"
#include "kernel/filesystem/vfs.h"

extern void shell_build_absolute_path(char* out, const char* input);

int cmd_write(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: write <file> <text>\n");
        return 1;
    }

    char full_path[MAX_PATH_LEN];
    shell_build_absolute_path(full_path, argv[1]);

    // Open for writing (and create if it doesn't exist)
    int fd = vfs_open(full_path, O_WRITE | O_CREAT);
    if (fd == VFS_INVALID_FD) {
        printf("write: could not open %s\n", argv[1]);
        return 1;
    }

    // Join all arguments after the filename into one string
    for (int i = 2; i < argc; i++) {
        vfs_write(fd, argv[i], strlen(argv[i]));
        if (i < argc - 1) vfs_write(fd, " ", 1);
    }

    vfs_close(fd);
    return 0;
}