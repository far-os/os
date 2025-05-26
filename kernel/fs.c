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
unsigned char *file_table = (void *) 0x20000;
struct dir_entry *root_dir = NULL;

unsigned int root_dir_secs_n = 0;
lba_n root_starts_at = 0;
lba_n data_starts_at = 0;

// TODO: embiggen
const unsigned char FAT_TYPE = 12;

void init_locs() {
  root_dir_secs_n = CEIL_DIV(bpb -> n_root_entries, bpb -> bytes_per_sec);
  root_starts_at = bpb -> hidden_secs + bpb -> reserved_secs + (bpb -> n_fats * bpb -> sec_per_fat);
  data_starts_at = root_starts_at + root_dir_secs_n;
}

void *get_cluster(cluster_id from) {
  return ((from - 2) * bpb -> sec_per_clust) + data_starts_at;
}

void read_fat() {
  read_pio28(
    file_table,
    bpb -> reserved_secs + bpb -> hidden_secs,
    bpb -> sec_per_fat * 2,
    hardware -> boot_disk_p.dev_path[0] & 0x01
  ); // reads disk for config, has to get master or slave

  int n = bpb -> sec_per_fat << 9;

  root_dir = file_table + n;

  if (!memcmp(file_table, file_table + n, n)) {
    msg(KERNERR, E_NOSTORAGE, "FATs are not identical");
  }
}

// follow chain
cluster_id next_cluster(cluster_id from) {
  switch (FAT_TYPE) {
    case 12:
      int byte_index = (from * 3) >> 1; // get byte
      unsigned short v_short = *((unsigned short *) (file_table + byte_index));
      return (from & 1) ? v_short >> 4 : v_short & 0xfff;
    case 16:
      return *((unsigned short *) (file_table + (from << 1)));
    default:
      return -1;
  }
}

// read root directory
void read_root() {
  read_pio28(
    root_dir,
    root_starts_at,
    root_dir_secs_n,
    hardware -> boot_disk_p.dev_path[0] & 0x01
  ); // reads disk for config, has to get master or slave
}

// e.g. abcdefgh.xyz => ABCDEFGHXYZ
void canonicalise_name(char *from, char *to) {
  memset(to, FAT_FILENAME_LEN, ' ');
  unsigned char dotted = 0; // have we dotted yet?
  int g = -1;
  for (int c = 0; (from[c] && dotted < 2); ++c) {
    if (from[c] == '.') {
      dotted++;
      g = 0;
      continue;
    }

    if (dotted == 0) {
      if (c < 8) {
        to[c] = to_upper(from[c]);
      }
    } else { // dotted == 1, can't be 2, as that's in for loop condition
      to[8 + g++] = to_upper(from[c]);
      if (g >= 3) {
        break;
      }
    }
  }
}

// opposite of above. because LFN may not make sense later on (TODO), i'm keeping it uppercase
void sane_name(char *from, char *to) {
  memzero(to, FAT_FILENAME_LEN);
  for (int c = 0; (c < 8 && from[c] != ' '); ++c) {
    to[c] = from[c];
  }

  if (from[8] != ' ') {
    *endof(to) = '.';
  }

  for (int c = 8; (c < FAT_FILENAME_LEN && from[c] != ' '); ++c) {
    *endof(to) = from[c];
  }
}

struct dir_entry get_file(char *name) {
  struct dir_entry blank = { .name = "\0" };
  char nbuf[FAT_FILENAME_LEN + 1] = {0};
  canonicalise_name(name, nbuf);
  for (unsigned int search = 0; *((unsigned char*) &root_dir[search]); search++) {
    if (memcmp(nbuf, root_dir[search].name, FAT_FILENAME_LEN)) return root_dir[search];
  }
  msg(PROGERR, E_NOFILE, "File not found");
  return blank;
}

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
