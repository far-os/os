#include "port.h"

#ifndef ATA_H
#define ATA_H

#define READ_SECTORS 0x20

void read_pio28(void *addr, unsigned int lba, unsigned char len, unsigned char drv) {
  // id: high nybble is E for master or F for slave - how nybble in highest nybble of LBA
  pbyte_out(0x1f6, 0xe0 | (drv << 4) | ((lba >> 24) & 0x0f));

  // is optional and wastes time
  // pbyte_out(0x1f1, 0x00);

  // sector count
  pbyte_out(0x1f2, len);

  // lba
  pbyte_out(0x1f3, lba & 0xff);
  pbyte_out(0x1f4, (lba >> 8) & 0xff);
  pbyte_out(0x1f5, (lba >> 16) & 0xff);

  // send command
  pbyte_out(0x1f7, READ_SECTORS);

  // poll until ready
  while (!(pbyte_in(0x1f7) & 0x08));

  // read!
  rep_insw(0x1f0, len << 8, addr);
}

#endif
