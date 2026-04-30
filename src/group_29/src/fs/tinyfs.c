#include "./tinyfs.h"

static bool tinyfs_ready;

/** \brief Fills a buffer with NULL
 * \param buffer This will be all 0 when this function is done
 * \param size Size of the buffer
 */
static void buffer_clear(uint8_t* buffer, uint32_t size) {
    uint32_t index;

    for (index = 0U; index < size; ++index) {
        buffer[index] = 0U;
    }
}

/** \brief Compares two strings
 * \returns 0 if they're equal, otherwise it's the difference between the two chars at the index where they diverge
 * \param left String that is TINYFS_NAME_LENGTH.
 * \param right Same as left
 * \warning This assumes that the string is TINYFS_NAME_LENGTH, not less and not more
 */
static int16_t name_compare(const char left[], const char right[]) {
    uint32_t index;

    for (index = 0U; index < TINYFS_NAME_LENGTH; ++index) {
        if (left[index] != right[index]) {
            return (int16_t)((unsigned char)left[index] - (unsigned char)right[index]);
        }

        if (left[index] == '\0') {
            return 0;
        }
    }

    return 0;
}

/** \brief Check that strlength(name) fits inside TINYFS_NAME_LENGTH and that it's not NULL or starts with NULL
 * \returns true if the check passes
*/
static bool name_is_valid(const char name[]) {
    unsigned int length;

    if (name == NULL || name[0] == '\0') {
        return false;
    }

    length = strlength(name);
    return length > 0 && length < TINYFS_NAME_LENGTH;
}

/** \brief Copies source to destination so that destination is TINYFS_NAME_LENGTH
 * 
 * If source is longer than destinetion it will be cut off in detination.
 * It will also be zero padded if it's shorter.
 * \param destination A TINYFS_NAME_LENGTH string
 * \param source The destination string
 */
static void name_copy(char destination[TINYFS_NAME_LENGTH], const char source[]) {
    uint32_t index = 0U;

    while (index + 1U < TINYFS_NAME_LENGTH && source[index] != '\0') {
        destination[index] = source[index];
        ++index;
    }

    while (index < TINYFS_NAME_LENGTH) {
        destination[index] = '\0';
        ++index;
    }
}

/** \brief Calculate how many sectors size_bytes takes up */
static uint32_t sectors_for_size(uint32_t size_bytes) {
    if (size_bytes == 0U) {
        return 1U;
    }

    return (size_bytes + ATA_SECTOR_SIZE - 1U) / ATA_SECTOR_SIZE;
}

/** \brief Reads the super block and checks validity
 * \param superblock The destination the superblock will be read into
 * \returns true if the superblock is valid, false if not
 */
static bool load_superblock(TinyFsSuperblock* superblock) {
    if (!ata_read_sector(TINYFS_SUPERBLOCK_SECTOR, (uint8_t*)superblock)) {
        return false;
    }

    if (superblock->magic != TINYFS_MAGIC || superblock->version != TINYFS_VERSION) {
        return false;
    }

    if (superblock->file_table_start != TINYFS_FILE_TABLE_START ||
        superblock->file_table_sectors != TINYFS_FILE_TABLE_SECTORS ||
        superblock->data_start != TINYFS_DATA_START ||
        superblock->next_free_sector < TINYFS_DATA_START) {
        return false;
    }

    return true;
}

/** \brief Write superblock to disk */
static bool save_superblock(const TinyFsSuperblock* superblock) {
    return ata_write_sector(TINYFS_SUPERBLOCK_SECTOR, (const uint8_t*)superblock);
}

/** \brief Reads the file table into entries
 * \param entries the file entry destination
 * \returns false if the read op failed, otherwise true.
 */
static bool load_file_table(TinyFsFileEntry entries[TINYFS_MAX_FILES]) {
    uint32_t sector;

    for (sector = 0U; sector < TINYFS_FILE_TABLE_SECTORS; ++sector) {
        if (
            !ata_read_sector(TINYFS_FILE_TABLE_START + sector,((uint8_t*)entries) + (sector * ATA_SECTOR_SIZE))
        ) {
            return false;
        }
    }

    return true;
}

/** \brief Save the file table
 * \param entries File table to save
 * \returns false if save failed, otherwise: true
 */
static bool save_file_table(const TinyFsFileEntry entries[TINYFS_MAX_FILES]) {
    uint32_t sector;

    for (sector = 0U; sector < TINYFS_FILE_TABLE_SECTORS; ++sector) {
        if (
            !ata_write_sector(TINYFS_FILE_TABLE_START + sector, ((const uint8_t*)entries) + (sector * ATA_SECTOR_SIZE))
        ) {
            return false;
        }
    }

    return true;
}

