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

#ifndef SHELL_H
#define SHELL_H

#define COM_LEN 16

char combuf[COM_LEN]; 
char hist_combuf[COM_LEN];
unsigned char comdex = 0;

#define OUT_LEN 128
char *outbuf;

const char *comnames[] = { // starting with 0xff, means arg for previous
  "clear",
  "cpu",
  "exec",
  "\xff<u32>",
  "file",
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
  if (strlen(combuf) == 0) {
    goto shell_clean;
  } else if (strcmp(combuf, "info")) {
    fmt = COLOUR(RED, B_YELLOW); // fmt
    sprintf(outbuf, "FarOS Kernel:\n\tVol. label \"%16s\"\n\tVol. ID %16X\n\tDisk %2xh\n\tVolume size %d",
      &(csdfs -> label),
      &(csdfs -> vol_id),
      &(hardware -> bios_disk),
      csdfs -> fs_size * SECTOR_LEN
    );
  } else if (strcmp(combuf, "cpu")) {
    fmt = COLOUR(YELLOW, B_GREEN); // fmt
    sprintf(outbuf, "CPUID.\n\t\x10 %12s\n\tFamily %2xh, Model %2xh, Stepping %1xh\n\tBrand \"%s\"",
      &(hardware -> vendor),
      &(hardware -> c_family), // family
      &(hardware -> c_model), // model
      &(hardware -> c_stepping), // stepping
      hardware -> cpuid_ext_leaves >= 0x80000004 ? &(hardware -> brand) : NULL // brand string
    );
  } else if (strcmp(combuf, "help")) {
    fmt = COLOUR(BLUE, B_MAGENTA);
    for (int cm = 0; comnames[cm]; ++cm) {
      if (comnames[cm][0] == -1) {
        sprintf(endof(outbuf), " %s", comnames[cm] + 1);
      } else {
        sprintf(endof(outbuf), "%c\t%s", cm ? '\n' : 0, comnames[cm]);
      }
    }
  } else if (strcmp(combuf, "ver")) {
    fmt = COLOUR(CYAN, B_YELLOW);
    to_ver_string(curr_ver, outbuf);
    sprintf(endof(outbuf), " build %d", curr_ver -> build);
  } else if (strcmp(combuf, "time")) {
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
  } else if (strcmp(combuf, "indic")) {
    fmt = COLOUR(GREEN, RED);
    // indicators
    sprintf(outbuf, "scroll: %d\nnum: %d\ncaps: %d",
      bittest(&(keys -> modifs), 0),
      bittest(&(keys -> modifs), 1),
      bittest(&(keys -> modifs), 2)
    );
  } else if (strcmp(combuf, "reset")) {
    cpu_reset();
  } else if (strcmp(combuf, "clear")) {
    clear_scr();
    set_cur(POS(0, 0));
    goto shell_clean;
  } else if (memcmp(combuf, "exec", 4)) {
    if (disk_config -> qi_magic != CONFIG_MAGIC) {
      msg(KERNERR, 4, "Disk is unavailable");
      line_feed();
      goto shell_clean;
    }

    read_pio28(0x100000, disk_config -> exec.lba, disk_config -> exec.len, hardware -> boot_disk_p.dev_path[0] & 0x01); // reads disk, has to get master or slave

    int ar = -1;
    if (strlen(combuf) > 5) {
      ar = to_uint(combuf + 5);
    }

    int ret = prog(ar);
    if (ret == 7) {
      msg(PROGERR, ret, "Program not found");
    } else if (ret == 9) {
      msg(KERNERR, ret, "Program executed illegal instruction");
    }
  } else if (strcmp(combuf, "file")) {
    char *datablk = malloc(disk_config -> wdata.len << 9);

    read_pio28(datablk, disk_config -> wdata.lba, disk_config -> wdata.len, hardware -> boot_disk_p.dev_path[0] & 0x01); // reads disk, has to get master or slave

    write_str(datablk, COLOUR(BLACK, WHITE));

    free(datablk);
    line_feed();
    goto shell_clean;
  } else if (strcmp(combuf, "rconfig")) {
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
  memcpy(combuf, hist_combuf, COM_LEN);
  comdex = 0;
  free(outbuf);
}

void curupd() {
  if (comdex > strlen(combuf)) comdex = strlen(combuf);
  set_cur(POS(comdex + 3, ln_nr()));
}

void comupd() {
  if (strlen(combuf) >= COM_LEN) {
    line_feed();
    msg(PROGERR, 23, "Command too long");
    line_feed();
    memzero(combuf, COM_LEN);
    comdex = 0;
  }

  int comlen = strlen(combuf);

  switch (combuf[comdex - 1]) {
  case '\b':
    memcpy(combuf + comdex, combuf + comdex - 2, COM_LEN - comdex);
    comdex -= 2;
    clear_ln(ln_nr());
    break;
  case '\n':
    line_feed();
    combuf[comdex - 1] = '\0';
    memcpy(combuf + comdex, combuf + comdex - 1, COM_LEN - comdex);

    shexec();
    memzero(combuf, COM_LEN);
    comdex = 0;
    break;
  }

  char printbuf[20] = "\r\x13> "; // 0x13 is the !! symbol
  strcpy(combuf, printbuf + 4);
  write_str(printbuf, COLOUR(BLACK, WHITE));
  curupd();
}

void sh_hist_restore() {
  memcpy(hist_combuf, combuf, COM_LEN);
  comdex = strlen(combuf);
  comupd();
}

void sh_ctrl_c() {
  write_str("^C", COLOUR(BLACK, B_BLACK));
  line_feed();
  memzero(combuf, 16);
  comupd();
}

void shell() {
//  set_cur(POS(0, 1)); // new line
  memzero(hist_combuf, 16);
  char *headbuf = "Kernel Executive Shell. (c) 2022-4.\n"; // the underscores are placeholder for the memcpy
  write_str(headbuf, COLOUR(BLUE, B_RED));
//  write_hex(buf, -1);
  
  comupd();

  for (;;) {
    asm volatile ("hlt");
  }
}

#endif
