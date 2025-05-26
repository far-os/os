struct HelpHost : KApp {
  void invoke() {
    // no real input

    // TODO: cursor keys, cause it is any key rn
    if (this -> ctrl_q[0] || this -> key_q[0]) {
      terminate_app(this->app_id & 0xf);
    }
  }

  void first_run() {
    clear_scr();
    set_cur(0);
    //cur_off();

    unsigned char pad_len = VGA_WIDTH - strlen(this->name) - 6;
    write_cell_cur(0xd5, COLOUR(BLUE, B_BLACK));
    write_str(" ? ", COLOUR(BLUE, B_RED));
    write_str(this->name, COLOUR(BLUE, B_WHITE));
    write_cell_cur(' ', COLOUR(BLUE, 0));
    for (int p = 0; p < pad_len; ++p) write_cell_cur(0xcd, COLOUR(BLUE, B_BLACK));
    write_cell_cur(0xb8, COLOUR(BLUE, B_BLACK));

    short furthest = 0;
    for (int ent = 0; this->ents[ent].type != -1; ++ent) {
      int hh = strlen(this->ents[ent].name);
      if (furthest < hh) furthest = hh;
    }

    for (int ent = 0; this->ents[ent].type != -1; ++ent) {
      write_cell_cur(0xb3, COLOUR(BLUE, B_BLACK));
      adv_cur();
      if (this->ents[ent].type == SUB_ENTRY) {
        adv_cur_by(2);
        write_cell_cur(':', COLOUR(BLUE, GREEN));
        write_str(this->ents[ent].name, COLOUR(BLUE, B_GREEN));
      } else {
        write_str(this->ents[ent].name, COLOUR(BLUE, B_YELLOW));
      }

      int cix = 0;
      for (; this->ents[ent].desc[cix] && (this->ents[ent].desc[cix] != '\xff'); ++cix);
      if (this->ents[ent].desc[cix] != '\xff') { // we can't edit the string to contain a null, because it would break it for future help runs.
                                                 // don't even think about fixing it afterwards. c++ "can't edit constants" or smth
        cix = -1;
      }

      set_cur(POS(furthest + (this->ents[ent].type) << 1, ent + 1));
      write_str("- ", COLOUR(BLUE, WHITE));

      if (cix != -1) {
        // budget write_str
        for (int q = 0; q < cix; ++q) {
          write_cell_cur(this->ents[ent].desc[q], COLOUR(BLUE, B_WHITE));
        }

        if (this->ents[ent].desc[cix + 1] == '[') {
          adv_cur();
          write_str(this->ents[ent].desc + cix + 1, COLOUR(BLUE, MAGENTA));
        } else if (this->ents[ent].desc[cix+1] == '<') {
          set_cur(POS(VGA_WIDTH - 8 - strlen(this->ents[ent].desc + cix + 1), ent + 1));
          write_str("args: ", COLOUR(BLUE, RED));
          write_str(this->ents[ent].desc + cix + 1, COLOUR(BLUE, B_RED));
        }
      } else { // sanity, at long last
        write_str(this->ents[ent].desc, COLOUR(BLUE, B_WHITE));
      }

      write_cell(0xb3, (ln_nr() + 1) * VGA_WIDTH - 1, COLOUR(BLUE, B_BLACK));
      paint_row(BLUE);
      line_feed();
    }


    write_cell_cur(0xd4, COLOUR(BLUE, B_BLACK));
    for (int p = 2; p < VGA_WIDTH; ++p) write_cell_cur(0xcd, COLOUR(BLUE, B_BLACK));
    write_cell_cur(0xbe, COLOUR(BLUE, B_BLACK));
    write_str("\n\020 any key to continue...  ", COLOUR(BLACK, RED));
    adv_cur_by(-1);
  }

public:
  // hmmmm
  enum EntType {
    PLAIN_ENTRY,
    SUB_ENTRY,
  };

  struct Entry {
    const char* name;
    const char* desc;
    EntType type;
  };

private:
  char* name;
  Entry* ents;

public:
  HelpHost(char* what, Entry* loads): ents(loads), name(what) {
    // bitflags, or as many together as need be
    config_flags = NEEDS_ENTER_CTRLCODE; // enter can exit too alright

    app_name = "helphost";
  }

  ~HelpHost() {
//    cur_on();
  }
};
