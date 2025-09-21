#pragma once

#define VECCAP_DEFAULT 16
namespace Extra {
  // shitty implementation of resizable Vec
  template <typename T>
  class Vector {
  protected:
    T* data;
    unsigned int capacity; // the max size, what is malloc'd basically
    unsigned int curr_size; // the reported size

    void grow(); // fix too big to fit in capacity

  public:
    unsigned int len(); // getter for curr_size

    void push(T itm);
    T pop();

    T& operator[](int);

    Vector();
    Vector(unsigned int w_capacity);
    ~Vector();
  };
}
