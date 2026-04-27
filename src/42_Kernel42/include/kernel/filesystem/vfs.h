#pragma once

#include <stdint.h>

/* --- Constants --- */
#define MAX_PATH_LEN    256
#define MAX_OPEN_FILES  32   // How many files the system can handle at once
#define VFS_INVALID_FD  (-1)

/* --- Flags for vfs_open --- */
#define O_READ   0x1
#define O_WRITE  0x2
#define O_CREAT  0x4

// forwards declaration
typedef struct vfs_fd vfs_fd_t;
typedef struct vfs_stat vfs_stat_t;

/* --- The "Interface" --- */
// Every filesystem (ramfs, binfs) implements these functions.
typedef struct vfs_driver {
    int (*read)(vfs_fd_t* fd, void* buf, size_t n);
    int (*write)(vfs_fd_t* fd, const void* buf, size_t n);
    int (*close)(vfs_fd_t* fd);
    int (*open)(vfs_fd_t* fd, const char* path);
    int (*mkdir)(const char* path);
    int (*move)(vfs_fd_t* src, const char* dst);
    int (*remove)(vfs_fd_t* fd, const char* path);
    int (*stat)(const char* rel_path, vfs_stat_t* st);
    int (*fstat)(vfs_fd_t* fd, vfs_stat_t* stat);
} vfs_driver_t;

/* --- File Descriptor --- */
struct vfs_fd {
    void* private_data;     // Points to the specific FS's inode (ram_inode_t)
    vfs_driver_t* driver;   // The "vtable" - tells VFS which functions to use
    uint32_t offset;        // Bytes read for files and children read for directories
    int flags;
    int used;
};

/* --- File Types --- */
typedef enum {
    VFS_FILE,
    VFS_DIRECTORY
} vfs_type_t;

/* --- File Metadata (Stat) --- */
// Used by users to get info about a file without opening it
struct vfs_stat {
    uint32_t size;
    vfs_type_t type;
    uint32_t inode_num;
};

/* --- Mount Point --- */
// This maps a path to a driver
typedef struct {
    char path[MAX_PATH_LEN];
    vfs_driver_t* driver;
    void* fs_root; // Pointer to the root of this specific filesystem instance
} vfs_mount_t;

/* --- Directory Entity (dirent) --- */
// Used to represent directories
typedef struct vfs_dirent {
    char name[MAX_PATH_LEN];
    vfs_type_t type;
    uint32_t inode_num;
} vfs_dirent_t;

/* --- Public VFS APIs --- */
void vfs_init(void);

int vfs_mount(const char* target, vfs_driver_t* driver);
int vfs_unmount(const char* target);

int vfs_open(const char* path, int flags);
int vfs_close(int fd);

int vfs_read(int fd, void* buf, size_t n);
int vfs_write(int fd, const void* buf, size_t n);
int vfs_lseek(int fd, int offset, int whence);
int vfs_mkdir(const char* path);

int vfs_stat(const char* path, vfs_stat_t* st);
int vfs_fstat(int fd, vfs_stat_t* st);
