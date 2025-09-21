#include "include/ata.h"
#include "include/err.h"
#include "include/fs.h"
#include "include/port.h"

// see TODO in header file
unsigned short *ata_identity = 0xd000;

static inline void ata_cache_flush() {
  pbyte_out(0x1f7, CACHE_FLUSH);
}

void ata_identify(void *addr, unsigned char drv) {
  // disk select: A0 for master or B0 for slave
  pbyte_out(0x1f6, 0xa0 | (drv << 4));

  // is optional and wastes time
  // pbyte_out(0x1f1, 0x00);

  // the great zero
  pbyte_out(0x1f2, 0);
  pbyte_out(0x1f3, 0);
  pbyte_out(0x1f4, 0);
  pbyte_out(0x1f5, 0);

  // send command
  pbyte_out(0x1f7, IDENTIFY);

  unsigned char p;
  // poll until ready
  while (!((p = pbyte_in(0x1f7)) & 0x09));

  if (p & 1) { // oh no bad
    msg(KERNERR, E_NOSTORAGE, "Failed to ATA IDENTIFY");
    return;
  }

  // read!
  rep_insw(0x1f0, 1 << 8, addr);
}

void read_pio28(void *addr, lba_n lba, unsigned int len, unsigned char drv) {
  // id: high nybble is E for master or F for slave - low nybble in highest nybble of LBA
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

  unsigned char p;
  for (unsigned int i = 0; i < len; i++) {
    // poll until ready
    while (!((p = pbyte_in(0x1f7)) & 0x09));

    if (p & 1) { // oh no bad
      msg(KERNERR, E_NOSTORAGE, "Failed to ATA READ28");
      return;
    }

    // read!
    rep_insw(0x1f0, 1 << 8, addr + (i << 9));
  }
}

void write_pio28(void *data, lba_n lba, unsigned int len, unsigned char drv) {
  // same as above 

  pbyte_out(0x1f6, 0xe0 | (drv << 4) | ((lba >> 24) & 0x0f));
  // pbyte_out(0x1f1, 0x00);
  pbyte_out(0x1f2, len);
  pbyte_out(0x1f3, lba & 0xff);
  pbyte_out(0x1f4, (lba >> 8) & 0xff);
  pbyte_out(0x1f5, (lba >> 16) & 0xff);
  // writing this time
  pbyte_out(0x1f7, WRITE_SECTORS);

  unsigned char p;
  for (unsigned int i = 0; i < len; i++) {
    // poll until ready
    // using a for loop, because a while loop caused mystery error in qemu
    for (; ((p & 0x88) != 0x08) && !(p & 1); p = pbyte_in(0x1f7));

    if (p & 1) { // oh no bad
      msg(KERNERR, E_NOSTORAGE, "Failed to ATA WRITE28");
      return;
    }

    // rep outsw is too fast, so we just have a fake outsw function
    fake_outsw(0x1f0, 1 << 8, data + (i << 9));
  }

  ata_cache_flush();
}
