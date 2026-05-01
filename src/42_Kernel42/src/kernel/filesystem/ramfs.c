#include <kernel/filesystem/ramfs.h>

#include "stddef.h"
#include "stdlib.h"
#include "string.h"

static ram_inode_t *ram_root = NULL;

void ramfs_init(const char *root_path) {
    ram_root = (ram_inode_t *) malloc(sizeof(ram_inode_t));
    memset(ram_root, 0, sizeof(ram_inode_t));
    strcpy(ram_root->name, root_path);
    ram_root->type = VFS_DIRECTORY;

    ram_inode_t *test_file = (ram_inode_t *) malloc(sizeof(ram_inode_t));
    memset(test_file, 0, sizeof(ram_inode_t));
    strcpy(test_file->name, "test.txt");
    test_file->type = VFS_FILE;

    // Link it as the first child of root
    ram_root->children = test_file;

    vfs_mount("/", ramfs_get_driver());
}

/* --- private helper --- */
static void *ramfs_realloc(void *old_ptr, size_t old_size, size_t new_size) {
    void *new_ptr = malloc(new_size);
    if (!new_ptr) return NULL;

    if (old_ptr) {
        // We only copy the minimum of the two sizes
        size_t copy_size = (old_size < new_size) ? old_size : new_size;
        memcpy(new_ptr, old_ptr, copy_size);
        free(old_ptr);
    }
    return new_ptr;
}