/** \brief Loop through the file table until the file with specified name is found
 * \param entries the file table
 * \param name The name to look for
 * \returns The index of the file or -1 if it's not found
 */
static int32_t find_file_index(const TinyFsFileEntry entries[TINYFS_MAX_FILES], const char name[]) {
    uint32_t index;

    for (index = 0U; index < TINYFS_MAX_FILES; ++index) {
        if (entries[index].used != 0U && name_compare(entries[index].name, name) == 0) {
            return (int32_t)index;
        }
    }

    return -1;
}

/** \brief Loop through the file table until it finds a free index
 * \param entries The file table
 * \returns The index of the free entry or -1 if none is found
 */
static int32_t find_free_file_index(const TinyFsFileEntry entries[TINYFS_MAX_FILES]) {
    uint32_t index;

    for (index = 0U; index < TINYFS_MAX_FILES; ++index) {
        if (entries[index].used == 0U) {
            return (int32_t)index;
        }
    }

    return -1;
}

/** \brief Tries to load the superblock and stores if it's successfull in the tinyfs_ready
 * 
 * \ref tinyfs_is_ready(void)
 */
void tinyfs_init(void) {
    TinyFsSuperblock superblock;
    tinyfs_ready = load_superblock(&superblock);
}

/** \brief Check tinyfs_ready
 * \returns tinyfs_ready
*/
bool tinyfs_is_ready(void) {
    return tinyfs_ready;
}

/** \brief Formats disk
 * \returns 0 if it worked, or a negative number defined by an unnamed struct in tinyfs.h
 * \see tinyfs_status_codes
 */
int8_t tinyfs_format(void) {
    TinyFsSuperblock superblock;
    TinyFsFileEntry entries[TINYFS_MAX_FILES];
    uint8_t zero_sector[ATA_SECTOR_SIZE];
    uint32_t sector;

    buffer_clear((uint8_t*)&superblock, sizeof(superblock));
    buffer_clear((uint8_t*)entries, sizeof(entries));
    buffer_clear(zero_sector, sizeof(zero_sector));

    superblock.magic = TINYFS_MAGIC;
    superblock.version = TINYFS_VERSION;
    superblock.file_table_start = TINYFS_FILE_TABLE_START;
    superblock.file_table_sectors = TINYFS_FILE_TABLE_SECTORS;
    superblock.data_start = TINYFS_DATA_START;
    superblock.next_free_sector = TINYFS_DATA_START;

    if (!save_superblock(&superblock)) {
        return TINYFS_STATUS_DISK_ERROR;
    }

    if (!save_file_table(entries)) {
        return TINYFS_STATUS_DISK_ERROR;
    }

    for (sector = TINYFS_DATA_START; sector < TINYFS_DATA_START + 4U; ++sector) {
        if (!ata_write_sector(sector, zero_sector)) {
            return TINYFS_STATUS_DISK_ERROR;
        }
    }

    tinyfs_ready = true;
    return TINYFS_STATUS_OK;
}

/** \brief Print listing of all files
 * \note No struct that can be programatically accessed is returned, the function prints directly to console
 * \returns tinyfs_status_codes
 */
int8_t tinyfs_list(void) {
    TinyFsSuperblock superblock;
    TinyFsFileEntry entries[TINYFS_MAX_FILES];
    uint32_t index;
    uint32_t listed = 0U;

    if (!load_superblock(&superblock)) {
        tinyfs_ready = false;
        return TINYFS_STATUS_NOT_FORMATTED;
    }

    if (!load_file_table(entries)) {
        return TINYFS_STATUS_DISK_ERROR;
    }

    tinyfs_ready = true;
    print(" Files:\n");

    for (index = 0U; index < TINYFS_MAX_FILES; ++index) {
        if (entries[index].used == 0U) {
            continue;
        }

        print(" ");
        print(entries[index].name);
        print(" (");
        char* size_text = format_string("%d", (int32_t)entries[index].size_bytes);
        if (size_text != NULL) {
            print(size_text);
            free(size_text);
        }
        print(" bytes)\n");
        ++listed;
    }

    if (listed == 0U) {
        print(" <empty>\n");
    }

    return TINYFS_STATUS_OK;
}

/** \brief Write to a file
 * 
 * Creates a new file or overwrites an existing one
 * \param name File name
 * \param content Bytes to be written
 * \returns tinyfs_status_codes
 */
