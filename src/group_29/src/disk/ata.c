#include "ata.h"
#include "../io/io.h"

void ata_delay_400ns(void) {
    inb(ATA_PRIMARY_CONTROL_BASE);
    inb(ATA_PRIMARY_CONTROL_BASE);
    inb(ATA_PRIMARY_CONTROL_BASE);
    inb(ATA_PRIMARY_CONTROL_BASE);
}

bool ata_poll(bool wait_for_drq) {
    uint8_t status;

    ata_delay_400ns();

    do {
        status = inb(ATA_PRIMARY_IO_BASE + ATA_REG_STATUS_COMMAND);
    } while ((status & ATA_STATUS_BSY) != 0U);

    if ((status & (ATA_STATUS_ERR | ATA_STATUS_DF)) != 0U) {
        return false;
    }

    if (wait_for_drq && (status & ATA_STATUS_DRQ) == 0U) {
        return false;
    }

    return true;
}

void ata_select_drive(uint32_t lba) {
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_DRIVE_SELECT, (uint8_t)(0xF0U | ((lba >> 24) & 0x0FU)));
    ata_delay_400ns();
}

void ata_program_lba(uint32_t lba) {
    ata_select_drive(lba);
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_SECTOR_COUNT, 1U);
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_LBA_LOW, (uint8_t)(lba & 0xFFU));
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_LBA_MID, (uint8_t)((lba >> 8) & 0xFFU));
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_LBA_HIGH, (uint8_t)((lba >> 16) & 0xFFU));
}

bool ata_read_sector(uint32_t lba, uint8_t buffer[ATA_SECTOR_SIZE]) {
    uint16_t* words = (uint16_t*)buffer;
    uint16_t index;

    ata_program_lba(lba);
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_STATUS_COMMAND, ATA_CMD_READ_SECTORS);

    if (!ata_poll(true)) {
        return false;
    }

    for (index = 0U; index < (ATA_SECTOR_SIZE / 2U); ++index) {
        words[index] = inw(ATA_PRIMARY_IO_BASE + ATA_REG_DATA);
    }

    return true;
}

bool ata_write_sector(uint32_t lba, const uint8_t buffer[ATA_SECTOR_SIZE]) {
    const uint16_t* words = (const uint16_t*)buffer;
    uint16_t index;

    ata_program_lba(lba);
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_STATUS_COMMAND, ATA_CMD_WRITE_SECTORS);

    if (!ata_poll(true)) {
        return false;
    }

    for (index = 0U; index < (ATA_SECTOR_SIZE / 2U); ++index) {
        outw(ATA_PRIMARY_IO_BASE + ATA_REG_DATA, words[index]);
    }

    outb(ATA_PRIMARY_IO_BASE + ATA_REG_STATUS_COMMAND, ATA_CMD_CACHE_FLUSH);
    return ata_poll(false);
}
