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

#endif
