// Kernel Applets
// OOP works a bit better for this, so we are using c++
// don't worry, c++ is surprisingly easy to shoehorn into existing c code

// all the function definitions, we just wrap them in `extern "C"' to stop the dreaded name mangling
extern "C" {
#include "../include/text.h"
#include "../include/memring.h"
#include "../include/misc.h"
#include "../include/util.h"
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

  KApp() {
    memzero(&key_q, QUEUE_LEN);
    memzero(&ctrl_q, QUEUE_LEN);
  }
  /* we can ofc add more app-specific methods here in child classes */
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
