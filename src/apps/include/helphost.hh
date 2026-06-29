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
  const char* name;
  Entry* ents;

  // first entry currently being shown
  unsigned int top_entry;

  // drawe everything on screen
  void put_entries() const;

  // helper methods to put_entries()
  void draw_edge(bool edge) const;
  unsigned int tally_entries() const;
  void put_single_entry(const Entry& ent, unsigned int row_num, unsigned int desc_indent) const;

public:
  HelpHost(const char* title, const Entry* loads);
  ~HelpHost();
};
