// Kernel Applets
// OOP works a bit better for this, so we are using c++
// don't worry, c++ is surprisingly easy to shoehorn into existing c code

#define k_app KApp
// all the function definitions, we just wrap them in `extern "C"' to stop the dreaded name mangling
extern "C" {
#include "../include/ata.h"
#include "../include/cmos.h"
#include "../include/config.h"
#include "../include/err.h"
#include "../include/fs.h"
#include "../include/hwinf.h"
#include "../include/kappldr.h"
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

void inp_strbuf::resize_by(int offset) {
  this->buf = realloc(this->buf, this->len += offset);
}

inp_strbuf::inp_strbuf(unsigned int with): len(with) {
  buf = malloc(len);
  this->clear(); // just in case
}

inp_strbuf::~inp_strbuf() {
  free(this->buf);
  this->buf = NULL;
}

// stops `new' and `delete' operators from shitting themselves
inline void* operator new(unsigned int, void* p) { return p; }
inline void* operator new[](unsigned int, void* p) { return p; }
inline void* operator new(unsigned int size) { return malloc(size); }

// hmmm
inline void operator delete(void* p) { free(p); }
inline void operator delete(void* p, unsigned int) { free(p); }

// with virtual methods, the *first* property of the struct (invisible to c++) is a `code**' type,
// pointing to a list of all that instance's virtual functions
// alternatively, it's be easier to create and expose a wrapper function
extern "C" struct KApp { // struct != class, class is everything's private by default (basically moronic)
  virtual void invoke() = 0; // = 0 is the jank syntax for saying that this class does not intend to define method (is "pure" virtual)
  virtual void first_run() = 0; // the first time it's run, as we dont want any setup messages to be printed on the wrong buffer

  // queue_len is def'd in text.h
  char key_q[QUEUE_LEN]; // normal keypress queue
  enum ctrl_char ctrl_q[QUEUE_LEN]; // control code queue (e.g. ^C, ^S, _F10, etc)

  const char *app_name; // name of application

  unsigned char app_id; // app identity: low nybble is current handle, high nybble is parent handle

  unsigned char config_flags; // configuration
  // bit 0: clear = accepts '\n' characters | set = accepts enter key


  KApp() {
    memzero(&key_q, QUEUE_LEN);
    memzero(&ctrl_q, QUEUE_LEN);
  }

  // we dont technically need to destruct anything, but out children might (but if we don't know they're a child we need virtuality)
  virtual ~KApp() {};

  /* we can ofc add more app-specific methods here in child classes */

protected: // only visible to this and children
  // it's a very common thing to need to do, write input keys to buffer, so we make a function to do it avoid repetition
  // returns the final length of the buffer, -1 if its too big (this will help us in reallocking)
  int write_keys_to_buf(struct inp_strbuf *to) {
    // if we'll be past the end of the buffer's allocation (comlen)
    // it'll error anyway, no point in putting characters
    char n_to_put = strlen(this->key_q);
    if (!n_to_put) return; // nothing to do beyond this point

    int targ_len = strlen(to->buf) + n_to_put;
    if (targ_len >= to->len) {
      msg(PROGERR, E_BUFOVERFLOW, "Buffer size exceeded");
      return -1;
    };

    backmemcpy(
      to->buf + to->len - (n_to_put + 1), // end of src
      to->buf + to->len - 1, // end of dest
      to->len - (to->ix + n_to_put) // length of memory to copy
    );
    strcpy(this->key_q, to->buf + to->ix);
    memzero(this->key_q, QUEUE_LEN);
    to->ix++;

    return targ_len;
  }
};

// all the app files
#include "helphost.hh"
#include "edit.hh"
//#include "atest.hh"
#include "shell.hh"

// c-friendly wrapper
extern "C" {
  // there are some private properties, that c's not supposed to ever know about (so it thinks it's smaller)
  KApp *mk_shell(int comlen) {
    return new KShell (comlen);
  };

  void kapp_destroy(KApp *a) {
    delete a;
  }
};
