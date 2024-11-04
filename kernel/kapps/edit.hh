// text editor
struct Editor : KApp {
  void invoke() {
    this->write_keys_to_buf(&this->contents);

    for (int i = 0; i < QUEUE_LEN && !!this->ctrl_q[i]; ++i) { // loop over each arrow key
      switch (this -> ctrl_q[i]) {
        case NO_CTRL: break; // already dealt with, should never happen
        case CTRL_S: {
          this->save();
          break;
        }
        case ESC:
          terminate_app(this->app_id & 0xf);
          return;
        case BACKSPACE:
        case DEL: {
          int offset = this->ctrl_q[i] - BACKSPACE; // 0 if backspace, 1 if delete (naive)
          this->contents.delchar_at(this->contents.ix + offset - 1);
          break;
        }
        case LEFT: {
          if (this->contents.ix != 0) this->contents.ix--;
          break;
        }
        case RIGHT: {
          if (this->contents.ix < strlen(this->contents.buf)) this->contents.ix++;
          break;
        }
        case PGUP: {
          this->contents.ix = 0;
          break;
        }
        case PGDOWN: {
          this->contents.ix = strlen(this->contents.buf);
          break;
        }
        default: break;
      }
    }
    memzero(this->ctrl_q, QUEUE_LEN);

    this->first_run();
  };

  void first_run() {
    clear_scr();
    set_cur(0);
    write_str(this->contents.buf, COLOUR(BLACK, WHITE));
    memcpy(header.buf, addr_of_loc(POS(0, VGA_HEIGHT - 1)), header.len);
    set_cur(trace_ch_until(contents.buf, contents.ix)); // trace character
  }

private:
  struct inp_strbuf contents; // file buffer
  inode_n file; // the exact file
  struct inp_strbuf header;

  void read_file() {
    read_inode(
      file,
      contents.buf
    );
  };

  void save() {
    write_inode(
      file,
      contents.buf
    );
  };

public:
  // constructor
  Editor(inode_n which) : KApp(), file(which), contents(file_table[which].loc.len << 9), header(VGA_WIDTH * 2) {
    config_flags = 0; // no flags - we want enter to appear as a real key

    // get file, has already been initted above
    this->read_file();
    contents.ix = strlen(contents.buf);

    // "^S to save, <Esc> to exit" is 31 ch long
    unsigned char pad_len = VGA_WIDTH - strlen(file_table[file].name) - 25 - 4; // 4 for padding
    write_cell_into(&header, ' ', COLOUR(RED, 0));
    write_str_into(&header, file_table[file].name, COLOUR(RED, B_WHITE));
    write_cell_into(&header, ' ', COLOUR(RED, 0));
    for (int p = 0; p < pad_len; ++p) write_cell_into(&header, 0xcd, COLOUR(RED, GREEN));
    write_cell_into(&header, ' ', COLOUR(RED, 0));
    write_str_into(&header, "^S", COLOUR(RED, B_CYAN));
    write_str_into(&header, " to save, ", COLOUR(RED, B_YELLOW));
    write_str_into(&header, "<Esc>", COLOUR(RED, B_CYAN));
    write_str_into(&header, " to exit", COLOUR(RED, B_YELLOW));
    write_cell_into(&header, ' ', COLOUR(RED, 0));
  }

  ~Editor() {
    header.~inp_strbuf();
    contents.~inp_strbuf();
  }
};