static ram_inode_t *ramfs_find_child(ram_inode_t *dir, const char *name) {
    if (dir->type != VFS_DIRECTORY) return NULL;

    ram_inode_t *curr = dir->children;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

static ram_inode_t *ramfs_resolve(const char *path) {
    if (!path || path[0] == '\0') return ram_root;

    ram_inode_t *curr = ram_root;
    char buffer[MAX_PATH_LEN];
    strncpy(buffer, path, MAX_PATH_LEN);

    // We use a simple pointer-based "tokenization" instead of strtok
    // to avoid modifying the original string buffer if possible
    const char *start = path;
    while (*start) {
        // Skip leading slashes
        while (*start == '/') start++;
        if (*start == '\0') break;

        // Extract the next component name
        char component[MAX_NAME_LEN] = {0};
        int i = 0;
        while (*start && *start != '/' && i < MAX_NAME_LEN - 1) {
            component[i++] = *start++;
        }

        // Look for this component in the current directory
        curr = ramfs_find_child(curr, component);
        if (!curr) return NULL; // Component not found
    }

    return curr;
}

static int ramfs_driver_open(vfs_fd_t *fd, const char *path) {
    ram_inode_t *node = ramfs_resolve(path);

    if (!node) {
        // If O_CREAT is set in fd->flags, you'd implement creation logic here
        if (fd->flags & O_CREAT) {
            //  Find the parent directory
            // We need to find the last '/' to separate the directory from the filename
            char parent_path[MAX_PATH_LEN] = {0};
            const char *last_slash = strrchr(path, '/');

            const char *filename;
            if (!last_slash) {
                // No slash means it's in the root
                strcpy(parent_path, "/");
                filename = path;
            } else {
                // Extract parent path (everything before the last slash)
                size_t len = last_slash - path;
                if (len == 0) len = 1; // Handle "/file.txt" -> root is "/"
                strncpy(parent_path, path, len);
                filename = last_slash + 1;
            }

            ram_inode_t *parent = ramfs_resolve(parent_path);
            if (!parent || parent->type != VFS_DIRECTORY) {
                return -1; // Parent directory doesn't exist
            }

            //  Allocate and initialize the new inode
            ram_inode_t *new_node = (ram_inode_t *) malloc(sizeof(ram_inode_t));
            if (!new_node) return -1;

            memset(new_node, 0, sizeof(ram_inode_t));
            strncpy(new_node->name, filename, MAX_NAME_LEN - 1);
            new_node->type = VFS_FILE;
            new_node->size = 0;
            new_node->capacity = 0;
            new_node->data = NULL;

            //  Link the new node into the parent's child list
            // We insert at the head of the linked list for simplicity
            new_node->next = parent->children;
            parent->children = new_node;

            node = new_node;
        } else {
            return -1;
        }
    }

    // We store our internal inode in the VFS's generic pointer.
    fd->private_data = node;

    return 0;
}

static int ramfs_driver_read(vfs_fd_t *fd, void *buf, size_t n) {
    ram_inode_t *inode = fd->private_data;

    //  Safety check: Can't read a directory like a file
    if (inode->type != VFS_DIRECTORY && inode->type != VFS_FILE) return -1;
    if (inode->type == VFS_DIRECTORY) {
        if (n < 1 || buf == NULL) return 0;

        vfs_dirent_t *out = (vfs_dirent_t *) buf;
        ram_inode_t *curr = inode->children; // Access the union member for dirs

        // Skip siblings until we reach the current offset
        for (uint32_t i = 0; i < fd->offset && curr != NULL; i++) {
            curr = curr->next;
        }

        if (!curr) return 0; // End of directory

        // Fill the dirent structure
        strncpy(out->name, curr->name, MAX_NAME_LEN);
        out->name[MAX_NAME_LEN - 1] = '\0'; // Safety null-termination
        out->type = curr->type;
        out->inode_num = (uint32_t) curr;

        return 1; // Returning 1 entry
    }

    //  EOF Check: Are we already at or past the end?
    if (fd->offset >= inode->size) return 0;

    //  Clamp 'n': Don't read past the end of the file
    if (fd->offset + n > inode->size) {
        n = inode->size - fd->offset;
    }

    //  Perform the copy
    memcpy(buf, inode->data + fd->offset, n);

    return (int) n;
}

static int ramfs_driver_write(vfs_fd_t *fd, const void *buf, size_t n) {
    ram_inode_t *inode = fd->private_data;

    if (inode->type != VFS_FILE) return -1;

    //  Expansion Logic: Do we need more memory?
    if (fd->offset + n > inode->capacity) {
        uint32_t new_capacity = fd->offset + n + 1024; // Allocate extra to avoid spamming realloc
        void *new_data = ramfs_realloc(inode->data, inode->capacity, new_capacity);
        if (!new_data) return -1; // Out of memory!

        inode->data = (uint8_t *) new_data;
        inode->capacity = new_capacity;
    }

    //  Perform the copy
    memcpy(inode->data + fd->offset, buf, n);

    //  Update size if we extended the file
    if (fd->offset + n > inode->size) {
        inode->size = fd->offset + n;
    }

    return (int) n;
}

static int ramfs_driver_mkdir(const char *path) {
    //  Check if it already exists
    if (ramfs_resolve(path) != NULL) return -1;

    //  Find parent
    char parent_path[MAX_PATH_LEN] = {0};
    const char *last_slash = strrchr(path, '/');
    const char *dirname;

    if (!last_slash) {
        strcpy(parent_path, "/");
        dirname = path;
    } else {
        size_t len = last_slash - path;
        if (len == 0) len = 1;
        strncpy(parent_path, path, len);
        dirname = last_slash + 1;
    }

    ram_inode_t *parent = ramfs_resolve(parent_path);
    if (!parent || parent->type != VFS_DIRECTORY) return -1;

    //  Create the directory inode
    ram_inode_t *new_dir = (ram_inode_t *) malloc(sizeof(ram_inode_t));
    if (!new_dir) return -1;

    memset(new_dir, 0, sizeof(ram_inode_t));
    strncpy(new_dir->name, dirname, MAX_NAME_LEN - 1);
    new_dir->type = VFS_DIRECTORY;

    //  Link to parent
    new_dir->next = parent->children;
    parent->children = new_dir;

    return 0;
}

static int ramfs_driver_stat(const char *path, vfs_stat_t *st) {
    ram_inode_t *inode = ramfs_resolve(path);
    if (!inode) return -1; // File not found

    st->type = inode->type;
    st->size = inode->size;
    st->inode_num = (uint32_t) inode;

    return 0;
}

static int ramfs_driver_fstat(vfs_fd_t *fd, vfs_stat_t *st) {
    ram_inode_t *inode = fd->private_data;

    st->size = inode->size;
    st->type = inode->type;
    st->inode_num = (uint32_t) inode; // Using the memory address as a unique ID

    return 0;
}

static vfs_driver_t ram_driver = {
    .read = ramfs_driver_read,
    .write = ramfs_driver_write,
    .open = ramfs_driver_open,
    .close = NULL, // RAMFS doesn't really need to do anything on close
    .mkdir = ramfs_driver_mkdir,
    .move = NULL,
    .remove = NULL,
    .stat = ramfs_driver_stat,
    .fstat = ramfs_driver_fstat,
};

vfs_driver_t *ramfs_get_driver() {
    return &ram_driver;
}
