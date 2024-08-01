#include "ata.h"
#include "config.h"

#ifndef FS_H
#define FS_H

#define SECTOR_LEN 512
struct csdfs_superblock {
  unsigned int magic; // 0xac50f0c5
  char label[16]; // volume 
  unsigned long long int vol_id; // 64-bit volume id
  unsigned short fs_start; // fs start LBA
  unsigned int fs_size; // fs size, in sectors
  unsigned char media_type; // media type, e.g. 0xa3 is 3½" HD 1.44M floppy
  unsigned char block_size; // block size in sectors
} __attribute__((packed));

struct csdfs_superblock *csdfs = (struct csdfs_superblock *) 0x19fc0;

// file system is purely in-memory, is abstracted to disk sectors in real time
struct inode {
  char *name;
  struct sector_box loc;
  struct timestamp created;
} __attribute__((packed));

typedef int inode_n;

struct inode *file_table = (void *) 0x30000;
void fs_init() {
  file_table[0].name = "kernel.bin";
  file_table[0].loc = (struct sector_box){ .lba = 0, .len = KERN_LEN },
  memcpy(curr_time, &file_table[0].created, sizeof(struct timestamp));

  file_table[1].name = "config.qi";
  file_table[1].loc = (struct sector_box){ .lba = KERN_LEN, .len = 1 },
  memcpy(curr_time, &file_table[1].created, sizeof(struct timestamp));

  file_table[2].name = "prog.bin";
  memcpy(&disk_config -> exec, &file_table[2].loc, sizeof(struct sector_box));
  memcpy(curr_time, &file_table[2].created, sizeof(struct timestamp));

  file_table[3].name = "data.txt";
  memcpy(&disk_config -> wdata, &file_table[3].loc, sizeof(struct sector_box));
  memcpy(curr_time, &file_table[3].created, sizeof(struct timestamp));

  file_table[4].name = NULL;
}

inode_n name2inode(char *name) {
  for (inode_n search = 0; !!(file_table[search].name); search++) {
    if (strcmp(name, file_table[search].name)) return search;
  }
  msg(PROGERR, 3, "File not found");
  line_feed();
  return -1;
}

void read_inode(inode_n file, void * where) {
  if (file < 0) {
    msg(KERNERR, 3, "Invalid inode");
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
    msg(KERNERR, 3, "Invalid inode");
    line_feed();
    return;
  }

  write_pio28(
    where,
    file_table[file].loc,
    hardware -> boot_disk_p.dev_path[0] & 0x01
  ); // reads disk for config, has to get master or slave
}

#endif
