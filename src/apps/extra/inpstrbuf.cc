// c glue
#include "../include/extra/fromc.hh"

// already declared in an ifdef
void inp_strbuf::delchar_at(size_t at) {
  // NOTE: we want at to be signed, as strlen can return negative values
  if (at < 0 || at >= strlen(this->buf)) return; // if trying to delete out of bounds

  memmove(this->buf + at, this->buf + at + 1, this->len - at - 1);
  if (this->ix > (unsigned) at) { this->ix--; }
}

void inp_strbuf::clear() {
  memzero(this->buf, this->len);
  this->ix = 0;
}

void inp_strbuf::resize_by(int offset) {
  this->buf = (char *) realloc(this->buf, this->len += offset);
}

inp_strbuf::inp_strbuf(size_t with): len(with) {
  buf = (char *) malloc(len);
  this->clear(); // just in case
}

inp_strbuf::~inp_strbuf() {
  free(this->buf);
  this->buf = NULL;
}
