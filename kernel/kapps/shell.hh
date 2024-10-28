// execute program
extern "C" int prog(int arg);

static const char *comnames[] = { // starting with 0xff, means arg for previous
  "clear",
  "cpu",
  "exec",
  "\xff<u32>",
  "file",
  "\xff<r|ed>",
  "help",
  "indic",
  "info",
  "ls",
  "rconfig",
  "reset",
  "stat",
  "\xff<inode|filename>",
  "time",
  "ver",
  NULL, // nullptr
};

// shell applet
#define SHELL_PROMPT_WIDTH 4
#define OUTBUF_LEN 120
struct KShell : KApp {
  void invoke() { // here, is effectively equivalent to comupd and curupd
    // write queued keys to the buffer, its a parent class function as it's quite common
    this->write_keys_to_buf(&this->work);

    // cause bools exist here
    bool to_exec = false;

    // control key processing
    for (int i = 0; i < QUEUE_LEN && !!this->ctrl_q[i]; ++i) { // loop over each arrow key
      switch (this -> ctrl_q[i]) {
        case NO_CTRL: break; // already dealt with, should never happen
        case CTRL_C: {
          write_str("^C\n", COLOUR(BLACK, B_BLACK));
          this->work.clear();
          break;
        }
        case ENTER: {// enter is a ctrlcode, we configured it so
          to_exec = true;
          break;
        }
        case BACKSPACE:
        case DEL: {
          int offset = this->ctrl_q[i] - BACKSPACE; // 0 if backspace, 1 if delete (naive)
          this->work.delchar_at(this->work.ix + offset - 1);
          break;
        }
        case SHIFT_F10: {
          clear_scr();
          set_cur(0); // set cursor to top left
          break;
        }
        case UP: {
          memcpy(histbuf, this->work.buf, this->work.len);
          this->work.ix = strlen(this->work.buf);
          break;
        }
        case LEFT: {
          if (this->work.ix != 0) this->work.ix--;
          break;
        }
        case RIGHT: {
          if (this->work.ix < strlen(this->work.buf)) this->work.ix++;
          break;
        }
        case HOME: {
          this->work.ix = 0;
          break;
        }
        case END: {
          this->work.ix = strlen(this->work.buf);
          break;
        }
        default: break;
      }
    }
    memzero(this->ctrl_q, QUEUE_LEN);

  comupd: 
    clear_ln(ln_nr());
    write_str(this->work.buf - SHELL_PROMPT_WIDTH, COLOUR(BLACK, WHITE));

  curupd:
    set_cur(POS(trace_ch_until_with(this->work.buf, this->work.ix, SHELL_PROMPT_WIDTH - 1), ln_nr()));

    if (!to_exec) return;

    line_feed();
    this->shexec();
    memcpy(this->work.buf, histbuf, this->work.len);
    this->work.clear();

    to_exec = 0;
    goto comupd; // get prompt to reappear
  }
 
private: // hidden fields (only for internal use)
  struct inp_strbuf work; // the buffer in which we work
  char *histbuf; // copy of the history
  char *outbuf; // buffer which is where data is dumped

