// Kernel Applets
// OOP works a bit better for this, so we are using c++
// don't worry, c++ is surprisingly easy to shoehorn into existing c code

// all the function definitions, we just wrap them in `extern "C"' to stop the dreaded name mangling
extern "C" {
#include "../include/ata.h"
#include "../include/cmos.h"
#include "../include/config.h"
#include "../include/err.h"
#include "../include/fs.h"
#include "../include/hwinf.h"
#include "../include/kbd.h"
#include "../include/memring.h"
#include "../include/misc.h"
#include "../include/text.h"
#include "../include/util.h"
}

// already declared in an ifdef
void inp_strbuf::delchar_at(int at) {
  if (at < 0 || at >= strlen(this->buf)) return; // if trying to delete out of bounds

  memcpy(this->buf + at + 1, this->buf + at, this->len - at - 1);
  if (this->ix > at) { this->ix--; }
}

void inp_strbuf::clear() {
  memzero(this->buf, this->len);
  this->ix = 0;
}

// stops `new' and `delete' operators from shitting themselves
inline void* operator new(unsigned int, void* p) { return p; }
inline void* operator new[](unsigned int, void* p) { return p; }
inline void* operator new(unsigned int size) { return malloc(size); }

/* // hmmm
inline void operator delete(void* p) { free(p); }
inline void operator delete(void* p, unsigned int) { free(p); }
*/

// with virtual methods, the *first* property of the struct (invisible to c++) is a `code**' type,
// pointing to a list of all that instance's virtual functions
// alternatively, it's be easier to create and expose a wrapper function
extern "C" struct KApp { // struct != class, class is everything's private by default (basically moronic)
  virtual void invoke() = 0; // = 0 is the jank syntax for saying that this class does not intend to define method (is "pure" virtual)

  // queue_len is def'd in text.h
  char key_q[QUEUE_LEN]; // normal keypress queue
  enum ctrl_char ctrl_q[QUEUE_LEN]; // control code queue (e.g. ^C, ^S, _F10, etc)

  unsigned char config_flags; // configuration
  // bit 0: clear = accepts '\n' characters | set = accepts enter key

  KApp() {
    memzero(&key_q, QUEUE_LEN);
    memzero(&ctrl_q, QUEUE_LEN);
  }
  /* we can ofc add more app-specific methods here in child classes */

  // it's a very common thing to need to do, write input keys to buffer, so we make a function to do it avoid repetition
  void write_keys_to_buf(struct inp_strbuf *to) {
    // if we'll be past the end of the buffer's allocation (comlen)
    // it'll error anyway, no point in putting characters
    char n_to_put = strlen(this->key_q);
    if (!n_to_put) return; // nothing to do beyond this point

    if (strlen(to->buf) + n_to_put >= to->len) return;

    backmemcpy(
      to->buf + to->len - (n_to_put + 1), // end of src
      to->buf + to->len - 1, // end of dest
      to->len - (to->ix + n_to_put) // length of memory to copy
    );
    strcpy(this->key_q, to->buf + to->ix);
    memzero(this->key_q, QUEUE_LEN);
    to->ix++;
  }
};

// all the app files
#include "shell.hh"

// c-friendly wrapper
extern "C" {
  // there are some private properties, that c's not supposed to ever know about (so it thinks it's smaller)
  KShell *mk_shell(int comlen) {
    return new KShell (comlen);
  }
};
