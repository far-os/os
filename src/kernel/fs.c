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

struct fat_superblock *bpb = (struct fat_superblock *) 0x7c00; // boot location, but thats where the fat superblock is
unsigned char *file_table = (void *) 0x20000; // in-memory copy of FAT
struct dir_entry *root_dir = NULL; // in-memory copy of root directory. XXX: stored directly after FAT.

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

// read the fat into memory.
void read_fat() {
  read_pio28(
    file_table,
    bpb -> reserved_secs + bpb -> hidden_secs,
    bpb -> sec_per_fat * 2,
    hardware -> boot_disk_p.dev_path[0] & 0x01
  ); // reads disk for config, has to get master or slave

  // we set the address of the root_dir.
  // XXX: this is directly after the FAT. we can't hardcode this, as the FAT could vary in size
  int n = bpb -> sec_per_fat << 9;
  root_dir = (struct dir_entry *) (file_table + n);

  if (!memcmp(file_table, file_table + n, n)) { // check the two FATs. if they're different, file system is likely borked
    msg(KERNERR, E_NOSTORAGE, "FATs are not identical (%x, %x)", file_table, file_table + n);
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
// with_min gives a minimum to start seeking from. makes allocating more than one cluster more efficient, and avoids repetitive allocation of the same one.
cluster_id alloc_cluster(cluster_id with_min) {
  // 2 is the first cluster ever
  if (with_min < 2) { with_min = 2; }

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
  return NO_NEXT_CLUSTER; // 0 is a fake invalid cluster
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

// shitty macro that converts di so that di can stay at 0
// dots is just true_di multiplied by 8
#define TRUE_DI ((dots << 3) | (di & 7))

// e.g. abcdefgh.xyz => ABCDEFGHXYZ
void canonicalise_name(const char *from, char *to) {
  // pad with spaces
  memset(to, FAT_FILENAME_LEN, ' ');

  unsigned char dots = 0; // counting the number of dots. not exactly a bool
  int si = 0;
  int di = 0; // set immediately to 8 when dot

  // si = 0; special checks for first character
  switch (from[si]) {
  case FILE_F_UNUSED:
  case FILE_F_DELETED:
  case FILE_F_DIRECTORY:
    msg(KERNERR, E_NOFILE, "Bad filename \"%s\"", from);
    return;
  case FILE_F_ACTUALLY_E5: // so if you really want to put an 0xe5 in your filename, have a 0x05 instead
    to[di++] = 0xe5; // TODO add function for this in a "decanonicalise" function
    break;
  default:
    to[di++] = to_upper(from[si++]);
  }

  // loop through the rest of the characters
  for (; TRUE_DI < 11; si++) { // si++ is up here to ensure it is always called
    if (!from[si]) break; // null terminator found

    if (from[si] == '.') {
      di = 0;
      dots++;
      continue;
    }

    if (di >= 8) {
      continue; // NB: ignores ALL characters after index 8.
                // TODO: `~1` nonsense after LFN support
    }

    // NOT incrementing si, as that is done in for loop body
    to[TRUE_DI] = to_upper(from[si]);
    di++;
  }
}

// opposite of above. because LFN may not make sense later on (TODO), i'm keeping it uppercase
void sane_name(const char *from, char *to) {
  memzero(to, FAT_FILENAME_LEN);
  for (int c = 0; (c < 8 && from[c] != ' '); ++c) {
    to[c] = from[c];
  }

  if (to[0] == FILE_F_ACTUALLY_E5) {
    to[0] = 0xe5;
  }

  if (from[8] != ' ') {
    *endof(to) = '.';
  }

  for (int c = 8; (c < FAT_FILENAME_LEN && from[c] != ' '); ++c) {
    *endof(to) = from[c];
  }
}

struct dir_entry *get_file(const char *name) {
  char nbuf[FAT_FILENAME_LEN + 1] = {0};
  canonicalise_name(name, nbuf);
  for (unsigned int search = 0; VALID_FILE(root_dir[search]); search++) {
    if (memcmp(nbuf, root_dir[search].name, FAT_FILENAME_LEN)) return &root_dir[search];
  }
  msg(PROGERR, E_NOFILE, "File not found");
  return NULL;
}

// similar logic as above
bool does_exist(const char *name) {
  char nbuf[FAT_FILENAME_LEN + 1] = {0};
  canonicalise_name(name, nbuf);
  for (unsigned int search = 0; VALID_FILE(root_dir[search]); search++) {
    if (memcmp(nbuf, root_dir[search].name, FAT_FILENAME_LEN)) return true;
  }

  return false;
}

void rename_file(const char *old, const char *new) {
  struct dir_entry *f = get_file(old);
  canonicalise_name(new, f->name);
}

void read_file(const char *filename, void * where) {
  struct dir_entry *f = get_file(filename);

  if (!f || !VALID_FILE(*f)) {
    msg(KERNERR, E_NOFILE, "Invalid file");
    line_feed();
    return;
  }

  // change atime
  struct dos_timestamp dos = to_dostime(*curr_time);
  f->adate = dos.dosdate;

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

struct dir_entry *create_file(const char *filename) {
  if (does_exist(filename)) {
    msg(KERNERR, E_NOFILE, "File %s already exists", filename);
    return NULL;
  }

  // get first unoccupied file slot
  // TODO: fix for non-root directories, as uses n_root_entries
  unsigned int search = 0;
  for (; VALID_FILE(root_dir[search]); search++)
    if (search >= bpb->n_root_entries)
      msg(KERNERR, E_NOSTORAGE, "Root directory full");

  cluster_id loc = alloc_cluster(2);
  if (loc == NO_NEXT_CLUSTER) msg(KERNERR, E_NOSTORAGE, "Failed allocate cluster");

  struct dir_entry *f = &root_dir[search];

  // write filename. done first so as to not bork the file later on
  canonicalise_name(filename, f->name);

  // stub file, let write_file deal with size
  set_cluster(loc, NO_NEXT_CLUSTER);

  f->first_cluster_lo = loc & 0xffff;
  f->first_cluster_hi = (loc >> 16) & 0xffff;


  // file stuff
  f -> attrib = A_ARCHIVE;

  struct dos_timestamp dos = to_dostime(*curr_time);
  f->cdate = dos.dosdate;
  f->ctime = dos.dostime;
  f->ctime_cs = dos.centisecs;
  f->adate = dos.dosdate;
  f->mdate = dos.dosdate;
  f->mtime = dos.dostime;

  f->size = 0;

  // save changes to disk
  write_fat();
  write_root();

  return f;
}

void delete_file(const char *filename) {
  struct dir_entry *f = get_file(filename);

  if (f -> attrib & A_READONLY) {
    msg(KERNERR, E_NOFILE, "Cannot delete %s; is readonly.", filename);
    return;
  }

  // empty fat chain
  cluster_id trace = (f->first_cluster_hi << 16) | (f->first_cluster_lo);
  do {
    cluster_id trace_old = trace;
    trace = next_cluster(trace);
    set_cluster(trace_old, 0); // empty last cluster.
    // trace is the pointer to the next cluster, and it will eventually be the terminating 0xff8, so we don't touch it
  } while ((trace & NO_NEXT_CLUSTER) != NO_NEXT_CLUSTER);

  // zeroise directory entry
  memzero(f, sizeof(struct dir_entry));

  // mark file as deleted
  *((unsigned char *) f) = FILE_F_DELETED;

  // save changes to disk
  write_fat();
  write_root();
}

void write_file(const char *filename, void *where, unsigned int new_size) {
  struct dir_entry *f = get_file(filename);

  // create file if not exist
  if (!f || !VALID_FILE(*f)) {
    f = create_file(filename);
  }

  // if is either a volume or a directory we want it to fail
  // so to future me, NOT A BUG
  if (f -> attrib & (A_VOLID | A_DIR)) {
    msg(KERNERR, E_NOFILE, "%s is not a file", filename);
    return;
  }

  if (f -> attrib & A_READONLY) {
    msg(KERNERR, E_NOFILE, "Cannot write to %s; is readonly.", filename);
    return;
  }

  unsigned int old_size = f->size;
  f->size = new_size;

  unsigned int new_n_clusts = CEIL_DIV(new_size, bytes_per_clust);
  unsigned int old_n_clusts = CEIL_DIV(old_size, bytes_per_clust);
  cluster_id *traces = malloc(MAX(new_n_clusts, old_n_clusts) * sizeof(cluster_id));

  cluster_id trace = (f->first_cluster_hi << 16) | (f->first_cluster_lo);
  cluster_id t_init = trace; // we do this twice (i am stupid)

  for (unsigned int i = 0; i < old_n_clusts; ++i) {
    traces[i] = next_cluster(i ? traces[i - 1] : t_init);
  }

  if (old_n_clusts < new_n_clusts) { // growth
    // e.g. 3 => 5: [2] 3 4 ! _ _ => [2] 3 4 5 6 !
    cluster_id with_min = 2;
    for (unsigned int z_growth = old_n_clusts - 1; z_growth < (new_n_clusts - 1); ++z_growth) {
      with_min =
        traces[z_growth] = alloc_cluster(with_min);
    }
    traces[new_n_clusts - 1] = NO_NEXT_CLUSTER;
  } else if (old_n_clusts > new_n_clusts) { // shrinkage
    // e.g. 5 => 3: [2] 3 4 5 6 ! => [2] 3 4 ! _ _
    traces[new_n_clusts - 1] = NO_NEXT_CLUSTER;
    for (unsigned int z_shrink = new_n_clusts; z_shrink < old_n_clusts; ++z_shrink) {
      traces[z_shrink] = 0;
    }
  }

  for (unsigned int i = 0; i < MAX(new_n_clusts, old_n_clusts); ++i) {
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

// global buffer, just don't think about it too hard
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