  void shexec() {
    unsigned char fmt = COLOUR(BLACK, WHITE);
    char *outbuf = malloc(OUTBUF_LEN);
    if (strlen(this->work.buf) == 0) {
      goto shell_clean;
    } else if (strcmp(this->work.buf, "clear")) {
      clear_scr();
      set_cur(POS(0, 0));
      goto shell_clean;
    } else if (strcmp(this->work.buf, "cpu")) {
      fmt = COLOUR(YELLOW, B_GREEN); // fmt
      sprintf(outbuf, "CPUID.\n\t\x10 %12s\n\tFamily %2xh, Model %2xh, Stepping %1xh\n\tBrand \"%s\"",
        &(hardware -> vendor),
        &(hardware -> c_family), // family
        &(hardware -> c_model), // model
        &(hardware -> c_stepping), // stepping
        hardware -> cpuid_ext_leaves >= 0x80000004 ? &(hardware -> brand) : NULL // brand string
      );
    } else if (memcmp(this->work.buf, "exec", 4)) {
      if (disk_config -> qi_magic != CONFIG_MAGIC) {
        msg(KERNERR, E_NOSTORAGE, "Disk is unavailable");
        line_feed();
        goto shell_clean;
      }

      read_inode(
        name2inode("prog.bin"),
        0x100000
      ); // reads disk, has to get master or slave

      int ar = -1;
      if (strlen(this->work.buf) > 5) {
        ar = to_uint(this->work.buf + 5);
      }

      enum ERRSIG ret = prog(ar);
      if (ret == E_BOUND) {
        msg(PROGERR, ret, "Program not found");
      } else if (ret == E_ILLEGAL) {
        msg(KERNERR, ret, "Program executed illegal instruction");
      }
    } else if (strcmp(this->work.buf, "file r")) {
      char *datablk = malloc(disk_config -> wdata.len << 9);
      read_inode(
        name2inode("data.txt"),
        datablk
      ); // reads disk, has to get master or slave
      write_str(datablk, COLOUR(BLACK, WHITE));

      free(datablk);
      line_feed();
      goto shell_clean;
    } else if (strcmp(this->work.buf, "help")) {
      fmt = COLOUR(BLUE, B_MAGENTA);
      for (int cm = 0; comnames[cm]; ++cm) {
        if (comnames[cm][0] == -1) {
          sprintf(endof(outbuf), " %s", comnames[cm] + 1);
        } else {
          sprintf(endof(outbuf), "%c\t%s", cm ? '\n' : 0, comnames[cm]);
        }
      }
    } else if (strcmp(this->work.buf, "indic")) {
      fmt = COLOUR(GREEN, RED);
      // indicators
      sprintf(outbuf, "scroll: %d\nnum: %d\ncaps: %d",
        bittest(&(keys -> modifs), 0),
        bittest(&(keys -> modifs), 1),
        bittest(&(keys -> modifs), 2)
      );
    } else if (strcmp(this->work.buf, "info")) {
      fmt = COLOUR(RED, B_YELLOW); // fmt
      sprintf(outbuf, "FarOS Kernel:\n\tVol. label \"%16s\"\n\tVol. ID %16X\n\tDisk %2xh\n\tVolume size %d",
        &(csdfs -> label),
        &(csdfs -> vol_id),
        &(hardware -> bios_disk),
        csdfs -> fs_size * SECTOR_LEN
      );
    } else if (strcmp(this->work.buf, "ls")) {
      fmt = COLOUR(BLACK, B_WHITE);
      for (int filek = 0; file_table[filek].name; filek++) {
        strcat(outbuf, file_table[filek].name);
        *endof(outbuf) = '\t';
      }
    } else if (strcmp(this->work.buf, "rconfig")) {
      fmt = COLOUR(BLUE, B_YELLOW); // fmt
      sprintf(outbuf, "config.qi\n\tProgram at lba sector %2X, %d sector(s)\n\t\x10\t%s\n\tWritable data at lba sector %2X, %d sector(s)",
        &(disk_config -> exec.lba),
        disk_config -> exec.len,
        hardware -> boot_disk_p.itrf_type,
        &(disk_config -> wdata.lba),
        disk_config -> wdata.len
      );
    } else if (strcmp(this->work.buf, "reset")) {
      cpu_reset();
    } else if (memcmp(this->work.buf, "stat", 4)) {
      int ar = -1;
      if (strlen(this->work.buf) > 5) {
        ar = to_uint(this->work.buf + 5);
        if (ar < 0) {
          ar = name2inode(this->work.buf + 5);
          if (ar < 0) goto shell_clean;
        }
      }
      
      if (ar < 0 || !(file_table[ar].name)) {
        msg(PROGERR, E_NOFILE, "Invalid inode");
        line_feed();
        goto shell_clean;
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

      fmt = COLOUR(GREEN, RED);
    } else if (strcmp(this->work.buf, "time")) {
      fmt = COLOUR(RED, B_CYAN);
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
    } else if (strcmp(this->work.buf, "ver")) {
      fmt = COLOUR(CYAN, B_YELLOW);
      to_ver_string(curr_ver, outbuf);
      sprintf(endof(outbuf), " build %d", curr_ver -> build);
    } else {
      msg(WARN, E_UNKENTITY, "Unknown command");
    }
    write_str(outbuf, fmt);
    line_feed();

  shell_clean: // clean up before leaving
    free(outbuf);
  }

public:
  // constructor
  KShell(int comlen = 28): KApp() {
    // bitflags, or as many together as need be
    config_flags = NEEDS_ENTER_CTRLCODE;

    work = inp_strbuf {
      .buf = malloc(comlen + SHELL_PROMPT_WIDTH) + SHELL_PROMPT_WIDTH, // this is to hide the prompt
      .len = comlen,
      .ix = 0
    };
    strcpy("\r\x13> ", work.buf - SHELL_PROMPT_WIDTH); // very naive way of hiding the prompt

    histbuf = malloc(comlen);

    /*
    write_cell_cur('f', 0x0a);
    write_cell_cur('a', 0x0c);
    write_cell_cur('r', 0x0e);
    write_str("os ", 0x07); */
    write_str("Kernel Executive Shell. (c) 2022-4.\n", COLOUR(BLUE, B_RED));

    // print out prompt
    write_str(work.buf - SHELL_PROMPT_WIDTH, COLOUR(BLACK, WHITE));
  }

  // destructor
  ~KShell() { // madatory cleanup
    free(work.buf - SHELL_PROMPT_WIDTH);
    work.buf = NULL;

    free(histbuf);
    histbuf = NULL;
  }
};
