#include "include/calc.hh"
#include "include/extra/stack.hh"

void Calc::first_run() {
  write_str("Reverse Polish Calculator\n", COLOUR(BLACK, B_GREEN));

  // print out prompt
  write_str(this->get_prompt(), COLOUR(BLACK, WHITE));
}

// may want it to be dynamic (i.e. flashes).
// XXX: in KShell::invoke() this is always printed in format 0x07, so flashes will basically tick until the next invoke call.
const char * Calc::get_prompt() {
  return "\r#> ";
}

// rhs and lhs are auto here because if i change the type i dont want to have to pass a T in
#define SIMPLE_OPERATOR(OP) if (this->work.buf[from] == #OP[0] && this->stack.len() >= 2) {\
                              auto rhs = this->stack.pop();\
                              auto lhs = this->stack.pop();\
                              this->stack.push(lhs OP rhs);\
                              if (untouched_top >= this->stack.len()) {\
                                untouched_top = this->stack.len() - 1;\
                              }\
                              ++to;\
                              justop = true;\
                            }

bool Calc::shexec() { 
  unsigned int untouched_top = this->stack.len();

  if (strcmp(this->work.buf, "exit")) {
    terminate_app(this->app_id & 0xf);
    return false;
  }

  bool justop = false; // was last number an operator
  unsigned int from = 0;

  // for each number in work.
  while (this->work.buf[from]) {
    unsigned int to = from;
    justop = false;

    // modern technology
    SIMPLE_OPERATOR(+) else
    SIMPLE_OPERATOR(-) else
    SIMPLE_OPERATOR(*) else
    SIMPLE_OPERATOR(/) else
    SIMPLE_OPERATOR(%) else
    SIMPLE_OPERATOR(|) else
    SIMPLE_OPERATOR(&) else
    SIMPLE_OPERATOR(^) else
      for (; ((unsigned char) (this->work.buf[to] - 0x30)) < 10; ++to);

    char old = this->work.buf[to];
    if (old != '\0' && old != ' ') break;

    // diy slice (BAD)
    this->work.buf[to] = 0;
    if (!justop) this->stack.push(to_uint(this->work.buf + from));
    this->work.buf[to] = old;

    from = to + 1;
  }

  char *intbuf = malloc(16); // integer buffer
  for (unsigned int i = 0; i < this->stack.len(); i++) {
    memzero(intbuf, 16);
    sprintf(intbuf, "%d ", this->stack[i]); 
    write_str(intbuf, COLOUR(
      BLACK,
      (i < untouched_top ? B_CYAN : B_RED)
    ));
  }
  free(intbuf);

  line_feed();
  write_str(this->get_prompt(), COLOUR(BLACK, WHITE));
  return true;
}

/* TODO:
  modified becomes red
  p - pop (1 in, 0 out)
  c - clear (infinite in, 0 out)
  n - negate (popsave)

  [ABCD] - save into register
*/

Calc::Calc() : KShell(80) {
  app_name = "calc";
}
