#include "include/ata.h"
#include "include/fs.h"
#include "include/port.h"

static inline void ata_cache_flush() {
  pbyte_out(0x1f7, CACHE_FLUSH);
}

void read_pio28(void *addr, lba_n lba, unsigned int len, unsigned char drv) {
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

  for (unsigned int i = 0; i < len; i++) {
    // poll until ready
    while (!(pbyte_in(0x1f7) & 0x08));

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

  for (unsigned int i = 0; i < len; i++) {
    // poll until ready
    while (!(pbyte_in(0x1f7) & 0x08));

    // rep outsw is too fast, so we just have a fake outsw function
    fake_outsw(0x1f0, 1 << 8, data + (i << 9));
  }

  ata_cache_flush();
}
