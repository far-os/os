// execute program
extern "C" int prog(int arg);

// list of entries, Entry and EntType are children of HelpHost
const HelpHost::Entry comnames[] = {
  { .name = "clr", .desc = "Clears screen\0[or \034\23710]", .type = HelpHost::PLAIN_ENTRY },
  { .name = "exec", .desc = "Executes program, takes <u32>", .type = HelpHost::PLAIN_ENTRY },
  { .name = "f", .desc = "File I/O namespace", .type = HelpHost::PLAIN_ENTRY },
  { .name = "edit", .desc = "Text editor", .type = HelpHost::SUB_ENTRY },
  { .name = "ls", .desc = "Lists all files", .type = HelpHost::SUB_ENTRY },
  { .name = "read", .desc = "Prints file content", .type = HelpHost::SUB_ENTRY },
  { .name = "stat", .desc = "Prints info about given <inode> or <filename>", .type = HelpHost::SUB_ENTRY },
  { .name = "help", .desc = "Prints this help menu", .type = HelpHost::PLAIN_ENTRY },
  { .name = "reset", .desc = "Resets machine\0[or ^\340\021]", .type = HelpHost::PLAIN_ENTRY },
  { .name = "sys", .desc = "Utilities that print/dump system info", .type = HelpHost::PLAIN_ENTRY },
  { .name = "cpu", .desc = "Prints CPU info", .type = HelpHost::SUB_ENTRY },
  { .name = "disk", .desc = "Prints disk/fs info", .type = HelpHost::SUB_ENTRY },
  { .name = "indic", .desc = "Prints keyboard LED status", .type = HelpHost::SUB_ENTRY },
  { .name = "mem", .desc = "Prints memory info", .type = HelpHost::SUB_ENTRY },
  { .name = "proc", .desc = "Prints currently running processes", .type = HelpHost::SUB_ENTRY },
  { .name = "time", .desc = "Gets current time", .type = HelpHost::PLAIN_ENTRY },
  { .type = -1 }
};

// shell applet
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
        case CTRL(C): {
          write_str("^C\n", COLOUR(BLACK, B_BLACK));
          this->work.clear();
          break;
        }
        case ENTER: { // enter is a ctrlcode, we configured it so
          to_exec = true;
          break;
        }
        case BACKSPACE:
        case DEL: {
          int offset = this->ctrl_q[i] - BACKSPACE; // 0 if backspace, 1 if delete (naive)
          this->work.delchar_at(this->work.ix + offset - 1);
          break;
        }
        case SHIFT_F(10): {
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
    write_str(prompt, COLOUR(BLACK, WHITE));
    write_str(this->work.buf, COLOUR(BLACK, WHITE));

  curupd:
    set_cur(POS(trace_ch_until_with(this->work.buf, this->work.ix, strlen(prompt) - 1), ln_nr()));

    if (!to_exec) return;

    line_feed();
    bool exitting = this->shexec();
    memcpy(this->work.buf, histbuf, this->work.len);
    this->work.clear();

    to_exec = 0;
    if (exitting) goto comupd; // get prompt to reappear
  }

  void first_run() {
    /*
    write_str("os ", 0x07); */
    write_str("Kernel Executive Shell. (c) 2022-4.\n", COLOUR(BLUE, B_RED));

    // print out prompt
    write_str(prompt, COLOUR(BLACK, WHITE));
  }
 
