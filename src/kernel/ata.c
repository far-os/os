#include "include/ata.h"
#include "include/err.h"
#include "include/fs.h"
#include "include/port.h"

// see TODO in header file
unsigned short *ata_identity = (unsigned short *) 0xd000;

void ata_soft_reset() {
  unsigned char inp = pbyte_in(0x3f6);
  inp |= 1 << 2;
  pbyte_out(0x3f6, inp);
  inp ^= 1 << 2;
  pbyte_out(0x3f6, inp);
}

// wait 10cs for timeout
#define TIMEOUT_AFTER_CENTISECS 10
bool ata_busy_wait(const char * command, bool skip_drq) {
  // ticks in centiseconds.
  asm volatile ( "sti" ); // NB: because apparently uptime breaks in qemu????
  unsigned int wait_since = uptime;
  unsigned int wait_until = wait_since + TIMEOUT_AFTER_CENTISECS;

  unsigned char status = pbyte_in(0x3f6);
  // read status byte
  for (;; status = pbyte_in(0x3f6)) {
    // poll until BSY clear and DRQ set
    if ((!skip_drq || !(status & 0x80)) && (skip_drq || !!(status & 0x08))) {
      break;
    }

    if (uptime > wait_until) {
      msg(WARN, E_NOSTORAGE, "Disk stuck <%2x> after %d ticks, whilst %s", &status, uptime - wait_since, command);
//      ata_soft_reset();
      return false;
    }
  }

  // if ERR is set
  if (status & 0x1) {
    unsigned char err = pbyte_in(ATA_REG_ERROR);
    msg(KERNERR, E_NOSTORAGE, "ATA %s error: code s%2x:%2x", command, &status, &err);
    return false;
  }

  return true;
}

void ata_400ns() {
  for (int i = 0; i < 15; i++) {
    pbyte_in(0x3f6);
  }
}

static inline void ata_cache_flush() {
  pbyte_out(0x1f7, CACHE_FLUSH);
}

void ata_identify(void *addr, master_slave_selector drv) {
  // disk select: A0 for master or B0 for slave
  pbyte_out(0x1f6, 0xa0 | (drv << 4));
  ata_400ns();

  // is optional and wastes time
  pbyte_out(0x1f1, 0x00);

  // the great zero
  pbyte_out(0x1f2, 0);
  pbyte_out(0x1f3, 0);
  pbyte_out(0x1f4, 0);
  pbyte_out(0x1f5, 0);

  // send command
  pbyte_out(0x1f7, IDENTIFY);

  ata_busy_wait("IDENTIFY", false);

  // read!
  rep_insw(0x1f0, 1 << 8, addr);

  ata_busy_wait("IDENTIFY", true);
}

void read_pio28(void *addr, lba_n lba, unsigned char len, master_slave_selector drv) {
  // id: high nybble is E for master or F for slave - low nybble in highest nybble of LBA
  pbyte_out(0x1f6, 0xe0 | (drv << 4) | ((lba >> 24) & 0x0f));
  ata_400ns();

  // is optional and wastes time
  pbyte_out(0x1f1, 0x00);

  // sector count
  pbyte_out(0x1f2, len);

  // lba
  pbyte_out(0x1f3, lba & 0xff);
  pbyte_out(0x1f4, (lba >> 8) & 0xff);
  pbyte_out(0x1f5, (lba >> 16) & 0xff);

  // send command
  pbyte_out(0x1f7, READ_SECTORS);

  for (unsigned int i = 0; i < len; i++) {
    ata_busy_wait("READ28", false);

    // read!
    rep_insw(0x1f0, 1 << 8, addr + (i << 9));
  }

  ata_busy_wait("READ28", true);
}

// NOTE: if under qemu this mysteriously fails, do not be surprised.
// this is an absolute pig's breakfast of "just in case" calls to stop qemu from shitting itself.
// i think this works.
// DO NOT TOUCH.

void write_pio28(void *data, lba_n lba, unsigned char len, master_slave_selector drv) {
  // same as above
  //msg(INFO, 0, "Writing %d from %p to %d", len, data, lba);
  pbyte_out(0x1f6, 0xe0 | (drv << 4) | ((lba >> 24) & 0x0f));
  ata_400ns();

  pbyte_out(0x1f1, 0x00);
  pbyte_out(0x1f2, len);
  //msg(INFO, 0, "unfinished business: %d thought", pbyte_in(0x1f3));
  pbyte_out(0x1f3, lba & 0xff);
  //msg(INFO, 0, "%d sectors but %d thought", lba, pbyte_in(0x1f3));
  pbyte_out(0x1f4, (lba >> 8) & 0xff);
  pbyte_out(0x1f5, (lba >> 16) & 0xff);
  // writing this time
  pbyte_out(0x1f7, WRITE_SECTORS);

  for (unsigned int i = 0; i < len; i++) {
    ata_busy_wait("WRITE28", false);

    // rep outsw is too fast, so we just have a fake outsw function
    fake_outsw(0x1f0, 1 << 8, data + (i << 9));

    ata_400ns();
  }

  ata_400ns();

  ata_busy_wait("WRITE28", true);

  pbyte_in(0x1f7);

  ata_cache_flush();
}
