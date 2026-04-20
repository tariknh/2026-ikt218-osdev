#include "stdio.h"
#include "string.h"
#include "kernel/filesystem/vfs.h"
#include "shell/shell.h"

extern void shell_build_absolute_path(char* out, const char* input);

int cmd_cd(int argc, char** argv) {
    if (argc < 2) {
        shell_set_cwd("/"); // Default to root if no path provided
        return 0;
    }

    char target_path[MAX_PATH_LEN];
    const char* input = argv[1];

    // 1. Resolve Absolute vs Relative
    shell_build_absolute_path(target_path, input);

    // 2. Validate the path exists
    int fd = vfs_open(target_path, O_READ);
    if (fd == VFS_INVALID_FD) {
        printf("cd: %s: No such directory\n", input);
        return 1;
    }

    // 3. Ensure it's a directory
    vfs_stat_t st;
    if (vfs_fstat(fd, &st) == 0) {
        if (st.type != VFS_DIRECTORY) {
            printf("cd: %s: Not a directory\n", input);
            vfs_close(fd);
            return 1;
        }
    }

    vfs_close(fd);

    // 4. Update the Shell CWD
    shell_set_cwd(target_path);
    return 0;
}