private: // hidden fields (only for internal use)
  struct inp_strbuf work; // the buffer in which we work
  char *histbuf; // copy of the history
  char *outbuf; // buffer which is where data is dumped
  const char *prompt = "\r\x13> "; // prompt

  bool shexec() {
    if (strlen(this->work.buf) == 0) return;
    outbuf = malloc(OUTBUF_LEN);

    unsigned char fmt = 0;

    // couldn't afford strtok
    int wher; // first whitespace character
    for (wher = 0; !is_whitespace(work.buf[wher]); ++wher);

    char *args = &(work.buf[wher + 1]);
    char safe_padding = work.buf[wher]; // keep the padding character, so that history works
    work.buf[wher] = '\0'; // make the command the only relevant word

    bool exitting = true; // whether we exit normally

    if (strcmp(work.buf, "clr")) { // clear screen
      clear_scr();
      set_cur(0);
    } else if (strcmp(work.buf, "exec")) {
      /* FIXME
      if (disk_config -> qi_magic != CONFIG_MAGIC) {
        msg(KERNERR, E_NOSTORAGE, "Disk is unavailable");
        goto shell_clean;
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
      */
    } else if (strcmp(work.buf, "f:edit")) {
    /* FIXME
      app_handle edt = instantiate(
        new Editor(name2inode("data.txt")),
        this->app_id & 0xf,
        true
      ); */

      exitting = false;
      goto shell_clean;
    } else if (strcmp(work.buf, "f:ls")) {
      /* FIXME
      for (int filek = 0; file_table[filek].name; filek++) {
        strcat(outbuf, file_table[filek].name);
        *endof(outbuf) = '\t';
      }
      */

      fmt = COLOUR(BLACK, B_WHITE);
    } else if (strcmp(work.buf, "f:read")) {
      /*char *datablk = malloc(file_table[name2inode("data.txt")].loc.len << 9);
      read_inode(
        name2inode("data.txt"),
        datablk
      ); // reads disk, has to get master or slave
      write_str(datablk, COLOUR(BLACK, WHITE));

      free(datablk); FIXME */
      line_feed();
    } else if (strcmp(work.buf, "f:stat")) {
      /* FIXME
      int ar = -1;
      if (strlen(args)) {
        ar = to_uint(args);
        if (ar < 0) {
          ar = name2inode(args);
          if (ar < 0) goto shell_clean;
        }
      }
      
      if (ar < 0 || !(file_table[ar].name)) {
        msg(PROGERR, E_NOFILE, "Invalid inode");
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

      */
      fmt = COLOUR(GREEN, RED);
    } else if (strcmp(work.buf, "help")) {
      app_handle help = instantiate(
        new HelpHost("shell commands", comnames),
        this->app_id & 0xf,
        true
      );

      exitting = false;
      goto shell_clean;
    } else if (strcmp(work.buf, "reset")) {
      cpu_reset();
      // if you reach past here, something has gone very wrong indeed
    } else if (strcmp(work.buf, "split")) {
      if (strcmp(args, "off")) {
        split_scr(0);
      } else if (args[0] == '@') {
        int ar = -1;
        if (strlen(args) > 1) {
          ar = to_uint(args + 1);
        }

        if (ar == -1) goto _kesh_split_fail;
        else if (!app_db[ar]) goto _kesh_split_fail;

        split_scr(ar);
        focus_app(ar);
        app_db[ar]->invoke();
        exitting = false;
      } else {
      _kesh_split_fail:
        msg(PROGERR, E_UNKENTITY, "No valid handle supplied");
      }
    } else if (strcmp(work.buf, "sys:cpu")) {
      sprintf(outbuf, "CPUID.\n\t\x10 %12s\n\tFamily %2xh, Model %2xh, Stepping %1xh\n\tBrand \"%s\"",
        &(hardware -> vendor),
        &(hardware -> c_family), // family
        &(hardware -> c_model), // model
        &(hardware -> c_stepping), // stepping
        hardware -> cpuid_ext_leaves >= 0x80000004 ? &(hardware -> brand) : NULL // brand string
      );

      fmt = COLOUR(YELLOW, B_GREEN); // fmt
    } else if (strcmp(work.buf, "sys:disk")) { 
      sprintf(outbuf, "%5s disk:\n\tVol. label \"%11s\"\n\tVol. ID %8X\n\tCluster size: %d sectors\n\tN. Fats: %d\n\tDisk %2xh\n\tVolume size %dB",
        &(bpb -> sys_ident),
        &(bpb -> vol_lbl),
        &(bpb -> serial_no),
        bpb -> sec_per_clust,
        bpb -> n_fats,
        &(hardware -> bios_disk),
        bpb -> n_sectors * bpb -> bytes_per_sec // TODO: move n_sectors into long anyway
      );

      fmt = COLOUR(RED, B_YELLOW); // fmt 
    } else if (strcmp(work.buf, "sys:indic")) {
      // indicators
      sprintf(outbuf, "scroll: %d\nnum: %d\ncaps: %d",
        bittest(&(keys -> modifs), 0),
        bittest(&(keys -> modifs), 1),
        bittest(&(keys -> modifs), 2)
      );

      fmt = COLOUR(GREEN, RED);
    } else if (strcmp(work.buf, "sys:mem")) {
      void *addr = malloc(1);
      sprintf(outbuf, "First free memory addr: %p\n\t\tout of: %p\n\nCurrently running from: %p", addr, MEM_END, rip_thunk());
      free(addr);

      fmt = COLOUR(MAGENTA, B_YELLOW); // fmt
    } else if (strcmp(work.buf, "sys:proc")) {
      for (app_handle i = 0; i < AVAILABLE_KAPPS; ++i) {
        sprintf(outbuf, "@%d: %s\n", i, app_db[i] ? app_db[i]->app_name : "-");
        write_str(outbuf, app_db[i] ? 0xa : 0xc);
        memzero(outbuf, strlen(outbuf));
      }
    } else if (strcmp(work.buf, "time")) {
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

      fmt = COLOUR(RED, B_CYAN);
    } else {
      msg(WARN, E_UNKENTITY, "Unknown command");
      goto shell_clean;
    }

    if (fmt) {
      write_str(outbuf, fmt);
      line_feed();
    }

  shell_clean:
    free(outbuf);
    work.buf[wher] = safe_padding; // ok so jank way of keeping history intact
    return exitting; // exit normally
  }

public:
  // constructor
  KShell(int comlen = 32): KApp(), work(comlen) {
    // bitflags, or as many together as need be
    config_flags = NEEDS_ENTER_CTRLCODE;

    app_name = "kesh";

    histbuf = malloc(comlen);
  }

  // destructor
  ~KShell() { // madatory cleanup
    delete &work;
    delete histbuf;
  }
};
