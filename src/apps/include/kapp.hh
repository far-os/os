#pragma once

// import kernel stuff
#include "extra/fromc.hh"

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

  // constructor
  KApp();

  // virtual destructor: allow our children to kill themselves
  virtual ~KApp();

  /* we can ofc add more app-specific methods here in child classes */

protected: // only visible to this and children
  // write keys util function, see kapp.cc for more info
  int write_keys_to_buf(struct inp_strbuf *);
};
