#pragma once

#include "kapp.hh"
#include "extra/fromc.hh"
#include "extra/vector.inl.hh"

using namespace Extra;

#include "shell.hh"

// calculator. is basically a shell, so is actually a child of KShell (cheating)
struct Calc : KShell {
  // no invoke, as it is identical to the shell (our parent)
  void first_run();

protected:
  const char* get_prompt(); // prompt function

  bool shexec();

// - end of inheritance

  Vector<int> stack; // resizeable vector

public:
  Calc();
};

