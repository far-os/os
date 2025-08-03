// Kernel Applets
// OOP works a bit better for this, so we are using c++
// don't worry, c++ is surprisingly easy to shoehorn into existing c code

// import kernel stuff
#include "include/extra/fromc.hh"

// all the app files
#include "include/kapp.hh"
#include "include/helphost.hh"
#include "include/edit.hh"
#include "include/shell.hh"
#include "include/calc.hh"

KApp::KApp() {
  memzero(&key_q, QUEUE_LEN);
  memzero(&ctrl_q, QUEUE_LEN);
}

// it's a very common thing to need to do, write input keys to buffer, so we make a function to do it avoid repetition
// returns the final length of the buffer, -1 if its too big (this will help us in reallocking)
int KApp::write_keys_to_buf(struct inp_strbuf *to) {
  // if we'll be past the end of the buffer's allocation (comlen)
  // it'll error anyway, no point in putting characters
  char n_to_put = strlen(this->key_q);
  if (!n_to_put) return; // nothing to do beyond this point

  int targ_len = strlen(to->buf) + n_to_put;
  if (targ_len >= to->len) {
    msg(PROGERR, E_BUFOVERFLOW, "Buffer size exceeded");
    return -1;
  };

  backmemcpy(
    to->buf + to->len - (n_to_put + 1), // end of src
    to->buf + to->len - 1, // end of dest
    to->len - (to->ix + n_to_put) // length of memory to copy
  );
  strcpy(this->key_q, to->buf + to->ix);
  memzero(this->key_q, QUEUE_LEN);
  to->ix++;

  return targ_len;
}

// we dont technically need to destruct anything, but out children might (but if we don't know they're a child we need virtuality)
KApp::~KApp() {};

// c-friendly wrapper
extern "C" {
  // there are some private properties, that c's not supposed to ever know about (so it thinks it's smaller)
  KApp *mk_shell(int comlen) {
    return new KShell (comlen);
  };

  void kapp_destroy(KApp *a) {
    delete a;
  }
};
