#include "stdio.h"
#include "kernel/filesystem/vfs.h"

extern void shell_build_absolute_path(char* out, const char* input);


int cmd_cat(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: cat <filename>\n");
        return 1;
    }

    char full_path[MAX_PATH_LEN];
    char buffer[512]; // A reasonable chunk size for stack allocation

    for (int i = 1; i < argc; i++) {
        //  Resolve path (relative to absolute)
        shell_build_absolute_path(full_path, argv[i]);

        //  Open for reading
        int fd = vfs_open(full_path, O_READ);
        if (fd == VFS_INVALID_FD) {
            printf("cat: %s: No such file\n", argv[i]);
            continue;
        }

        vfs_stat_t st;
        vfs_fstat(fd, &st);
        if (st.type == VFS_DIRECTORY) {
            printf("cat: %s: Is a directory\n", argv[i]);
            vfs_close(fd);
            continue;
        }

        //  Read in chunks until EOF
        int bytes_read;
        while ((bytes_read = vfs_read(fd, buffer, sizeof(buffer) - 1)) > 0) {
            // Null-terminate the chunk so printf knows where to stop
            buffer[bytes_read] = '\0';
            printf("%s", buffer);
        }

        vfs_close(fd);
    }

    printf("\n"); // Standard Unix behavior: end with a newline
    return 0;
}