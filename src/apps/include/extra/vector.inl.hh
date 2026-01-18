#pragma once

#include "fromc.hh"
#include "bound.hh"

namespace Extra {
#define VECCAP_DEFAULT 16
  // shitty implementation of resizable Vec
  template <typename T>
  class Vector {
  protected:
    T* data;
    unsigned int capacity; // the max size, what is malloc'd basically
    unsigned int curr_size; // the reported size

    void grow() { // grow: fixes too big to fit in capacity
      this->data = (T *) realloc(this->data, (this->capacity *= 2) * sizeof(T));
    }

  public:
    // getter for curr_size
    unsigned int len() { return curr_size; }

    void push(T itm) {
      if (curr_size >= capacity) this->grow(); // if growing will kaboom
      this->data[this->curr_size++] = itm;
    }

    T pop() {
      if (!this->curr_size) {
        msg(WARN, E_BUFOVERFLOW, "Cannot pop empty Vector, ignoring");
        return 0;
      }

      // copy data, then remove it
      T keep = this->data[this->curr_size - 1];
      this->data[this->curr_size - 1] = 0;

      this->curr_size--; // shrink
      return keep; // return copy
    }

    // get at index
    T& operator[](int at) {
      struct BoundPacket bound {
        .first = this->data,
        .last = &(this->data[this->curr_size - 1])
      };

      // TODO: when we use a program, we probably want to use some form of assertion
      T& addr = this->data[at];
      if (!bound.contains(&addr)) { // we check if bound contains this.
        msg(PROGERR, E_BOUND, "Cannot access Vector with len %d at index %d", curr_size, at);
      }

      return addr;
    }

    /*
    T& operator* () {
      return (*this)[0];
    }
    */

    Vector() : Vector(
      sizeof(T) >= VECCAP_DEFAULT ? 1 : (VECCAP_DEFAULT/sizeof(T))
    ) {} // defaults to declaring the capacity with 16 bytes (1 memring cell)

    Vector(unsigned int w_capacity): capacity(w_capacity) {
      this->curr_size = 0;
      this->data = (T *) malloc(w_capacity * sizeof(T));
    }

    ~Vector() {
      free(data);
      data = NULL;
      //delete data;
    }
  };

  /* mini rant here:
     so the nitwits who write the c++ standard basically implemented generics like so:
       - you have a template <typename T>, which basically creates a puppet class that only serves as a template (hence the name).
       - the puppet classes are created into real classes, a separate one for each different T. this is done at compile-time.
     the problem is, well, this is a different source file than where it's used. if there are no uses of the template, then an empty object file gets generated. genius, i know.
     option 1 is to put the methods in the .hh file, (convention is this should be an .inl.hh file).
        linker is actually clever enough to not break one definition rule (believe it or not)
     option 2 is to *manually* declare all the T variations i'd want to use here, so that they actually get generated in the object file.
        this is even more batshit crazy than option 1, so don't even think about it.

     in conclusion, curse you bjarne stroustrup.
   */
}
