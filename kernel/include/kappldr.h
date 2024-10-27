#include "text.h"

#pragma once

// loading the KApp, interfacing with the c++

struct k_app {
  // it might be a litle unclear what's happening here: but basically, virtual methods in c++ create a jumptable for each child class, so that you can jump to that specific method if you don't know which child class something is. the pointer to that jumptable is _always_ element 1 of the struct, before any other elements. OOP without the object bit is literally this easy.
  // other normal methods are just seperate functions (but name mangled ofc) so we can mangle the names ahead of time and use them here. very jank _here_, but makes writing the apps a heck of a lot easier
  // invoke is ofc a method, so you pass `this' as the first parameter
  void (**invoke)(struct k_app *);
  char key_q[QUEUE_LEN];
  enum ctrl_char ctrl_q[QUEUE_LEN];
};

#define AVAILABLE_KAPPS 8
typedef int app_handle;

extern struct k_app *app_db[AVAILABLE_KAPPS];
extern app_handle curr_kapp;

// returns a shell object in the specified ptr
extern struct k_app *mk_shell(int comlen);

