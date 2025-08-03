// c glue
#include "../include/extra/fromc.hh"

// already declared in an ifdef
void inp_strbuf::delchar_at(int at) {
  if (at < 0 || at >= strlen(this->buf)) return; // if trying to delete out of bounds

  memcpy(this->buf + at + 1, this->buf + at, this->len - at - 1);
  if (this->ix > at) { this->ix--; }
}

void inp_strbuf::clear() {
  memzero(this->buf, this->len);
  this->ix = 0;
}

void inp_strbuf::resize_by(int offset) {
  this->buf = realloc(this->buf, this->len += offset);
}

inp_strbuf::inp_strbuf(unsigned int with): len(with) {
  buf = malloc(len);
  this->clear(); // just in case
}

inp_strbuf::~inp_strbuf() {
  free(this->buf);
  this->buf = NULL;
}

