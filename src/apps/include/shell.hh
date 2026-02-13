#pragma once

#include "kapp.hh"
#include "extra/fromc.hh"

// shell applet
struct KShell : KApp {
  // the old comupd and curupd are both here
  void invoke();
  void first_run();

protected: // hidden fields (only for internal use)
  struct inp_strbuf work; // the buffer in which we work

  virtual const char* get_prompt(); // prompt function
  virtual const char* motd_msg(); // motd message
  virtual bool shexec();

private:
  char *histbuf; // copy of the history

public:
  // constructor
  KShell(int comlen = 32);

  // destructor
  ~KShell();
};
