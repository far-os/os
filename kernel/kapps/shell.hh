// shell applet
#define SHELL_PROMPT_WIDTH 4
struct KShell : KApp {
  void invoke() { // here, is effectively equivalent to comupd and curupd
    // write queued keys to the buffer, its a parent class function as it's quite common
    this->write_keys_to_buf(&this->work);

    // control key processing
    for (int i = 0; i < QUEUE_LEN && !!this->ctrl_q[i]; ++i) { // loop over each arrow key
      switch (this -> ctrl_q[i]) {
        case NO_CTRL: break; // already dealt with, should never happen
        case CTRL_C: {
          write_str("^C\n", COLOUR(BLACK, B_BLACK));
          memzero(this->work.buf, this->work.len);
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
        case UP: { /* TODO: hist restore */ break; }
        case LEFT: {
          if (this->work.ix != 0) this->work.ix--;
          break;
        }
        case RIGHT: {
          if (this->work.ix >= strlen(this->work.buf)) this->work.ix++;
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
  }
 
private: // hidden fields (only for internal use)
  struct inp_strbuf work; // the buffer in which we work
  char *histbuf; // copy of the history

public:
  // constructor
  KShell(int comlen = 28): KApp() {
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
