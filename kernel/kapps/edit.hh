const HelpHost::Entry keyb_shortcuts[] = {
  { .name = "\340K", .desc = "Clear line", .type = HelpHost::PLAIN_ENTRY },
  { .name = "^H", .desc = "Show this help menu", .type = HelpHost::PLAIN_ENTRY },
  { .name = "^S", .desc = "Save", .type = HelpHost::PLAIN_ENTRY },
  { .name = "<Esc>", .desc = "Quit\0[without saving]", .type = HelpHost::PLAIN_ENTRY },
  { .type = -1 }
};

// text editor
struct Editor : KApp {
  void invoke() {
    if (this->key_q[0]) dirty = true;
    this->write_keys_to_buf(&this->contents);

    for (int i = 0; i < QUEUE_LEN && !!this->ctrl_q[i]; ++i) { // loop over each arrow key
      switch (this -> ctrl_q[i]) {
        case NO_CTRL: break; // already dealt with, should never happen
        case CTRL(H): {
          app_handle help = instantiate(
            new HelpHost("text editor", keyb_shortcuts),
            this->app_id & 0xf,
            true
          );
          goto no_fill;
        }
        case CTRL(S): {
          this->save();
          break;
        }
        case ALT(K): {
          int index = 0;
          for (int piece = 1; piece < this->contents.ix; piece++) {
            if (index < piece && this->contents.buf[piece] == '\n') {
              index = piece + 1;
            }
          }

          do {
            dirty = true;
            this->contents.delchar_at(index);
          } while (this->contents.buf[index] && this->contents.buf[index] != '\n');
          this->contents.delchar_at(index);

          this->contents.ix = index - 1;
          break;
        }
        case ESC:
          terminate_app(this->app_id & 0xf);
          return;
        case BACKSPACE:
        case DEL: {
          dirty = true;
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
    this->header.buf[0] = dirty ? 0x07 : 0x20;
    this->first_run();

no_fill:
    memzero(this->ctrl_q, QUEUE_LEN);
  };

  void first_run() {
    clear_scr();
    set_cur(0);
    write_str(this->contents.buf, COLOUR(BLACK, WHITE));
    memcpy(header.buf, addr_of_loc(POS(0, VP_HEIGHT - 1)), header.len);
    set_cur(trace_ch_until(contents.buf, contents.ix)); // trace character
  }

private:
  struct inp_strbuf contents; // file buffer
  inode_n file; // the exact file
  struct inp_strbuf header; // first character
  bool dirty; // whether the file has recently been saved

  void read_file() {
    read_inode(
      file,
      contents.buf
    );
  };

  void save() {
    dirty = false;
    write_inode(
      file,
      contents.buf
    );
  };

public:
  // constructor
  Editor(inode_n which) : KApp(), file(which), contents(file_table[which].loc.len << 9), header(VGA_WIDTH * 2), dirty(false) {
    config_flags = 0; // no flags - we want enter to appear as a real key

    app_name = "fedit";

    // get file, has already been initted above
    this->read_file();
    contents.ix = strlen(contents.buf);

    // "^S to save, <Esc> to exit" is 31 ch long
    unsigned char pad_len = VGA_WIDTH - strlen(file_table[file].name) - 11 - 4; // 4 for padding
    write_cell_into(&header, ' ', COLOUR(RED, B_GREEN));
    write_str_into(&header, file_table[file].name, COLOUR(RED, B_WHITE));
    write_cell_into(&header, ' ', COLOUR(RED, 0));
    for (int p = 0; p < pad_len; ++p) write_cell_into(&header, 0xcd, COLOUR(RED, B_BLACK));
    write_cell_into(&header, ' ', COLOUR(RED, 0));
    write_str_into(&header, "^H", COLOUR(RED, B_CYAN));
    write_str_into(&header, " for help ", COLOUR(RED, B_YELLOW));
  }

  ~Editor() {
    header.~inp_strbuf();
    contents.~inp_strbuf();
  }
};
