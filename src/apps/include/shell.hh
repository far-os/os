#pragma once

#include "kapp.hh"
#include "extra/fromc.hh"

// shell applet
#define OUTBUF_LEN 160
struct KShell : KApp {
  // the old comupd and curupd are both here
  void invoke();
  void first_run();
 
protected: // hidden fields (only for internal use)
  struct inp_strbuf work; // the buffer in which we work

  virtual const char* get_prompt(); // prompt function
  virtual bool shexec();

private:
  char *histbuf; // copy of the history
  char *outbuf; // buffer which is where data is dumped

public:
  // constructor
  KShell(int comlen = 32);

  // destructor
  ~KShell();
};
