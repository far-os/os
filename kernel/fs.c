#include "include/fs.h"
#include "include/ata.h"
#include "include/cmos.h"
#include "include/config.h"
#include "include/err.h"
#include "include/hwinf.h"
#include "include/misc.h"
#include "include/util.h"
#include "include/text.h"
#include "include/memring.h"

struct fat_superblock *bpb = (struct fat_superblock *) 0x7c00; // boot location, but thats where the superblock os
unsigned char *file_table = (void *) 0x30000;
struct dir_entry *root_dir = NULL;

void read_fat() {
  read_pio28(
    file_table,
    bpb -> reserved_secs + bpb -> hidden_secs,
    bpb -> sec_per_fat * 2,
    hardware -> boot_disk_p.dev_path[0] & 0x01
  ); // reads disk for config, has to get master or slave

  int n = bpb -> sec_per_fat << 9;

  if (!memcmp(file_table, file_table + n, n)) {
    msg(KERNERR, E_NOSTORAGE, "FATs are not identical");
  }
}

//inode_n name2inode(char *name) { // FIXME
/*  for (inode_n search = 0; !!(file_table[search].name); search++) {
    if (strcmp(name, file_table[search].name)) return search;
  }
  msg(PROGERR, E_NOFILE, "File not found"); */
  //return -1;
//}

//void read_inode(inode_n file, void * where) {
  /* FIXME
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
  */
//}

//void write_inode(inode_n file, void * where) { /* FIXME
  /*if (file < 0) {
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
  memcpy(curr_time, &file_table[file].modified, sizeof(struct timestamp)); */
//}
