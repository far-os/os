#include "include/helphost.hh"
#include "include/calc.hh"
#include "include/edit.hh"
#include "include/extra/split.hh"

#include "include/shell.hh"

// list of entries, Entry and EntType are children of HelpHost
const HelpHost::Entry comnames[] = {
  { .name = "calc", .desc = "Calculator applet", .type = HelpHost::PLAIN_ENTRY },
  { .name = "clr", .desc = "Clears screen\xff[or shift+\23710]", .type = HelpHost::PLAIN_ENTRY },
  { .name = "exec", .desc = "Executes program\xff<u32>", .type = HelpHost::PLAIN_ENTRY },
  { .name = "f", .desc = "File I/O namespace", .type = HelpHost::PLAIN_ENTRY },
  { .name = "ls", .desc = "Lists all files", .type = HelpHost::SUB_ENTRY },
  { .name = "new", .desc = "Creates file\xff<filename>", .type = HelpHost::SUB_ENTRY },
  { .name = "ren", .desc = "Renames file\xff<filename> x2", .type = HelpHost::SUB_ENTRY },
  { .name = "stat", .desc = "Prints info about given file\xff<filename>", .type = HelpHost::SUB_ENTRY },
  { .type = HelpHost::SUB_ENTRY | HelpHost::DIVIDER },
  { .name = "edit", .desc = "Text editor\xff<filename>", .type = HelpHost::SUB_ENTRY },
  { .name = "read", .desc = "Prints file content\xff<filename>", .type = HelpHost::SUB_ENTRY },
  { .name = "help", .desc = "Prints this help menu", .type = HelpHost::PLAIN_ENTRY },
  { .name = "proc", .desc = "Utilities that manage running processes", .type = HelpHost::PLAIN_ENTRY },
  { .name = "ls", .desc = "Prints currently running processes", .type = HelpHost::SUB_ENTRY },
  { .name = "kill", .desc = "Kills the specified process\xff<@handle>", .type = HelpHost::SUB_ENTRY },
  { .name = "reset", .desc = "Resets machine\xff[or ctrl+alt+del]", .type = HelpHost::PLAIN_ENTRY },
  { .name = "split", .desc = "Forms a split-screen with another open app\xff<@handle>", .type = HelpHost::PLAIN_ENTRY },
  { .name = "set", .desc = "Gets/Sets config variables (see sub-entries)", .type = HelpHost::PLAIN_ENTRY },
  { .name = "verbose", .desc = "Sets verbose level. Max 2.\xff<u8>?", .type = HelpHost::SUB_ENTRY },
  { .name = "sys", .desc = "Utilities that print/dump system info", .type = HelpHost::PLAIN_ENTRY },
  { .name = "ata", .desc = "Prints ATA disk info", .type = HelpHost::SUB_ENTRY },
  { .name = "cpu", .desc = "Prints CPU info", .type = HelpHost::SUB_ENTRY },
  { .name = "fs", .desc = "Prints file system info", .type = HelpHost::SUB_ENTRY },
  { .name = "indic", .desc = "Prints keyboard LED status", .type = HelpHost::SUB_ENTRY },
  { .name = "mem", .desc = "Prints memory info", .type = HelpHost::SUB_ENTRY },
  { .name = "time", .desc = "Gets current time", .type = HelpHost::PLAIN_ENTRY },
  { .name = "util", .desc = "Utilities and tools", .type = HelpHost::PLAIN_ENTRY },
  { .name = "to8.3", .desc = "Canonicalises a filename into 8.3\xff<string>", .type = HelpHost::SUB_ENTRY },
  { .type = HelpHost::TERMINATE }
};

void KShell::invoke() { // here, is effectively equivalent to comupd and curupd
  // cause bools exist here
  bool to_exec = false;
  bool exitting = true;

  // write queued keys to the buffer, it's a parent class function as it's quite common
  if (this->write_keys_to_buf(&this->work) < 0) {
    goto histify;
  }

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
  write_str(this->get_prompt(), COLOUR(BLACK, WHITE));
  write_str(this->work.buf, COLOUR(BLACK, WHITE));

curupd:
  set_cur(POS(trace_ch_until_with(this->work.buf, this->work.ix, strlen(this->get_prompt()) - 1), ln_nr()));

  if (!to_exec) return;

  line_feed();

  if (!strlen(this->work.buf)) goto dehistify;
  
  exitting = this->shexec();

histify:
  memcpy(this->work.buf, histbuf, this->work.len);
  this->work.clear();

dehistify:
  to_exec = 0;
  if (exitting) goto comupd; // get prompt to reappear
}

void KShell::first_run() {
  /*
  write_str("os ", 0x07); */
  write_str("Kernel Executive Shell. (c) 2022-6.\n", COLOUR(BLUE, B_RED));

  // print out prompt
  write_str(get_prompt(), COLOUR(BLACK, WHITE));
}

