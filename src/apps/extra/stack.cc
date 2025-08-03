#include "../include/extra/fromc.hh"
#include "../include/extra/stack.hh"

namespace Extra {
  template <typename T>
  void Vector<T>::grow() { // grow
    this->data = realloc(this->data, (this->capacity *= 2) * sizeof(T));
  }

  template <typename T>
  unsigned int Vector<T>::len() { return curr_size; }
  
  template <typename T>
  void Vector<T>::push(T itm) {
    if (curr_size >= capacity) this->grow(); // if growing will kaboom
    this->data[this->curr_size++] = itm;
  }

  template <typename T>
  T Vector<T>::pop() {
    if (!this->curr_size) {
      msg(WARN, E_BUFOVERFLOW, "Cannot pop empty Vector, ignoring");
      return 0;
    }
    T keep = this->data[this->curr_size - 1];
    this->data[this->curr_size - 1] = 0;
    this->curr_size--;
    return keep;
  }

  template <typename T>
  T& Vector<T>::operator[] (int at) {
    if (at >= this->curr_size) {
      msg(PROGERR, E_BUFOVERFLOW, "Attempted to access past end of Vector");
    }
    return this->data[at];
  }

  /*
  T& operator* () {
    return (*this)[0];
  }
  */

  template <typename T>
  Vector<T>::Vector() : Vector(
    sizeof(T) >= VECCAP_DEFAULT ? 1 : (VECCAP_DEFAULT/sizeof(T))
  ) {} // defaults to declaring the capacity with 16 bytes (1 memring cell)

  template <typename T>
  Vector<T>::Vector(unsigned int w_capacity): capacity(w_capacity) {
    this->curr_size = 0;
    this->data = malloc(w_capacity * sizeof(T));
  }

  template <typename T>
  Vector<T>::~Vector() {
    delete data;
  }

  /* mini rant here:
     so the nitwits who write the c++ standard basically implemented generics like so:
       - you have a template <typename T>, which basically creates a puppet class that only serves as a template (hence the name).
       - the puppet classes are created into real classes, a separate one for each different T. this is done at compile-time.
     the problem is, well, this is a different source file than where it's used. if there are no uses of the template, then an empty object file gets generated. genius, i know.
     option 1 is to put the methods in the .hh file, (macro$haft convention says this should be a .tpp file, as opposed to be a .hpp file, but just no).
        this is bad, because use the same T twice in different files, you'll have all the methods twice and linker shits itself.
     option 2 is this, to *manually* declare all the T variations i'm gonna use here, so that they actually get generated in the object file.

     in conclusion, curse you bjarne stroustrup.
   */
  template class Vector<int>;
}

