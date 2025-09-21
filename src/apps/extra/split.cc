#include "../include/extra/fromc.hh"
#include "../include/extra/vector.hh"
#include "../include/extra/split.hh"

namespace Extra {
  void Split::pop() {
    char *ptr = Vector::pop();
    free(ptr);
  }

  // diy "split" function
  Split::Split(char *str, char delim) {
    int last = 0;
    int at = 0;
    do if (str[at] == delim || !(str[at])) {
      char *ptr = malloc(at - last + 1);
      memcpy(str+last, ptr, at - last);
      this->push(ptr);
      last = at;
    } while (str[at++]);
  }

  Split::~Split() {
    //for (int i = 0; i < curr_size; ++i) free((*this)[i]);
    Vector::~Vector();
  }
}
