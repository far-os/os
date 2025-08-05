#pragma once

#include "kapp.hh"
#include "extra/fromc.hh"

struct HelpHost : KApp {
  // virtual methods
  void invoke();
  void first_run();

public:
  // hmmmm
  enum EntType { // bitmask
    PLAIN_ENTRY = 0, // normal entry, nothing special
    SUB_ENTRY = 1 << 0, // indented, and preceded by a colon
    DEBUG_ENTRY = 1 << 1, // lighter text, "greyed out"
  };

  struct Entry {
    const char* name;
    const char* desc;
    EntType type;
  };

private:
  char* name;
  Entry* ents;

  unsigned int start_ix;

  void put_entries();

public:
  HelpHost(char* what, Entry* loads);
  ~HelpHost();
};
