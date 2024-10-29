// execute program
extern "C" int prog(int arg);

#define DONT_PRINT 0

struct builtin_cmd {
  char *name;
  char *args; // if at all
  unsigned char (*func) (char*, char*); // actual function
};

extern struct builtin_cmd cmdlist[];

namespace sh_builtins {
  // each function returns the style in which to print - 0x00 if don't print

  // clear screen
  unsigned char clr(char *outbuf, char *args) {
    write_str("HEY\n\n", 0x2f);
    clear_scr();
    set_cur(0);

    return DONT_PRINT;
  }

  unsigned char help(char *outbuf, char *args) {
    for (int cm = 0; cmdlist[cm].name; ++cm) {
      sprintf(endof(outbuf), "%c\t%s", cm ? '\n' : 0, cmdlist[cm].name);
      if (cmdlist[cm].args) {
        sprintf(endof(outbuf), " %s", cmdlist[cm].args);
      }
    }

    return COLOUR(BLUE, B_MAGENTA);
  }

  unsigned char exec(char *outbuf, char *args) {
    if (disk_config -> qi_magic != CONFIG_MAGIC) {
      msg(KERNERR, E_NOSTORAGE, "Disk is unavailable");
      return DONT_PRINT;
    }

    read_inode(
      name2inode("prog.bin"),
      0x100000
    ); // reads disk, has to get master or slave

    int ar = -1;
    if (strlen(args)) {
      ar = to_uint(args);
    }

    enum ERRSIG ret = prog(ar);
    if (ret == E_BOUND) {
      msg(PROGERR, ret, "Program not found");
    } else if (ret == E_ILLEGAL) {
      msg(KERNERR, ret, "Program executed illegal instruction");
    } else {
      line_feed();
    }

    return DONT_PRINT;
  }

  namespace file {
    // read file
    unsigned char read(char *outbuf, char *args) {
      char *datablk = malloc(disk_config -> wdata.len << 9);
      read_inode(
        name2inode("data.txt"),
        datablk
      ); // reads disk, has to get master or slave
      write_str(datablk, COLOUR(BLACK, WHITE));

      free(datablk);
      line_feed();

      return DONT_PRINT;
    }

    // stat
    unsigned char stat(char *outbuf, char *args) {
      int ar = -1;
      if (strlen(args)) {
        ar = to_uint(args);
        if (ar < 0) {
          ar = name2inode(args);
          if (ar < 0) return DONT_PRINT;
        }
      }
      
      if (ar < 0 || !(file_table[ar].name)) {
        msg(PROGERR, E_NOFILE, "Invalid inode");
        return DONT_PRINT;
      }

      sprintf(outbuf, "%s <%d>\n\t%d bytes, sector %2X\n\tModified %4d-%2d-%2d %2d:%2d:%2d",
        file_table[ar].name,
        ar,
        file_table[ar].loc.len << 9,
        &(file_table[ar].loc.lba),
        file_table[ar].modified.year,
        file_table[ar].modified.month,
        file_table[ar].modified.date,
        file_table[ar].modified.hour,
        file_table[ar].modified.minute,
        file_table[ar].modified.second
      );

      return COLOUR(GREEN, RED);
    }
  }

  unsigned char ls(char *outbuf, char *args) {
    for (int filek = 0; file_table[filek].name; filek++) {
      strcat(outbuf, file_table[filek].name);
      *endof(outbuf) = '\t';
    }

    return COLOUR(BLACK, B_WHITE);
  }

  // system info routine
  namespace sys {
    unsigned char cpu(char *outbuf, char *args) {
      sprintf(outbuf, "CPUID.\n\t\x10 %12s\n\tFamily %2xh, Model %2xh, Stepping %1xh\n\tBrand \"%s\"",
        &(hardware -> vendor),
        &(hardware -> c_family), // family
        &(hardware -> c_model), // model
        &(hardware -> c_stepping), // stepping
        hardware -> cpuid_ext_leaves >= 0x80000004 ? &(hardware -> brand) : NULL // brand string
      );

      return COLOUR(YELLOW, B_GREEN); // fmt
    }

    unsigned char conf(char *outbuf, char *args) {
      sprintf(outbuf, "config.qi\n\tProgram at lba sector %2X, %d sector(s)\n\t\x10\t%s\n\tWritable data at lba sector %2X, %d sector(s)",
        &(disk_config -> exec.lba),
        disk_config -> exec.len,
        hardware -> boot_disk_p.itrf_type,
        &(disk_config -> wdata.lba),
        disk_config -> wdata.len
      );

      return COLOUR(BLUE, B_YELLOW); // fmt
    }

    unsigned char disk(char *outbuf, char *args) {
      sprintf(outbuf, "CSDFS Disk:\n\tVol. label \"%16s\"\n\tVol. ID %16X\n\tDisk %2xh\n\tVolume size %d",
        &(csdfs -> label),
        &(csdfs -> vol_id),
        &(hardware -> bios_disk),
        csdfs -> fs_size * SECTOR_LEN
      );

      return COLOUR(RED, B_YELLOW); // fmt
    }

    unsigned char indic(char *outbuf, char *args) {
      // indicators
      sprintf(outbuf, "scroll: %d\nnum: %d\ncaps: %d",
        bittest(&(keys -> modifs), 0),
        bittest(&(keys -> modifs), 1),
        bittest(&(keys -> modifs), 2)
      );

      return COLOUR(GREEN, RED);
    }
  }

  unsigned char reset(char *outbuf, char *args) {
    cpu_reset();
    return 0; // if you reach here, something has gone very wrong indeed
  }

  unsigned char time(char *outbuf, char *args) {
    sprintf(outbuf, "Time since kernel load: %d.%2ds\n%s%c%4d-%2d-%2d %2d:%2d:%2d",
      countx / 100,
      countx % 100,
      curr_time -> weekday ? weekmap[curr_time -> weekday - 1] : NULL,
      curr_time -> weekday ? ' ' : 0,
      curr_time -> year,
      curr_time -> month,
      curr_time -> date,
      curr_time -> hour,
      curr_time -> minute,
      curr_time -> second
    );

    return COLOUR(RED, B_CYAN);
  }
}

struct builtin_cmd cmdlist[] = {
  { .name = "clr", .args = "[or shift+F10]", .func = &sh_builtins::clr },
  { .name = "exec", .args = "<u32>", .func = &sh_builtins::exec },
  { .name = "fread", .args = NULL, .func = &sh_builtins::file::read },
  { .name = "fstat", .args = "<inode|filename>", .func = &sh_builtins::file::stat },
  { .name = "help", .args = NULL, .func = &sh_builtins::help },
  { .name = "ls", .args = NULL, .func = &sh_builtins::ls },
  { .name = "reset", .args = NULL, .func = &sh_builtins::reset },
  { .name = "sys conf", .args = NULL, .func = &sh_builtins::sys::conf },
  { .name = "sys cpu", .args = NULL, .func = &sh_builtins::sys::cpu },
  { .name = "sys disk", .args = NULL, .func = &sh_builtins::sys::disk },
  { .name = "sys indic", .args = NULL, .func = &sh_builtins::sys::indic },
  { .name = "time", .args = NULL, .func = &sh_builtins::time },
  { .name = NULL }
};
