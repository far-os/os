#include "fs.h"

#pragma once

#define READ_SECTORS 0x20
#define WRITE_SECTORS 0x30
#define CACHE_FLUSH 0xe7

void read_pio28(void *addr, unsigned int lba, unsigned int len, unsigned char drv);
void write_pio28(void *data, unsigned int lba, unsigned int len, unsigned char drv);
