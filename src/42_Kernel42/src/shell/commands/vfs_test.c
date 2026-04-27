#include <drivers/video/vga_text.h>
#include <kernel/filesystem/vfs.h>
#include <kernel/log.h>
#include <kernel/pit.h>
#include <stdio.h>
#include <string.h>

#include "shell/shell_command.h"

static void vfs_debug_tree(const char *path, int level) {
  if (level == 0) {
    vga_text_setcolor(VGA_COLOR_LIGHT_GREEN | VGA_COLOR_BLACK << 4);
    printf("/ (first level)\n");
    vga_text_setcolor(VGA_COLOR_WHITE | VGA_COLOR_BLACK << 4);
  }

  int fd = vfs_open(path, O_READ);
  if (fd == VFS_INVALID_FD)
    return;

  vfs_dirent_t entry;
  while (vfs_read(fd, &entry, 1) > 0) {
    for (int i = 0; i <= level; i++)
      printf("  ");

    if (entry.type == VFS_DIRECTORY) {
      vga_text_setcolor(VGA_COLOR_LIGHT_GREEN | VGA_COLOR_BLACK << 4);
      printf("/%s\n", entry.name);
      vga_text_setcolor(VGA_COLOR_WHITE | VGA_COLOR_BLACK << 4);

      char next_path[MAX_PATH_LEN];
      int len = strlen(path);
      memcpy(next_path, path, len);

      if (path[len - 1] != '/') {
        next_path[len] = '/';
        len++;
      }
      strcpy(next_path + len, entry.name);

      vfs_debug_tree(next_path, level + 1);
    } else {
      printf("%s\n", entry.name);
    }
  }

  vfs_close(fd);
}

int cmd_vfs_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  vfs_stat_t st;
  char buffer[64];
  int fd;
  log_info("--- Starting VFS Integration Tests ---\n");

  if (vfs_mkdir("/test_dir") == 0) {
    if (vfs_stat("/test_dir", &st) == 0 && st.type == VFS_DIRECTORY) {
      log_info("[PASS] mkdir and stat (directory)\n");
    }
  } else {
    log_error("[FAIL] mkdir /test_dir\n");
  }

  fd = vfs_open("/test_dir/hello.txt", O_READ | O_WRITE | O_CREAT);
  if (fd != VFS_INVALID_FD) {
    log_info("[PASS] open O_CREAT in subdirectory (FD: %d)\n", fd);

    vfs_write(fd, "VFS_TEST data", 13);
    if (vfs_fstat(fd, &st) == 0 && st.size == 13) {
      log_info("[PASS] write and fstat (size: %d)\n", st.size);
    }

    vfs_lseek(fd, 4, 0);
    memset(buffer, 0, 64);
    int bytes = vfs_read(fd, buffer, 4);
    if (bytes == 4 && strcmp(buffer, "TEST") == 0) {
      log_info("[PASS] lseek and read (data: '%s')\n", buffer);
    }

    if (vfs_close(fd) == 0) {
      log_info("[PASS] close\n");
    }
  } else {
    log_error("[FAIL] O_CREAT /test_dir/hello.txt\n");
  }

  if (vfs_stat("/test_dir/hello.txt", &st) == 0) {
    log_info("[PASS] stat (file) size: %d, inode: %x\n", st.size, st.inode_num);
  } else {
    log_error("[FAIL] stat /test_dir/hello.txt\n");
  }

  log_info("Testing Directory Read on /test_dir:\n");
  fd = vfs_open("/test_dir", O_READ);
  if (fd != VFS_INVALID_FD) {
    vfs_dirent_t entry;
    int count = 0;

    while (vfs_read(fd, &entry, 1) > 0) {
      log_info("  - Found: %s (Type: %s, Inode: %x)\n", entry.name,
               (entry.type == VFS_DIRECTORY ? "DIR" : "FILE"), entry.inode_num);
      count++;
    }

    if (count >= 1) {
      log_info("[PASS] Directory read returned %d entries\n", count);
    } else {
      log_error("[FAIL] Directory read only found %d entries\n", count);
    }

    vfs_close(fd);
  } else {
    log_error("[FAIL] Could not open directory /test_dir for reading\n");
  }

  log_info("--- VFS Tests Complete ---\n");
  return 0;
}

int cmd_tree(int argc, char** argv) {
  const char* path = "/";
  if (argc > 1) {
    path = argv[1];
  }
  vfs_debug_tree(path, 0);
  return 0;
}