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
    DIVIDER = 1 << 2, // a load of dashes
    SYNOPSIS = 1 << 3, // a straight line of text.
    TERMINATE = -1,
  };

  struct Entry {
    const char* name;
    const char* desc;
    EntType type;
  };

private:
  char* name;
  Entry* ents;

  unsigned int top_entry;

  void put_entries() const;

  // helper methods
  void HelpHost::draw_edge(bool edge) const;
  unsigned int HelpHost::tally_entries() const;
  void HelpHost::put_single_entry(const Entry& ent, unsigned int row_num, unsigned int desc_indent) const;

public:
  HelpHost(char* what, const Entry* loads);
  ~HelpHost();
};
