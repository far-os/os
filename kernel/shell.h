#include "text.h"
#include "util.h"
#include "hwinf.h"
#include "fs.h"
#include "ata.h"
#include "config.h"
#include "memring.h"
#include "defs.h"
#include "cmos.h"
#include "err.h"
// #include "kbd.h"
// because of cyclic include, we declare what we want

struct keystates *keys;
void cpu_reset();

extern int prog(int arg);

#define IS_COM actv == &comd
#define IS_USR actv == &usrd

#ifndef SHELL_H
#define SHELL_H

#define COMLEN 16
struct inp_strbuf comd = {
  .buf = NULL,
  .len = COMLEN,
  .ix = 0
};

char hist_combuf[COMLEN];

#define OUT_LEN 128
char *outbuf;

struct inp_strbuf usrd = {
  .buf = NULL,
  .len = 0,
  .ix = 0,
};

struct inp_strbuf *actv = &comd;

const char *comnames[] = { // starting with 0xff, means arg for previous
  "clear",
  "cpu",
  "exec",
  "\xff<u32>",
  "file",
  "\xff<r|ed>",
  "help",
  "indic",
  "info",
  "rconfig",
  "reset",
  "time",
  "ver",
  NULL, // nullptr
};

void shexec() {
  unsigned char fmt = COLOUR(BLACK, WHITE);
  char *outbuf = malloc(OUT_LEN);
  if (strlen(comd.buf) == 0) {
    goto shell_clean;
  } else if (strcmp(comd.buf, "info")) {
    fmt = COLOUR(RED, B_YELLOW); // fmt
    sprintf(outbuf, "FarOS Kernel:\n\tVol. label \"%16s\"\n\tVol. ID %16X\n\tDisk %2xh\n\tVolume size %d",
      &(csdfs -> label),
      &(csdfs -> vol_id),
      &(hardware -> bios_disk),
      csdfs -> fs_size * SECTOR_LEN
    );
  } else if (strcmp(comd.buf, "cpu")) {
    fmt = COLOUR(YELLOW, B_GREEN); // fmt
    sprintf(outbuf, "CPUID.\n\t\x10 %12s\n\tFamily %2xh, Model %2xh, Stepping %1xh\n\tBrand \"%s\"",
      &(hardware -> vendor),
      &(hardware -> c_family), // family
      &(hardware -> c_model), // model
      &(hardware -> c_stepping), // stepping
      hardware -> cpuid_ext_leaves >= 0x80000004 ? &(hardware -> brand) : NULL // brand string
    );
  } else if (strcmp(comd.buf, "help")) {
    fmt = COLOUR(BLUE, B_MAGENTA);
    for (int cm = 0; comnames[cm]; ++cm) {
      if (comnames[cm][0] == -1) {
        sprintf(endof(outbuf), " %s", comnames[cm] + 1);
      } else {
        sprintf(endof(outbuf), "%c\t%s", cm ? '\n' : 0, comnames[cm]);
      }
    }
  } else if (strcmp(comd.buf, "ver")) {
    fmt = COLOUR(CYAN, B_YELLOW);
    to_ver_string(curr_ver, outbuf);
    sprintf(endof(outbuf), " build %d", curr_ver -> build);
  } else if (strcmp(comd.buf, "time")) {
    fmt = COLOUR(RED, B_CYAN);
    sprintf(outbuf, "Time since kernel load: %d.%2ds\n%s%c%2d-%2d-%2d %2d:%2d:%2d",
      countx / 100,
      countx % 100,
      curr_time -> weekday ? weekmap[curr_time -> weekday - 1] : NULL,
      curr_time -> weekday ? ' ' : NULL,
      curr_time -> year,
      curr_time -> month,
      curr_time -> date,
      curr_time -> hour,
      curr_time -> minute,
      curr_time -> second
    );
  } else if (strcmp(comd.buf, "indic")) {
    fmt = COLOUR(GREEN, RED);
    // indicators
    sprintf(outbuf, "scroll: %d\nnum: %d\ncaps: %d",
      bittest(&(keys -> modifs), 0),
      bittest(&(keys -> modifs), 1),
      bittest(&(keys -> modifs), 2)
    );
  } else if (strcmp(comd.buf, "reset")) {
    cpu_reset();
  } else if (strcmp(comd.buf, "clear")) {
    clear_scr();
    set_cur(POS(0, 0));
    goto shell_clean;
  } else if (memcmp(comd.buf, "exec", 4)) {
    if (disk_config -> qi_magic != CONFIG_MAGIC) {
      msg(KERNERR, 4, "Disk is unavailable");
      line_feed();
      goto shell_clean;
    }

    read_inode(
      name2inode("prog.bin"),
      0x100000
    ); // reads disk, has to get master or slave

    int ar = -1;
    if (strlen(comd.buf) > 5) {
      ar = to_uint(comd.buf + 5);
    }

    int ret = prog(ar);
    if (ret == 7) {
      msg(PROGERR, ret, "Program not found");
    } else if (ret == 9) {
      msg(KERNERR, ret, "Program executed illegal instruction");
    }
  } else if (strcmp(comd.buf, "file ed")) {
    set_page(1);
    set_cur(0);
    clear_scr();

    char * linebox = malloc(23);
    memset(linebox, 22, 0xcd); // double line box drawing

    write_str(linebox, COLOUR(GREEN, RED));
    write_str(" Editor \x07 Press ", COLOUR(GREEN, B_YELLOW));
    write_str("^S", COLOUR(GREEN, B_CYAN));
    write_str(" to save and exit ", COLOUR(GREEN, B_YELLOW));
    write_str(linebox, COLOUR(GREEN, RED));

    free(linebox);

    // usermode init
    usrd.len = disk_config -> wdata.len << 9; // 1 sector
    usrd.buf = malloc(usrd.len);
    read_inode(
      name2inode("data.txt"),
      usrd.buf
    ); // reads disk, has to get master or slave
    usrd.ix = strlen(usrd.buf);

    actv = &usrd;
    goto shell_clean;
  } else if (strcmp(comd.buf, "file r")) {
    char *datablk = malloc(disk_config -> wdata.len << 9);
    read_inode(
      name2inode("data.txt"),
      datablk
    ); // reads disk, has to get master or slave
    write_str(datablk, COLOUR(BLACK, WHITE));

    free(datablk);
    line_feed();
    goto shell_clean;

  } else if (strcmp(comd.buf, "rconfig")) {
    fmt = COLOUR(BLUE, B_YELLOW); // fmt
    sprintf(outbuf, "config.qi\n\tProgram at lba sector %2X, %d sector(s)\n\t\x10\t%s\n\tWritable data at lba sector %2X, %d sector(s)",
      &(disk_config -> exec.lba),
      disk_config -> exec.len,
      hardware -> boot_disk_p.itrf_type,
      &(disk_config -> wdata.lba),
      disk_config -> wdata.len
    );
  } else {
    msg(WARN, 11, "Unknown command");
  }
  write_str(outbuf, fmt);
  line_feed();

shell_clean:
  memcpy(comd.buf, hist_combuf, comd.len);
  comd.ix = 0;
  free(outbuf);
}

