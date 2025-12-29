#pragma once

namespace Extra {
  // wrapper for bound instruction.
  // the appeal for using this, is that this throws an error if not in bounds, that is handleable by the error handler
  struct BoundPacket {
    void *first;
    void *last;

    bool contains(void *check);
  } __attribute__((packed));
}