// the meat of the shell
bool KShell::shexec() {
  if (strlen(this->work.buf) == 0) return true;

  // split work.buf by spaces. returns Split, which is a type of Vector
  Extra::Split argv(work.buf, ' ');

  bool exitting = true; // whether we exit normally

  if (strcmp(argv[0], "calc")) {
    app_handle help = instantiate(
      new Calc(),
      this->app_id & 0xf,
      true
    );

    exitting = false;
    goto shell_clean;
  } else if (strcmp(argv[0], "clr")) { // clear screen
    clear_scr();
    set_cur(0);
  } else if (strcmp(argv[0], "exec")) {
    read_file(
      "prog.bin",
      0x100000
    ); // reads disk, has to get master or slave

    int ar = -1;
    if (argv.len() > 1) {
      ar = to_uint(argv[1]);
    }

    enum ERRSIG ret = prog(ar);
    if (ret == E_BOUND) {
      msg(PROGERR, ret, "Program not found");
    } else if (ret == E_ILLEGAL) {
      msg(KERNERR, ret, "Program executed illegal instruction");
    } else {
      line_feed();
    }
  } else if (strcmp(argv[0], "f:ls")) {
    char *name = malloc(13);
    for (unsigned int search = 0; VALID_FILE(root_dir[search]); search++) {
      sane_name(root_dir[search].name, name);
      printf("%$%s\t", COLOUR(BLACK, B_WHITE), name);
    }
    free(name);

    line_feed();
  } else if (strcmp(argv[0], "f:new")) {
    if (argv.len() <= 1) {
      msg(PROGERR, E_ARGS, "Please provide a filename");
      goto shell_clean;
    }

    if (create_file(argv[1])) {
      goto shell_f_stat;
    }
  } else if (strcmp(argv[0], "f:ren")) {
    if (argv.len() <= 2) {
      msg(PROGERR, E_ARGS, "Please provide two source and destination filenames");
      goto shell_clean;
    }

    rename_file(argv[1], argv[2]);
  } else if (strcmp(argv[0], "f:stat")) {
shell_f_stat:
    struct dir_entry *f = NULL;
    if (!f) {
      f = get_file(argv[1]);
      if (!f) goto shell_clean;
    }

    struct timestamp ctime = from_dostime({
      .dostime = f->ctime,
      .dosdate = f->cdate,
      .centisecs = f->ctime_cs
    });

    struct timestamp mtime = from_dostime({
      .dostime = f->mtime,
      .dosdate = f->mdate
    });

    struct timestamp atime = from_dostime({
      .dosdate = f->adate
    });

    char *name = malloc(13);
    sane_name(f->name, name);

    // returns to a global buffer. be not confused, be horrified
    attribify(f->attrib);

    printf("%$%s\n\t%d bytes, intial cluster %3X\n\tAttributes: %s\n\tCreated  %4d-%2d-%2d %2d:%2d:%2d\n\tModified %4d-%2d-%2d %2d:%2d:%2d\n\tAccessed %4d-%2d-%2d\n",
      COLOUR(BLUE, B_YELLOW),
      name,
      f->size,
      &(f->first_cluster_lo),
      attribify_buf,
      ctime.year,
      ctime.month,
      ctime.date,
      ctime.hour,
      ctime.minute,
      ctime.second,
      mtime.year,
      mtime.month,
      mtime.date,
      mtime.hour,
      mtime.minute,
      mtime.second,
      atime.year,
      atime.month,
      atime.date
    );

    free(name);
  } else if (strcmp(argv[0], "f:edit")) {
    // TODO: allow no file
    struct dir_entry *f = NULL;
    if (!f) {
      f = get_file(argv[1]);
      if (!f) goto shell_clean;
    }

    app_handle edt = instantiate(
      new Editor(argv[1]),
      this->app_id & 0xf,
      true
    );

    exitting = false;
    goto shell_clean;
  } else if (strcmp(argv[0], "f:read")) {
    struct dir_entry *f = NULL;
    if (!f) {
      f = get_file(argv[1]);
      if (!f) goto shell_clean;
    }

    char *datablk = malloc(f->size);
    read_file(
      argv[1],
      datablk
    ); // reads disk, has to get master or slave
    write_str(datablk, COLOUR(BLACK, WHITE));

    free(datablk);
    line_feed();
  } else if (strcmp(argv[0], "help")) {
    app_handle help = instantiate(
      new HelpHost("shell builtins", comnames),
      this->app_id & 0xf,
      true
    );

    exitting = false;
    goto shell_clean;
  } else if (strcmp(argv[0], "proc:ls")) {
    for (app_handle i = 0; i < AVAILABLE_KAPPS; ++i) {
      printf("%$@%d: %s\n", app_db[i] ? 0xa : 0xc, i, app_db[i] ? app_db[i]->app_name : "-");
    }
  } else if (strcmp(argv[0], "proc:kill")) {
    if (argv[1][0] == '@') {
      int ar = -1;
      if (strlen(argv[1]) > 1) {
        ar = to_uint(argv[1] + 1);
      }

      if (ar == -1) goto _kesh_proc_kill_fail;
      else if (!app_db[ar]) goto _kesh_proc_kill_fail;

      if (ar == (this->app_id & 0xf)) {
        msg(PROGERR, E_SUICIDE, "Cannot kill current process");
      } else terminate_app(ar);
    } else {
    _kesh_proc_kill_fail:
      msg(PROGERR, E_ARGS, "No valid handle supplied");
    }
  } else if (strcmp(argv[0], "reset")) {
    cpu_reset();
    // if you reach past here, something has gone very wrong indeed
  } else if (strcmp(argv[0], "set:verbose")) {
    int ar = -1;
    if (argv.len() > 1) {
      ar = to_uint(argv[1]);
    }

    if (ar != -1) {
      printf("%$was %d\n%$now ", COLOUR(BLUE, B_WHITE), xconf -> verbosity, COLOUR(BLUE, B_GREEN));
      xconf->verbosity = ar;
      WRITE_CONF();
    }

    printf("%$verbose = %d\n",
      ar != -1 ? COLOUR(BLUE, B_GREEN) : COLOUR(BLUE, B_YELLOW),
      xconf -> verbosity
    );
  } else if (strcmp(argv[0], "split")) {
    if (strcmp(argv[1], "off")) {
      split_scr(0);
    } else if (argv[1][0] == '@') {
      int ar = -1;
      if (strlen(argv[1]) > 1) {
        ar = to_uint(argv[1] + 1);
      }

      if (ar == -1) goto _kesh_split_fail;
      else if (!app_db[ar]) goto _kesh_split_fail;

      split_scr(ar);
      focus_app(ar);
      app_db[ar]->invoke();
      exitting = false;
    } else {
    _kesh_split_fail:
      msg(PROGERR, E_ARGS, "No valid handle supplied");
    }
  } else if (strcmp(argv[0], "sys:ata")) {
    printf("%$IDENTIFY of disk %2xh:\n\tLBA28\t%d sectors\n\tLBA48\t%B, %l sectors\n\tUDMA\t80CC: %B, %4x\n",
      COLOUR(RED, B_GREEN),
      &(hardware -> bios_disk),
      ATA_N_LBA28,
      ATA_HAS_LBA48,
      ATA_N_LBA48,
      ATA_HAS_80CC,
      &(ATA_UDMA_MODES)
    );
  } else if (strcmp(argv[0], "sys:cpu")) {
    printf("%$CPUID.\n\t\x10 %12s\n\tFamily %2xh, Model %2xh, Stepping %1xh\n\tBrand \"%s\"\n",
      COLOUR(YELLOW, B_GREEN),
      &(hardware -> vendor),
      &(hardware -> c_family), // family
      &(hardware -> c_model), // model
      &(hardware -> c_stepping), // stepping
      hardware -> cpuid_ext_leaves >= 0x80000004 ? &(hardware -> brand) : NULL // brand string
    );
  } else if (strcmp(argv[0], "sys:fs")) {
    printf("%$%5s disk:\n\tVol. label \"%11s\"\n\tVol. ID %8X\n\tCluster size: %d sectors\n\tN. Fats: %d\n\tVolume size %dKiB\n",
      COLOUR(RED, B_YELLOW),
      &(bpb -> sys_ident),
      &(bpb -> vol_lbl),
      &(bpb -> serial_no),
      bpb -> sec_per_clust,
      bpb -> n_fats,
      (bpb -> n_sectors * bpb -> bytes_per_sec) >> 10
    );
  } else if (strcmp(argv[0], "sys:indic")) {
    // indicators
    printf("%$scroll: %d\nnum: %d\ncaps: %d\n",
      COLOUR(GREEN, RED),
      bittest(&(keys -> modifs), 0),
      bittest(&(keys -> modifs), 1),
      bittest(&(keys -> modifs), 2)
    );
  } else if (strcmp(argv[0], "sys:mem")) {
    void *addr = malloc(1);
    printf("%$First free memory addr: %p\n\t\tout of: %p\n\nCurrently running from: %p\n", COLOUR(MAGENTA, B_YELLOW), addr, MEM_END, rip_thunk());
    free(addr);
  } else if (strcmp(argv[0], "time")) {
    printf("%$Time since kernel load: %d.%2ds\n%s%c%4d-%2d-%2d %2d:%2d:%2d\n",
      COLOUR(RED, B_CYAN),
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
  } else if (strcmp(argv[0], "util:to8.3")) {
    char *name = malloc(13);
    canonicalise_name(argv[1], name);
    printf("%$%s\n", COLOUR(RED, B_GREEN), name);
    free(name);
  } else {
    msg(WARN, E_UNKENTITY, "Unknown command \"%s\"", argv[0]);
    goto shell_clean;
  }

shell_clean:
//  argv.~Split(); // NB: by the magic of "c++ does it for you"...
  return exitting; // exit normally
}

const char * KShell::get_prompt() {
  return "\r\x13> ";
}

// comlen has a default, check .hh
KShell::KShell(int comlen): KApp(), work(comlen) {
  // bitflags, or as many together as need be
  config_flags = NEEDS_ENTER_CTRLCODE;

  app_name = "kesh";

  histbuf = malloc(comlen);
}

KShell::~KShell() { // madatory cleanup
  // delete &work; NB: already magically destructured cause it's a property (thanks c++ i guess?)
  delete histbuf;
}
