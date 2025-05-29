const HelpHost::Entry keyb_shortcuts[] = {
  { .name = "^H", .desc = "Show this help menu", .type = HelpHost::PLAIN_ENTRY },
  { .name = "^S", .desc = "Save", .type = HelpHost::PLAIN_ENTRY },
  { .name = "alt+K", .desc = "Clear line", .type = HelpHost::PLAIN_ENTRY },
  { .name = "shift+\23712", .desc = "Insert 80 `a`s", .type = HelpHost::PLAIN_ENTRY | HelpHost::DEBUG_ENTRY },
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
        case SHIFT_F(12): {
          memset(endof(this->contents.buf), 80, 'a');
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
    write_str(contents.buf, COLOUR(BLACK, WHITE));
    memcpy(header.buf, addr_of_loc(POS(0, VP_HEIGHT - 1)), header.len);
    set_cur(trace_ch_until(contents.buf, contents.ix)); // trace character
  }

private:
  struct inp_strbuf contents; // file buffer
  char filename[13]; // the exact file
  struct inp_strbuf header; // first character
  bool dirty; // whether the file has recently been saved

  void read() {
    //contents.buf[0] = '\0';
    read_file(
      filename,
      contents.buf
    );
  };

  void save() {
    dirty = false;
    write_file(
      filename,
      contents.buf,
      strlen(contents.buf)
    );
  };

public:
  // constructor
  Editor(char *which) : KApp(), header(VGA_WIDTH * 2), dirty(false), contents(get_file(which)->size + 128) {
    config_flags = 0; // no flags - we want enter to appear as a real key

    app_name = "edit";

    // get file, has already been initted above
    struct dir_entry *f = get_file(which);

    sane_name(f->name, this->filename);

    this->read();
    contents.ix = strlen(contents.buf);


    // "^S to save, <Esc> to exit" is 31 ch long
    unsigned char pad_len = VGA_WIDTH - strlen(filename) - 11 - 4; // 4 for padding
    write_cell_into(&header, ' ', COLOUR(RED, B_GREEN));
    write_str_into(&header, filename, COLOUR(RED, B_WHITE));
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
