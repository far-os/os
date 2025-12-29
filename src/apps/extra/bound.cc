#include "../include/extra/bound.hh"
#include "../include/extra/fromc.hh"

namespace Extra {
  // TODO: once is moved to stdlib, add maybe an assert function that just tells the interrupt handler to use the normal error handler.
  bool BoundPacket::contains(void *check) {
    // by default, raising an exception (like bound's #BR) will return back to the exact same address,
    // in the hopes of the interrupt handler being able to reconcile the issue at hand, and then re-running the instruction.
    // here, we have to tell the interrupt handler that no, in the event of a failure, we want to jump to a certain address, which we put in %ebx.
    // this is because we are in KERNEL MODE, and we don't have a real error handler in kernel mode.
    asm goto volatile (
      "leal %l2, %%ebx\nboundl %0, %1"
      :
      : "r" (check), "m" (*this)
      : "ebx"
      : bd_after
    );

    return true; // got here without issue

    // TODO: because it was very hacky (and i didn't want to break PROG.BIN), this only works in the interrupt handler in kernel ring (q.v.).
    // i was also under the impression that a real error handler would have some way of dealing with this, but this is patently not true,
    // as this is a statement that acts as an assertion, but we are here using it as a checker.
    //
    // for a program, we would probably want this to assert. XXX, see vector.inl.hh for more
  bd_after:
    return false;
  }
}