void curupd() {
  if (actv -> ix > strlen(actv -> buf)) actv -> ix = strlen(actv -> buf);
  if (IS_COM) {
    set_cur(POS(actv->ix + 3, ln_nr()));
  } else if (IS_USR) {
    unsigned short runx = POS(0, 1);
    for (int ii = 0; ii < actv -> ix; ii++) {
      switch (actv->buf[ii]) {
      case '\n':
        runx /= POS(0, 1);
        runx++;
        runx *= POS(0, 1);
        break;
      case '\b':
        runx--;
        break;
      default:
        runx++;
      }
    }
    set_cur(runx);
  }
}

void comupd() {
  if (strlen(actv -> buf) >= actv -> len) {
    if (IS_COM) {
      line_feed();
      msg(PROGERR, 23, "Buffer size exceeded");
      line_feed();
      memzero(actv -> buf, actv -> len);
      actv -> ix = 0;
    } else if (IS_USR) {
      actv->buf[actv->len -1] = 0;
    }
  }

  int comlen = strlen(actv -> buf);

  switch (actv->buf[actv->ix - 1]) {
  case '\b':
    memcpy(actv->buf + actv->ix, actv->buf + actv->ix - 2, actv->len - actv->ix);
    actv -> ix -= 2;

    if (IS_COM) clear_ln(ln_nr());
    else for (int il = 1; il < VGA_WIDTH; ++il) clear_ln(il); // extremely 100% not cursed else for

    break;
  case '\n':
    if (IS_USR) {
      for (int il = 1; il < VGA_WIDTH; ++il) clear_ln(il);
      break;
    }

    line_feed();
    comd.buf[comd.ix - 1] = '\0';
    memcpy(comd.buf + comd.ix, comd.buf + comd.ix - 1, comd.len - comd.ix);

    shexec();
    memzero(comd.buf, comd.len);
    comd.ix = 0;
    break;
  }

  if (IS_COM) {
    char printbuf[20] = "\r\x13> "; // 0x13 is the !! symbol
    strcpy(comd.buf, printbuf + 4);
    write_str(printbuf, COLOUR(BLACK, WHITE));
  } else if (IS_USR) {
    set_cur(POS(0, 1));
    write_str(usrd.buf, COLOUR(BLACK, WHITE));
  }
  curupd();
}

void sh_hist_restore() {
  memcpy(hist_combuf, comd.buf, comd.len);
  comd.ix = strlen(comd.buf);
  comupd();
}

void sh_ctrl_c() {
  write_str("^C", COLOUR(BLACK, B_BLACK));
  line_feed();
  memzero(comd.buf, comd.len);
  comupd();
}

void usr_ctrl_s() {
  usrd.ix = strlen(usrd.buf);
  write_str("^S", COLOUR(BLACK, B_BLACK));
  curupd();
  write_inode(
    name2inode("data.txt"),
    usrd.buf
  ); // writes to disk, see above

  if (usrd.buf != NULL) free(usrd.buf);
  usrd.len = 0;
  usrd.ix = 0;
  actv = &comd;

  set_page(0);
  page_curloc_cache[1] = 0;
  msg(INFO, 0, "File written");
  line_feed();
  comupd();
}

void shell() {
  comd.buf = malloc(comd.len);
//  set_cur(POS(0, 1)); // new line
  memzero(hist_combuf, comd.len);
  char *headbuf = "Kernel Executive Shell. (c) 2022-4.\n"; // the underscores are placeholder for the memcpy
  write_str(headbuf, COLOUR(BLUE, B_RED));
//  write_hex(buf, -1);
  
  comupd();

  for (;;) {
    asm volatile ("hlt");
  }
}

#endif
