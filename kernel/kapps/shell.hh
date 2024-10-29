#include "sh_builtins.hh"

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
    if (strlen(this->work.buf) == 0) return;

    char found = -1; // have we found a command yet
    for (int cm = 0; cmdlist[cm].name; ++cm) {
      int checked_len = strlen(cmdlist[cm].name);
      if (memcmp(this->work.buf, cmdlist[cm].name, checked_len) && is_whitespace(this->work.buf[checked_len])) { // memcmp, because of arguments, and check that command doesnt go on
        found = cm;
        break;
      }
    }

    if (found == -1) {
      msg(WARN, E_UNKENTITY, "Unknown command");
      return;
    }

    char *outbuf = malloc(OUTBUF_LEN);

    // we pass the out buffer, and the arguments
    unsigned char result = (*cmdlist[found].func)(
      outbuf,
      this->work.buf + strlen(cmdlist[found].name) + 1
    );

    if (result) {
      write_str(outbuf, result);
      line_feed();
    }

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
