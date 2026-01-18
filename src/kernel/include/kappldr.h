#include "text.h"

#pragma once

// loading the KApp, interfacing with the c++

// this is the c++ class. we define it very differently there, so we hide this one
#ifndef __cplusplus
struct k_app; // to avoid cyclic struct resolution

// all the virtual methods. take "this" as first parameter
struct k_app_fntable {
  void (*invoke)(struct k_app *);
  void (*first_run)(struct k_app *);
};

struct k_app {
  // it might be a little unclear what's happening here: but basically, virtual methods in c++ create a jumptable for each child class, so that you can jump to that specific method if you don't know which child class something is. the pointer to that jumptable is _always_ element 1 of the struct, before any other elements. OOP without the object bit is literally this easy.
  // other normal methods are just seperate functions (but name mangled ofc) so we can mangle the names ahead of time and use them here. very jank _here_, but makes writing the apps a heck of a lot easier
  // invoke is ofc a method, so you pass `this' as the first parameter
  struct k_app_fntable *virts; // all the virtual methods
  char key_q[QUEUE_LEN];
  enum ctrl_char ctrl_q[QUEUE_LEN];
  const char *app_name;
  unsigned char app_id;
  unsigned char config_flags;
};
#endif

#define AVAILABLE_KAPPS 8
typedef int app_handle;

app_handle instantiate(struct k_app *, app_handle parent, char is_fg);
void focus_app(app_handle which);
void terminate_app(app_handle which);

extern struct k_app *app_db[AVAILABLE_KAPPS];
extern app_handle curr_kapp;

// returns a shell object in the specified ptr
extern struct k_app *mk_shell(int comlen);
extern void kapp_destroy(struct k_app *);

