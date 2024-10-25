#pragma once

#define SECTOR_LEN 512

struct sector_box { // a pointer to a place of disk
  unsigned int lba; // lba of start
  unsigned char len; // length
} __attribute__((packed));

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
  struct timestamp modified;
} __attribute__((packed));

typedef int inode_n;

struct inode *file_table = (void *) 0x30000;

void fs_init();
inode_n name2inode(char *name);
void read_inode(inode_n file, void * where);
void write_inode(inode_n file, void * where);
