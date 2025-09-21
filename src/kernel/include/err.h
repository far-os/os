#pragma once

// error message shit
enum MSG_TYPE {
  INFO = 0, // ->
  WARN = 1, // +
  PROGERR = 2, // !
  KERNERR = 3, // !!
  PANIC = 4, // x
};

enum ERRSIG {
  NONE = 0,
  E_PROG = 1,
  E_EXTERN = 2,
  E_NOFILE = 3,
  E_NOSTORAGE = 4,
  E_BADADDR = 5,
  E_ARGS = 6,
  E_BOUND = 7,
  E_ILLEGAL = 9,
  E_SUICIDE = 10,
  E_UNKENTITY = 11,
  E_TIME = 12,
  E_BINLOAD = 15,
  E_CONFIG = 18,
  E_BUFOVERFLOW = 23,
  E_HANDLEALLOC = 28
};

extern char msg_symbs[5];

void msg(enum MSG_TYPE type, enum ERRSIG sig, char* supp);
