#pragma once

#include "../../include/libc/stdbool.h"
#include "../../include/libc/stdint.h"
#include "../io/io.h"

#define ATA_SECTOR_SIZE 512U

#define ATA_PRIMARY_IO_BASE 0x1F0U
#define ATA_PRIMARY_CONTROL_BASE 0x3F6U

#define ATA_REG_DATA 0x00U
#define ATA_REG_SECTOR_COUNT 0x02U
#define ATA_REG_LBA_LOW 0x03U
#define ATA_REG_LBA_MID 0x04U
#define ATA_REG_LBA_HIGH 0x05U
#define ATA_REG_DRIVE_SELECT 0x06U
#define ATA_REG_STATUS_COMMAND 0x07U

#define ATA_STATUS_ERR 0x01U
#define ATA_STATUS_DRQ 0x08U
#define ATA_STATUS_DF 0x20U
#define ATA_STATUS_BSY 0x80U

#define ATA_CMD_READ_SECTORS 0x20U
#define ATA_CMD_WRITE_SECTORS 0x30U
#define ATA_CMD_CACHE_FLUSH 0xE7U

/** \brief Read a disk sector
 * 
 * This is done over ATA PIO, which means that bytes are read over I/O ports uring inw.
 * \returns true on sucsess, otherwise: false
 * \param lba Location on the disk to read from
 * \param buffer Where the read data is put
 */
bool ata_read_sector(uint32_t lba, uint8_t buffer[ATA_SECTOR_SIZE]);

/** \brief Write to a disk sector
 * \param lba Destination byte address on disk
 * \param buffer Data to be written
 * \returns false on error, otherwise: ata_poll(false)
 */
bool ata_write_sector(uint32_t lba, const uint8_t buffer[ATA_SECTOR_SIZE]);

/** \brief Check if the drive is ready before disk read/write.
 * This uses pooling so it is blocking.
 * \note This is blocking until the disk is ready
 * \returns true when done, false if there was an error.
 * \see https://wiki.osdev.org/ATA_PIO_Mode#Polling_the_Status_vs._IRQs
 * \see https://wiki.osdev.org/ATA_PIO_Mode#28_bit_PIO
 */
bool ata_poll(bool wait_for_drq);

/** \brief Just runs inb() against ATA disk 4 times
 * \see https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays
 */
void ata_delay_400ns(void);

/** \brief Select drive and/or head */
void ata_select_drive(uint32_t lba);

/** \brief Move HDD head to the correct position on the disk for I/O */
void ata_program_lba(uint32_t lba);