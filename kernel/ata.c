#include "include/ata.h"
#include "include/fs.h"
#include "include/port.h"

static inline void ata_cache_flush() {
  pbyte_out(0x1f7, CACHE_FLUSH);
}

void read_pio28(void *addr, struct sector_box where, unsigned char drv) {
  // id: high nybble is E for master or F for slave - how nybble in highest nybble of LBA
  pbyte_out(0x1f6, 0xe0 | (drv << 4) | ((where.lba >> 24) & 0x0f));

  // is optional and wastes time
  // pbyte_out(0x1f1, 0x00);

  // sector count
  pbyte_out(0x1f2, where.len);

  // lba
  pbyte_out(0x1f3, where.lba & 0xff);
  pbyte_out(0x1f4, (where.lba >> 8) & 0xff);
  pbyte_out(0x1f5, (where.lba >> 16) & 0xff);

  // send command
  pbyte_out(0x1f7, READ_SECTORS);

  // poll until ready
  while (!(pbyte_in(0x1f7) & 0x08));

  // read!
  rep_insw(0x1f0, where.len << 8, addr);
}

void write_pio28(void *data, struct sector_box where, unsigned char drv) {
  // same as above 

  pbyte_out(0x1f6, 0xe0 | (drv << 4) | ((where.lba >> 24) & 0x0f));
  // pbyte_out(0x1f1, 0x00);
  pbyte_out(0x1f2, where.len);
  pbyte_out(0x1f3, where.lba & 0xff);
  pbyte_out(0x1f4, (where.lba >> 8) & 0xff);
  pbyte_out(0x1f5, (where.lba >> 16) & 0xff);
  // writing this time
  pbyte_out(0x1f7, WRITE_SECTORS);
  // poll until ready
  while (!(pbyte_in(0x1f7) & 0x08));

  // rep outsw is too fast, so we just have a fake outsw function
  fake_outsw(0x1f0, where.len << 8, data);

  ata_cache_flush();
}
