#pragma once

#include "kapp.hh"
#include "extra/fromc.hh"

// text editor
struct Editor : KApp {
  void invoke();
  void first_run();

private:
  struct inp_strbuf contents; // file buffer
  char filename[13]; // the exact file
  struct inp_strbuf header; // first character
  struct inp_strbuf modal; 
  unsigned char dirty; // whether the file has recently been saved, jank three-way bool:
                       // 0 = just saved
                       // 1 = made changes
                       // 2 = are you sure? modal

  void read();
  void save();

public:
  Editor(char *which);
  ~Editor();
};
