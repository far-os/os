#include "fs.h"
#include "misc.h"

#pragma once

#define ATA_REG_DATA 0x1f0
// error is on read, feature is on write
#define ATA_REG_ERROR 0x1f1
#define ATA_REG_FEATURE 0x1f1
#define ATA_REG_SECTOR_COUNT 0x1f2
#define ATA_REG_LBA_LO 0x1f3
#define ATA_REG_LBA_MID 0x1f4
#define ATA_REG_LBA_HI 0x1f5
#define ATA_REG_DRIVE 0x1f6
// status is on read, command is on write
#define ATA_REG_STATUS 0x1f7
#define ATA_REG_CMD 0x1f7

#define READ_SECTORS 0x20
#define WRITE_SECTORS 0x30
#define CACHE_FLUSH 0xe7
#define IDENTIFY 0xec

// TODO: in the event i wish to do multiple disks, this all needs to be revamped.
extern unsigned short *ata_identity;
typedef bool master_slave_selector;
#define DISK_MASTER 0
#define DISK_SLAVE 1

// ata_identity[0] is apparently useful if disk is not a harddisk (idk how it's useful, grrrr osdev wiki)
#define ATA_HAS_LBA48 ( ata_identity[83] & (1 << 10) )
#define ATA_N_LBA48 ( *((unsigned long long *) &ata_identity[100]) )
#define ATA_N_LBA28 ( *((unsigned *) &ata_identity[60]) )

// 80 conductor cable? idk, im just macroing what osdev wiki says to
// is apparently udma stuff, so i shall keep it
// NOTE: this *must* be from the master, doesn't make sense otherwise (see the TODO above).
#define ATA_HAS_80CC ( ata_identity[93] & (1 << 11) )

// lo byte is supported modes, hi byte is ones in use. if using udma, hope they're equal else you'll have pqin
#define ATA_UDMA_MODES ( ata_identity[88] )

void ata_identify(void *addr, master_slave_selector drv);
void read_pio28(void *addr, unsigned int lba, unsigned char len, master_slave_selector drv);
void write_pio28(void *data, unsigned int lba, unsigned char len, master_slave_selector drv);
