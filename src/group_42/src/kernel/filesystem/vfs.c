#include <kernel/filesystem/vfs.h>
#include <kernel/filesystem/ramfs.h>

#include "stddef.h"
#include "string.h"

vfs_fd_t fd_table[MAX_OPEN_FILES];
static vfs_mount_t mount_table[8]; // example limit

void vfs_init() {
    // 1. Initialize the FD table
    for(int i = 0; i < MAX_OPEN_FILES; i++) {
        fd_table[i].used = 0;
    }

    // Reserve fd 0-2 for stdio
    fd_table[0].used = 1;
    fd_table[1].used = 1;
    fd_table[2].used = 1;
}

/* --- common stub for resolving paths to driver and getting relative fs path --- */
static vfs_mount_t* vfs_resolve_mount(const char* path, const char** out_relative_path) {
    if (path == NULL) return NULL;

    int mount_idx = -1;
    size_t longest_match = 0;

    for (int i = 0; i < 8; i++) {
        if (mount_table[i].driver == NULL) continue;

        size_t mnt_len = strlen(mount_table[i].path);

        // Match the mount path (e.g., "/dev") against the full path (e.g., "/dev/tty0")
        if (strncmp(mount_table[i].path, path, mnt_len) == 0) {
            // Ensure we don't partially match (e.g., "/res" matching "/resource")
            // If the mount path is "/", mnt_len is 1, and path[1] check is safe.
            if (path[mnt_len] == '/' || path[mnt_len] == '\0' || strcmp(mount_table[i].path, "/") == 0) {
                if (mnt_len >= longest_match) {
                    longest_match = mnt_len;
                    mount_idx = i;
                }
            }
        }
    }

    if (mount_idx == -1) return NULL;

    // Calculate relative path for the driver
    const char* rel = path + longest_match;
    while (*rel == '/') rel++; // Strip leading slashes

    if (out_relative_path) *out_relative_path = rel;

    return &mount_table[mount_idx];
}

int vfs_mount(const char* target, vfs_driver_t* driver) {
    if (target == NULL || driver == NULL) return -1;

    for (int i = 0; i < 8; i++) {
        if (mount_table[i].driver == NULL) {
            strncpy(mount_table[i].path, target, MAX_PATH_LEN);
            mount_table[i].driver = driver;
            // Call
            return 0;
        }
    }

    return -1;
}

int vfs_unmount(const char* target) {
    if (target == NULL) return -1;

    for (int i = 0; i < 8; i++) {
        if (mount_table[i].driver != NULL && strcmp(mount_table[i].path, target) == 0) {
            // INFO: does not check if files are in use
            mount_table[i].driver = NULL;
            mount_table[i].fs_root = NULL;
            memset(mount_table[i].path, 0, MAX_PATH_LEN);
            return 0;
        }
    }

    return -1;
}

int vfs_open(const char* path, int flags) {
    if (path == NULL) return -1;

    const char* rel_path;
    vfs_mount_t* mnt = vfs_resolve_mount(path, &rel_path);
    if (!mnt) return -1;

    // Find free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!fd_table[i].used) {
            fd = i;
            break;
        }
    }
    if (fd == -1) return -1;

    // get file descriptor
    vfs_fd_t* file = &fd_table[fd];
    file->driver = mnt->driver;
    file->flags = flags;
    // file->offset = 0; // NOTE: lseek should do this

    if (file->driver->open(file, rel_path) < 0) {
        return -1;
    }

    file->used = 1;
    return fd;
}

int vfs_close(const int fd) {
    if (fd >= 0 && fd < MAX_OPEN_FILES) {
        vfs_fd_t* file = &fd_table[fd];
        if (file->driver->close != NULL) file->driver->close(file);
        file->used = 0;
        file->offset = 0;
        return 0;
    }
    return -1;
}

int vfs_read(const int fd, void* buf, const size_t n) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd].used) return -1;

    vfs_fd_t* file = &fd_table[fd];

    int bytes_read = file->driver->read(file, buf, n);

    if (bytes_read > 0) {
        file->offset += bytes_read;
    }

    return bytes_read;
}

int vfs_write(const int fd, const void* buf, const size_t n) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd].used) return -1;

    vfs_fd_t* file = &fd_table[fd];
    
    int bytes_written = file->driver->write(file, buf, n);

    if (bytes_written == -1) return -1;

    file->offset += bytes_written;

    return bytes_written;
}

int vfs_lseek(int fd, int offset, int whence) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd].used) return -1;

    vfs_fd_t* file = &fd_table[fd];

    if (whence == 0)      file->offset = offset;          // SEEK_SET
    else if (whence == 1) file->offset += offset;         // SEEK_CUR
    else return -1;

    return file->offset;
}

int vfs_mkdir(const char* path) {
    const char* rel_path;
    vfs_mount_t* mnt = vfs_resolve_mount(path, &rel_path);

    // Note: mkdir usually needs an fd or a specialized path call.
    // If your interface uses fd->driver->mkdir(fd, path):
    if (!mnt || !mnt->driver->mkdir) return -1;

    return mnt->driver->mkdir(rel_path);
}

int vfs_stat(const char* path, vfs_stat_t* st)
{
    const char* rel_path;
    vfs_mount_t* mnt = vfs_resolve_mount(path, &rel_path);

    if (!mnt || !mnt->driver->stat) return -1;
    return mnt->driver->stat(rel_path, st);
}

int vfs_fstat(int fd, vfs_stat_t* st) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) return -1;

    vfs_fd_t* file = &fd_table[fd];

    return file->driver->fstat(file, st);
}