

#include "stdio.h"
#include "kernel/filesystem/vfs.h"
#include "shell/shell.h"

extern void shell_build_absolute_path(char* out, const char* input);


int cmd_ls(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : shell_get_cwd();
    char target_path[MAX_PATH_LEN];
    shell_build_absolute_path(target_path, path);

    int fd = vfs_open(target_path, O_READ);
    if (fd == VFS_INVALID_FD) {
        printf("ls: %s: No such directory\n", target_path);
        return 1;
    }

    vfs_dirent_t dirent;

    // Call the driver through the VFS
    // (Ensure you've added vfs_readdir wrapper to vfs.c!)
    while (vfs_read(fd, &dirent, 1) > 0) {
        if (dirent.type == VFS_DIRECTORY) {
            printf("[DIR]  %s\n", dirent.name);
        } else {
            printf("[FILE] %s \n", dirent.name);
        }
    }

    vfs_close(fd);
    return 0;
}
