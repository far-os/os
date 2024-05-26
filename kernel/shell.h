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

#define OUT_LEN 120
char *outbuf;

void shexec() {
  unsigned char fmt = COLOUR(BLACK, WHITE);
  char *outbuf = malloc(OUT_LEN);
  if (strlen(combuf) == 0) {
    goto shell_clean;
  } else if (strcmp(combuf, "info")) {
    fmt = COLOUR(RED, B_YELLOW); // fmt
    strcpy("FarOS Kernel:\n\tVol. label \"________________\"\n\tVol. ID 0x________________\n\tDisk __h\n\tVolume size ", outbuf);
    memcpy(&(csdfs -> label), outbuf + 27, 16); // memcpy because we need to control the length
    to_hex(&(csdfs -> vol_id), 16, outbuf + 56);
    to_hex(&(hardware -> bios_disk), 2, outbuf + 79);
    to_dec(csdfs -> fs_size * SECTOR_LEN, outbuf + 96);
  } else if (strcmp(combuf, "cpu")) {
    fmt = COLOUR(YELLOW, B_GREEN); // fmt
    strcpy("CPUID.\n\t\x10 ____________\n\tFamily __h, Model __h, Stepping _h\n\tBrand \"________________________________________________\"", outbuf);
    memcpy(&(hardware -> vendor), outbuf + 10, 12); // cpu vendor

    to_hex(&(hardware -> c_family), 2, outbuf + 31); // family
    to_hex(&(hardware -> c_model), 2, outbuf + 42); // model
    to_hex(&(hardware -> c_stepping), 2, outbuf + 55); // stepping
    outbuf[55] = ' '; // hack
    if (hardware -> cpuid_ext_leaves >= 0x80000004) {
      strcpy(&(hardware -> brand), outbuf + 67); // brand string
    }
  } else if (strcmp(combuf, "help")) {
    fmt = COLOUR(BLUE, B_MAGENTA);
    strcpy("\tinfo\n\tcpu\n\thelp\n\ttime\n\tindic\n\treset\n\tclear\n\texec <u32>\n\tfile\n\tver\n\trconfig", outbuf);
  } else if (strcmp(combuf, "ver")) {
    fmt = COLOUR(CYAN, B_YELLOW);
    to_ver_string(curr_ver, outbuf);
    strcpy(" build ", outbuf + strlen(outbuf));
    to_dec(curr_ver -> build, outbuf + strlen(outbuf));
  } else if (strcmp(combuf, "time")) {
    fmt = COLOUR(RED, B_CYAN);
    strcpy("Time since kernel load: ", outbuf);
    to_dec(countx / 100, outbuf + strlen(outbuf));
    outbuf[strlen(outbuf)] = '.';
    to_filled_dec(countx % 100, outbuf + strlen(outbuf), 2, '0');
    strcpy("s\n", outbuf + strlen(outbuf));

    if (curr_time -> weekday) { // if weekday isn't valid (probably zero)
      strcpy(weekmap[curr_time -> weekday - 1], outbuf + strlen(outbuf));
      outbuf[strlen(outbuf)] = ' ';
    }

    to_dec(curr_time -> year, outbuf + strlen(outbuf));
    outbuf[strlen(outbuf)] = '-';
    to_filled_dec(curr_time -> month, outbuf + strlen(outbuf), 2, '0');
    outbuf[strlen(outbuf)] = '-';
    to_filled_dec(curr_time -> date, outbuf + strlen(outbuf), 2, '0');
    outbuf[strlen(outbuf)] = ' ';
    to_filled_dec(curr_time -> hour, outbuf + strlen(outbuf), 2, '0');
    outbuf[strlen(outbuf)] = ':';
    to_filled_dec(curr_time -> minute, outbuf + strlen(outbuf), 2, '0');
    outbuf[strlen(outbuf)] = ':';
    to_filled_dec(curr_time -> second, outbuf + strlen(outbuf), 2, '0');
  } else if (strcmp(combuf, "indic")) {
    fmt = COLOUR(GREEN, RED);
    // indicators
    strcpy("scroll: 0\nnum: 0\ncaps: 0", outbuf);
    if (bittest(&(keys -> modifs), 0)) { outbuf[8]++; } // scroll
    if (bittest(&(keys -> modifs), 1)) { outbuf[15]++; } // num
    if (bittest(&(keys -> modifs), 2)) { outbuf[23]++; } // caps
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
    strcpy("config.qi\n\tProgram at lba sector 0x__, ", outbuf);
    to_hex(&(disk_config -> exec.lba), 2, outbuf + 35);
    to_dec(disk_config -> exec.len, outbuf + strlen(outbuf));
    strcpy(" sector(s)\n\t\x10\t", outbuf + strlen(outbuf));
    strcpy(hardware -> boot_disk_p.itrf_type, outbuf + strlen(outbuf));

    strcpy("\n\tWritable data at lba sector 0x", outbuf + strlen(outbuf));
    to_hex(&(disk_config -> wdata.lba), 2, outbuf + strlen(outbuf));
    strcpy(", ", outbuf + strlen(outbuf));
    to_dec(disk_config -> wdata.len, outbuf + strlen(outbuf));
    strcpy(" sector(s)", outbuf + strlen(outbuf));
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
    memcpy(combuf + comdex, combuf + comdex - 2, COM_LEN - (comdex + 2));
    comdex -= 2;
    clear_ln(ln_nr());
    break;
  case '\n':
    line_feed();
    combuf[comdex - 1] = '\0';
    memcpy(combuf + comdex, combuf + comdex - 1, COM_LEN - (comdex + 1));

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
