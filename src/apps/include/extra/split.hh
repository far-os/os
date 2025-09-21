#include "vector.hh"

#pragma once

namespace Extra {
  class Split : public Vector<char *> {
  public:
    void pop();

    Split(char *str, char delim);
    ~Split();
  };
}