int8_t tinyfs_write_file(const char name[], const char content[]) {
    TinyFsSuperblock superblock;
    TinyFsFileEntry entries[TINYFS_MAX_FILES];
    uint32_t size_bytes;
    uint32_t sector_count;
    uint32_t sector_offset;
    uint32_t source_index;
    int32_t entry_index;
    uint8_t sector_buffer[ATA_SECTOR_SIZE];

    if (!name_is_valid(name)) {
        return TINYFS_STATUS_INVALID_NAME;
    }

    if (!load_superblock(&superblock)) {
        tinyfs_ready = false;
        return TINYFS_STATUS_NOT_FORMATTED;
    }

    if (!load_file_table(entries)) {
        return TINYFS_STATUS_DISK_ERROR;
    }

    size_bytes = (content == NULL) ? 0U : (uint32_t)strlength(content);
    sector_count = sectors_for_size(size_bytes);
    entry_index = find_file_index(entries, name);

    if (entry_index < 0) {
        entry_index = find_free_file_index(entries);
        if (entry_index < 0) {
            return TINYFS_STATUS_NO_SPACE;
        }
    }

    for (sector_offset = 0U; sector_offset < sector_count; ++sector_offset) {
        buffer_clear(sector_buffer, sizeof(sector_buffer));

        for (source_index = 0U; source_index < ATA_SECTOR_SIZE; ++source_index) {
            uint32_t content_index = sector_offset * ATA_SECTOR_SIZE + source_index;
            if (content_index >= size_bytes) {
                break;
            }

            sector_buffer[source_index] = (uint8_t)content[content_index];
        }

        if (!ata_write_sector(superblock.next_free_sector + sector_offset, sector_buffer)) {
            return TINYFS_STATUS_DISK_ERROR;
        }
    }

    buffer_clear((uint8_t*)&entries[entry_index], sizeof(TinyFsFileEntry));
    entries[entry_index].used = 1U;
    entries[entry_index].start_sector = superblock.next_free_sector;
    entries[entry_index].size_bytes = size_bytes;
    name_copy(entries[entry_index].name, name);

    superblock.next_free_sector += sector_count;

    if (!save_file_table(entries)) {
        return TINYFS_STATUS_DISK_ERROR;
    }

    if (!save_superblock(&superblock)) {
        return TINYFS_STATUS_DISK_ERROR;
    }

    tinyfs_ready = true;
    return TINYFS_STATUS_OK;
}

/** \brief Read a file
 * \param name The name of the file to read
 * \param out_content Pointer to a pointer to the destination for the bytes from the file, will be written to
 * \param out_size Pointer to a byte count, will be written to
 * \returns tinyfs_status_codes
 */
int8_t tinyfs_read_file(const char name[], char** out_content, uint32_t* out_size) {
    TinyFsSuperblock superblock;
    TinyFsFileEntry entries[TINYFS_MAX_FILES];
    int32_t entry_index;
    uint32_t sector_count;
    uint32_t sector_offset;
    uint32_t byte_index;
    uint8_t sector_buffer[ATA_SECTOR_SIZE];
    char* content;

    if (out_content == NULL || out_size == NULL) {
        return TINYFS_STATUS_DISK_ERROR;
    }

    *out_content = NULL;
    *out_size = 0U;

    if (!load_superblock(&superblock)) {
        tinyfs_ready = false;
        return TINYFS_STATUS_NOT_FORMATTED;
    }

    if (!load_file_table(entries)) {
        return TINYFS_STATUS_DISK_ERROR;
    }

    entry_index = find_file_index(entries, name);
    if (entry_index < 0) {
        return TINYFS_STATUS_FILE_NOT_FOUND;
    }

    content = (char*)malloc(entries[entry_index].size_bytes + 1U);
    if (content == NULL) {
        return TINYFS_STATUS_OUT_OF_MEMORY;
    }

    sector_count = sectors_for_size(entries[entry_index].size_bytes);
    for (sector_offset = 0U; sector_offset < sector_count; ++sector_offset) {
        if (!ata_read_sector(entries[entry_index].start_sector + sector_offset, sector_buffer)) {
            free(content);
            return TINYFS_STATUS_DISK_ERROR;
        }

        for (byte_index = 0U; byte_index < ATA_SECTOR_SIZE; ++byte_index) {
            uint32_t content_index = sector_offset * ATA_SECTOR_SIZE + byte_index;
            if (content_index >= entries[entry_index].size_bytes) {
                break;
            }

            content[content_index] = (char)sector_buffer[byte_index];
        }
    }

    content[entries[entry_index].size_bytes] = '\0';
    *out_content = content;
    *out_size = entries[entry_index].size_bytes;
    tinyfs_ready = true;
    return TINYFS_STATUS_OK;
}
