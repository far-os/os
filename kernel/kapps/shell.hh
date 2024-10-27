// shell applet
#define SHELL_PROMPT_WIDTH 4
struct KShell : KApp {
  void invoke() { // here, is effectively equivalent to curupd and comupd
    // TODO: read from ctrl_q

    // curupd

    // if we'll be past the end of the buffer's allocation (comlen)
    // it'll error anyway, no point in putting characters
    char n_to_put = strlen(this->key_q);
    if (!n_to_put) return; // nothing to do beyond this point

    if (this->work.ix + n_to_put > this->work.len) return;

    backmemcpy(
      this->work.buf + this->work.len - (n_to_put + 1), // end of src
      this->work.buf + this->work.len - 1, // end of dest
      this->work.len - (this->work.ix + n_to_put) // length of memory to copy
    );
    strcpy(this->key_q, this->work.buf + this->work.ix);
    memzero(this->key_q, QUEUE_LEN);
    this->work.ix++;

  comupd: 
    write_str(this->work.buf - SHELL_PROMPT_WIDTH, COLOUR(BLACK, WHITE));
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
  }

  // destructor
  ~KShell() { // madatory cleanup
    free(work.buf - SHELL_PROMPT_WIDTH);
    work.buf = NULL;

    free(histbuf);
    histbuf = NULL;
  }
};
