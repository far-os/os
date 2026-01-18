#pragma once

#include "kapp.hh"
#include "extra/fromc.hh"

// text editor
struct Editor : KApp {
  void invoke();
  void first_run();

private:
  enum State {
    CLEAN = 0,
    CHANGES = 1,
    SHOW_EXIT_MODAL = 2,
  };

  struct inp_strbuf header; // first character
  struct inp_strbuf modal;
  State dirty; // whether the file has recently been saved
  struct inp_strbuf contents; // file buffer
  char filename[13]; // the exact file

  void read();
  void save();

public:
  Editor(char *which);
  ~Editor();
};
