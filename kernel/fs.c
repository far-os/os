#include "include/fs.h"
#include "include/ata.h"
#include "include/cmos.h"
#include "include/config.h"
#include "include/err.h"
#include "include/hwinf.h"
#include "include/misc.h"
#include "include/util.h"
#include "include/text.h"

struct csdfs_superblock *csdfs = (struct csdfs_superblock *) 0x19fc0;
struct inode *file_table = (void *) 0x30000;

void fs_init() {
  file_table[0].name = "kernel.bin";
  file_table[0].loc = (struct sector_box){ .lba = 0, .len = KERN_LEN },
  memcpy(curr_time, &file_table[0].modified, sizeof(struct timestamp));

  file_table[1].name = "config.qi";
  file_table[1].loc = (struct sector_box){ .lba = KERN_LEN, .len = 1 },
  memcpy(curr_time, &file_table[1].modified, sizeof(struct timestamp));

  file_table[2].name = "prog.bin";
  memcpy(&disk_config -> exec, &file_table[2].loc, sizeof(struct sector_box));
  memcpy(curr_time, &file_table[2].modified, sizeof(struct timestamp));

  file_table[3].name = "data.txt";
  memcpy(&disk_config -> wdata, &file_table[3].loc, sizeof(struct sector_box));
  memcpy(curr_time, &file_table[3].modified, sizeof(struct timestamp));

  file_table[4].name = NULL;
}

inode_n name2inode(char *name) {
  for (inode_n search = 0; !!(file_table[search].name); search++) {
    if (strcmp(name, file_table[search].name)) return search;
  }
  msg(PROGERR, E_NOFILE, "File not found");
  line_feed();
  return -1;
}

void read_inode(inode_n file, void * where) {
  if (file < 0) {
    msg(KERNERR, E_NOFILE, "Invalid inode");
    line_feed();
    return;
  }

  read_pio28(
    where,
    file_table[file].loc,
    hardware -> boot_disk_p.dev_path[0] & 0x01
  ); // reads disk for config, has to get master or slave
}

void write_inode(inode_n file, void * where) {
  if (file < 0) {
    msg(KERNERR, E_NOFILE, "Invalid inode");
    line_feed();
    return;
  }

  write_pio28(
    where,
    file_table[file].loc,
    hardware -> boot_disk_p.dev_path[0] & 0x01
  ); // reads disk for config, has to get master or slave

  // changes mtime to most recent write
  memcpy(curr_time, &file_table[file].modified, sizeof(struct timestamp));
}
