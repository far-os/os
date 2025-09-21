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
    unsigned int last = 0;
    unsigned int at = 0;
    do if (str[at] == delim || !(str[at])) {
      if (at != last) {
        char *ptr = malloc(at - last + 1);
        memcpy(str+last, ptr, at - last);
        this->push(ptr);
      } /**/
      last = at + 1;
    } while (str[at++]);
  }

  Split::~Split() {
    for (int i = 0; i < curr_size; ++i) free((*this)[i]);
    Vector::~Vector();
  }
}
