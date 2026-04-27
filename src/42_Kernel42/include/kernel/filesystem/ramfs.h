#pragma once

#include "kernel/filesystem/vfs.h"

/* --- Configuration --- */
#define MAX_NAME_LEN 32

/* --- Node Types --- */
typedef enum {
    RAMFS_FILE,
    RAMFS_DIR
} ramfs_type_t;

typedef struct ram_inode {
    char name[MAX_NAME_LEN];
    uint32_t size;          // Current size in bytes
    uint32_t capacity;      // Allocated buffer size (to avoid constant realloc)
    vfs_type_t type;        // VFS_FILE or VFS_DIRECTORY
    // OR a list of children (for directories)
    union {
        uint8_t* data;              // If it's a file
        struct ram_inode* children; // If it's a directory (head of a linked list)
    };

    // For directories, you'll eventually need a linked list or array of children
    struct ram_inode* next;
} ram_inode_t;

void ramfs_init(const char* root_path);

vfs_driver_t* ramfs_get_driver();