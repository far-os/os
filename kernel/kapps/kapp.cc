// Kernel Applets
// oop works a bit better for this, so we are using c++
// don't worry, c++ is surprisingly easy to shoehorn into existing c code

extern "C" {
#include "../include/text.h"
#include "../include/memring.h"
}

inline void* operator new(unsigned int size) { return malloc(size); }


// not "real" stdlib
//#include <new>

// with virtual methods, the *first* property of the struct (invisible to c++) is a `code**' type,
// pointing to a list of all that instance's virtual functions
// alternatively, it's be easier to create and expose a wrapper function
extern "C" struct KApp { // struct != class, class is everything's private by default (basically moronic)
  virtual void invoke() = 0; // = 0 is jank of way of saying that this class does not intend to define method (is purely virtual)
  struct inp_strbuf workbuf; // buffer in which we work
  struct inp_strbuf ctrlbuf; // control code buffer (e.g. ^C, ^S, _F10, etc)
  /* we can ofc add more app-specific methods here in child classes */
};

#define NULL nullptr

// shell applet
struct KShell : KApp {
  void invoke() {
    write_str("invocation\n", 0x0f);
  }

  // constructor
  KShell(int comlen = 32) {
    workbuf = inp_strbuf {
      .buf = NULL,
      .len = comlen,
      .ix = 0
    };

    ctrlbuf = inp_strbuf {
      .buf = NULL,
      .len = 4,
      .ix = 0
    };

    write_cell_cur('f', 0x0a);
    write_cell_cur('a', 0x0c);
    write_cell_cur('r', 0x0e);
    write_str("os Kernel Executive Shell. (c) 2022-4.\n", 0x07);
  }
};

extern "C" {
  KShell *mk_shell() {
    return new KShell ();
  }
};
