#pragma once

#include "../../include/libc/stdbool.h"
#include "../../include/libc/stdint.h"

#include "../disk/ata.h"
#include "../memory/heap.h"
#include "../printing/printing.h"
#include "../string/string.h"

#define TINYFS_MAGIC 0x53465954U
#define TINYFS_VERSION 1U
#define TINYFS_NAME_LENGTH 16U
#define TINYFS_MAX_FILES 64U

#define TINYFS_SUPERBLOCK_SECTOR 0U
#define TINYFS_FILE_TABLE_START 1U
#define TINYFS_FILE_TABLE_SECTORS 4U
#define TINYFS_DATA_START (TINYFS_FILE_TABLE_START + TINYFS_FILE_TABLE_SECTORS)

/** \brief Status codes for some tinyfs related funtions */
enum tinyfs_status_codes{
    TINYFS_STATUS_OK = 0,
    TINYFS_STATUS_FAILED_TO_WRITE_SECTOR_10 = -17,
    TINYFS_STATUS_DISKTEST_INVALID_USAGE = -18,
    TINYFS_STATUS_FAILED_TO_READ_SECTOR_10 = -19,
    TINYFS_STATUS_NOT_ENOUGH_MEMORY_TO_BUILD_FILE_CONTENT = -20,
    TINYFS_STATUS_DISK_ERROR = -21,
    TINYFS_STATUS_NOT_FORMATTED = -22,
    TINYFS_STATUS_FILE_NOT_FOUND = -23,
    TINYFS_STATUS_NO_SPACE = -24,
    TINYFS_STATUS_INVALID_NAME = -25,
    TINYFS_STATUS_OUT_OF_MEMORY = -26
};

/** \brief First thing stored on the disk for the filesystem */
typedef struct {
    /** An 'ID' for the filesystem on the disk */
    uint32_t magic;
    /** FS version */
    uint32_t version;
    uint32_t file_table_start;
    uint32_t file_table_sectors;
    uint32_t data_start;
    uint32_t next_free_sector;
    uint8_t reserved[ATA_SECTOR_SIZE - (6U * sizeof(uint32_t))];
} TinyFsSuperblock;

/** \brief On disk struct with file metadata 
 * 
 * The file table containes a list of these
 */
typedef struct {
    char name[TINYFS_NAME_LENGTH];
    uint32_t start_sector;
    uint32_t size_bytes;
    uint8_t used;
    uint8_t reserved[7];
} TinyFsFileEntry;

void tinyfs_init(void);
bool tinyfs_is_ready(void);
int8_t tinyfs_format(void);
int8_t tinyfs_list(void);
int8_t tinyfs_write_file(const char name[], const char content[]);
int8_t tinyfs_read_file(const char name[], char** out_content, uint32_t* out_size);
