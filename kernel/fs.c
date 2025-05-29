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
unsigned int bytes_per_clust = 0;

// TODO: embiggen
const unsigned char FAT_TYPE = 12;

void init_locs() {
  root_dir_secs_n = CEIL_DIV(bpb -> n_root_entries * 32, bpb -> bytes_per_sec);
  root_starts_at = bpb -> hidden_secs + bpb -> reserved_secs + (bpb -> n_fats * bpb -> sec_per_fat);
  data_starts_at = root_starts_at + root_dir_secs_n;
  bytes_per_clust = bpb -> bytes_per_sec * bpb -> sec_per_clust;
}

lba_n get_cluster(cluster_id from) {
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

void write_fat() {
  write_pio28(
    file_table,
    bpb -> reserved_secs + bpb -> hidden_secs,
    bpb -> sec_per_fat,
    hardware -> boot_disk_p.dev_path[0] & 0x01
  ); // reads disk for config, has to get master or slave
  
  // the second fat
  write_pio28(
    file_table,
    bpb -> reserved_secs + bpb -> hidden_secs + bpb -> sec_per_fat,
    bpb -> sec_per_fat,
    hardware -> boot_disk_p.dev_path[0] & 0x01
  ); // reads disk for config, has to get master or slave
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

// next free
cluster_id alloc_cluster(cluster_id with_min) {
  // bytes per fat
  int n_clusters = bpb -> sec_per_fat * bpb -> bytes_per_sec;
  switch (FAT_TYPE) {
    case 12:
      n_clusters *= 2;
      n_clusters /= 3;
      break;
    case 16:
      n_clusters /= 2;
      break;
    default:
      n_clusters *= -1;
  }

  for (cluster_id ci = with_min; ci < n_clusters; ++ci) {
    if (!next_cluster(ci)) return ci;
  }

  // TODO: out of space
  return -1;
}

// follow chain
void set_cluster(cluster_id nth, cluster_id to) {
  switch (FAT_TYPE) {
    case 12:
      int byte_index = (nth * 3) >> 1; // get byte
      unsigned short v_short = *((unsigned short *) (file_table + byte_index));
      if (nth & 1) {
        v_short &= 0x000f;
        v_short |= to << 4;
      } else {
        v_short &= 0xf000;
        v_short |= to;
      };
      *((unsigned short *) (file_table + byte_index)) = v_short;
      break;
    case 16:
      *((unsigned short *) (file_table + (nth << 1))) = to;
      break;
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

void write_root() {
  write_pio28(
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

struct dir_entry *get_file(char *name) {
  char nbuf[FAT_FILENAME_LEN + 1] = {0};
  canonicalise_name(name, nbuf);
  for (unsigned int search = 0; VALID_FILE(root_dir[search]); search++) {
    if (memcmp(nbuf, root_dir[search].name, FAT_FILENAME_LEN)) return &root_dir[search];
  }
  msg(PROGERR, E_NOFILE, "File not found");
  return NULL;
}

void read_file(char *filename, void * where) {
  struct dir_entry *f = get_file(filename);

  if (!f || !VALID_FILE(*f)) {
    msg(KERNERR, E_NOFILE, "Invalid file");
    line_feed();
    return;
  }

  cluster_id trace = (f->first_cluster_hi << 16) | (f->first_cluster_lo);
  unsigned int bytes_left = f->size;

  do {
    if (bytes_left < bytes_per_clust) {
      void *tmp = malloc(bytes_per_clust); // to avoid overwriting, we put it in a buf first. there are umpteen better ways of doing this, but no

      read_pio28(
        tmp,
        get_cluster(trace),
        bpb -> sec_per_clust,
        hardware -> boot_disk_p.dev_path[0] & 0x01
      ); // reads disk for config, has to get master or slave

      memcpy(tmp, where, bytes_left);
      free(tmp);
      
      // although considered illegal, it is possible to have a cluster chain longer than the file size, so we just stop here
      break; // just in case
    } else {
      read_pio28(
        where,
        get_cluster(trace),
        bpb -> sec_per_clust,
        hardware -> boot_disk_p.dev_path[0] & 0x01
      ); // reads disk for config, has to get master or slave
    }

    bytes_left -= bytes_per_clust;
    where += bytes_per_clust;

    trace = next_cluster(trace);
  } while ((trace & NO_NEXT_CLUSTER) != NO_NEXT_CLUSTER);
}

// TODO: create files

void write_file(char *filename, void *where, unsigned int new_size) {
  struct dir_entry *f = get_file(filename);

  if (!f || !VALID_FILE(*f)) {
    msg(KERNERR, E_NOFILE, "Invalid file");
    line_feed();
    return;
  }

  if (f -> attrib & (A_VOLID | A_DIR)) {
    msg(KERNERR, E_NOFILE, "Not a file");
    line_feed();
    return;
  }

  if (f -> attrib & A_READONLY) {
    msg(KERNERR, E_NOFILE, "Cannot write to file; is readonly.");
    line_feed();
    return;
  }

  unsigned int old_size = f->size;
  f->size = new_size;

  unsigned int new_n_clusts = CEIL_DIV(new_size, bytes_per_clust);
  unsigned int old_n_clusts = CEIL_DIV(old_size, bytes_per_clust);
  cluster_id *traces = malloc(MAX(new_n_clusts, old_n_clusts) * sizeof(cluster_id));

  cluster_id trace = (f->first_cluster_hi << 16) | (f->first_cluster_lo);
  cluster_id t_init = trace; // we do this twice (i am stupid)

  for (int i = 0; i < old_n_clusts; ++i) {
    traces[i] = next_cluster(i ? traces[i - 1] : t_init);
  }

  if (old_n_clusts < new_n_clusts) { // growth
    // e.g. 3 => 5: [2] 3 4 ! _ _ => [2] 3 4 5 6 !
    cluster_id with_min = 2;
    for (int z_growth = old_n_clusts - 1; z_growth < (new_n_clusts - 1); ++z_growth) {
      with_min =
        traces[z_growth] = alloc_cluster(with_min);
    }
    traces[new_n_clusts - 1] = NO_NEXT_CLUSTER;
  } else if (old_n_clusts > new_n_clusts) { // shrinkage
    // e.g. 5 => 3: [2] 3 4 5 6 ! => [2] 3 4 ! _ _
    traces[new_n_clusts - 1] = NO_NEXT_CLUSTER;
    for (int z_shrink = new_n_clusts; z_shrink < old_n_clusts; ++z_shrink) {
      traces[z_shrink] = 0;
    }
  }

  for (int i = 0; i < MAX(new_n_clusts, old_n_clusts); ++i) {
    set_cluster(t_init, traces[i]);
    t_init = traces[i];
  }

  free(traces);

  unsigned int bytes_left = new_size;
  unsigned int clusts_written = 0;

  do {
    if (bytes_left < bytes_per_clust) {
      void *tmp = malloc(bytes_per_clust); // to avoid overwriting, we put it in a buf first. there are umpteen better ways of doing this, but no
      memcpy(where, tmp, bytes_left);

      write_pio28(
        tmp,
        get_cluster(trace),
        bpb -> sec_per_clust,
        hardware -> boot_disk_p.dev_path[0] & 0x01
      ); // reads disk for config, has to get master or slave

      free(tmp);
      break; // just in case
    } else {
      write_pio28(
        where,
        get_cluster(trace),
        bpb -> sec_per_clust,
        hardware -> boot_disk_p.dev_path[0] & 0x01
      ); // reads disk for config, has to get master or slave
    }

    bytes_left -= bytes_per_clust;
    where += bytes_per_clust;
    clusts_written++;

    trace = next_cluster(trace);
  } while ((trace & NO_NEXT_CLUSTER) != NO_NEXT_CLUSTER);

  // file stuff
  f -> attrib |= A_ARCHIVE;

  struct dos_timestamp dos = to_dostime(*curr_time);
  f->mdate = dos.dosdate;
  f->mtime = dos.dostime;

  write_fat();
  write_root();
}

char attribify_buf[9] = {0};

void attribify(unsigned char att) {
  strcpy("RHSVDA L", attribify_buf);
  for (int z = 0; z < 6; ++z) {
    if (!(att & (1 << z))) {
      attribify_buf[z] = '-';
    }
  }

  if ((att & A_LFN) != A_LFN) {
    attribify_buf[7] = '-';
  }
}
