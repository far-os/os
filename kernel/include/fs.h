#pragma once

#include "cmos.h"

#define SECTOR_LEN 512

struct fat_superblock {
  char jmp_seq[3];
  char oem_id[8];
  unsigned short bytes_per_sec;
  unsigned char sec_per_clust;
  unsigned short reserved_secs; // should be KERN_LEN
  unsigned char n_fats;
  unsigned short n_root_entries;
  unsigned short small_n_sectors; // bootloader zeroised this for us, in favour of n_sectors later on
  unsigned char media_desc;
  unsigned short sec_per_fat;
  unsigned short sec_per_track;
  unsigned short n_heads;
  unsigned int hidden_secs; // n sectors before partition
  unsigned int n_sectors;
  unsigned char drive_number;
  unsigned char nt_flags; // mystery
  unsigned char sig; // 0x28 or 0x29, idrk which means what
  unsigned int serial_no;
  char vol_lbl[11];
  char sys_ident[8]; // supposed to be "FATxx   ", where xx is the type of FAT. do not trust
} __attribute__((packed));

extern struct fat_superblock *bpb;

typedef unsigned short dostime;
typedef unsigned short dosdate;

// fat file entry structure
struct dir_entry {
  char name[8];
  char ext[3];
  unsigned char attrib;
  unsigned char rsrvd;
  unsigned char ctime_cs;
  dostime ctime;
  dosdate cdate;
  dosdate adate;
  unsigned short first_cluster_hi; // always zero on fat12/16
  dostime mtime;
  dosdate mdate;
  unsigned short first_cluster_lo;
  unsigned int size;
} __attribute__((packed));

#define NO_NEXT_CLUSTER 0xff8

#define FAT_FILENAME_LEN 11

typedef unsigned short cluster_id;
typedef unsigned int lba_n;

typedef unsigned short dir_n;
cluster_id next_cluster(cluster_id from);

void init_locs();
void read_fat();
void read_root();

void canonicalise_name(char *from, char *to);
void sane_name(char *from, char *to);
void *get_cluster(cluster_id from);

struct dir_entry get_file(char *name);

extern unsigned char *file_table;
extern struct dir_entry *root_dir;

