extern "C" {
#include "err.h"
#include "memring.h"
#include "text.h"
#include "util.h"
}

struct fchar {
  char ch;
  unsigned char fmt;
} __attribute__((packed));

template <typename Slot>
struct textbuf {
public:
  unsigned int len;
  Slot *buf;
  unsigned int ix;

  void clear() {
    memzero(this->buf, this->len);
    this->ix = 0;
  };

  void resize_by(int offset) {
    this->buf = realloc(this->buf, this->len += offset);
  }

  textbuf(unsigned int with): len(with) {
    buf = malloc(len);
    this->clear(); // just in case
  }

  textbuf(void *predef, unsigned int size) : buf(predef), len(size) {
    this->clear();
  }

  ~textbuf() {
    free(this->buf);
    this->buf = NULL;
  }

  // FIXME: this is broken. msg calls write_str, which calls this function, so if index isn't set correctly, infinite loop.
  // eventually stack blows up, so nothing happens
  Slot& operator[] (int at) {
    if (at >= this->len) {
      msg(PROGERR, E_BUFOVERFLOW, "Attempted to access past end of buffer");
    }
    return buf[at];
  }

  Slot& operator* () {
    return (*this)[this->ix];
  }
};

struct inp_strbuf : textbuf<char> {
  inp_strbuf(unsigned int size) : textbuf(size) { }

  void delchar_at(int at) {
    if (at < 0 || at >= strlen(this->buf)) return; // if trying to delete out of bounds

    memcpy(this->buf + at + 1, this->buf + at, this->len - at - 1);
    if (this->ix > at) { this->ix--; }
  };
};

// formatted videobuf
struct f_videobuf : textbuf<struct fchar> {
public:
  unsigned char page;

  f_videobuf(unsigned int size) : textbuf(size) { }

  f_videobuf(void *predef, unsigned int size) : textbuf(predef, size) {
    this->page = 0;
  }

  struct fchar& operator[] (int at) {
    if (at >= this->len) {
//      msg(PROGERR, E_BUFOVERFLOW, "Attempted to access past end of buffer");
    }
    return buf[at + page << 11];
  }

  short ln_nr() {
    return this->ix / VGA_WIDTH;
  }

  void line_feed() {
    short i = ln_nr();
    this->ix = ++i * VGA_WIDTH;
  }

  void carriage_return() {
    short i = ln_nr();
    this->ix = i * VGA_WIDTH;
  }

  void tab() {
    short i = this->ix / TAB_WIDTH;
    this->ix = (i + 1) * TAB_WIDTH;
  }

  void v_tab() {
    this->ix += VGA_WIDTH;
  }

  // don't think about it too hard
  void paint_row(unsigned char colr, int ln = -1) {
    ln = ln == -1 ? this->ln_nr() : ln;
    for (int k = 0; k < VGA_WIDTH; ++k) {
      (*this)[POS(k, ln)].fmt &= 0xf;
      (*this)[POS(k, ln)].fmt |= colr << 4;
    }
  }

  void paste(struct f_videobuf *rhs, int offs = 0) {
    memcpy(rhs->buf, &((*this)[offs]), rhs->len);
  }
};
