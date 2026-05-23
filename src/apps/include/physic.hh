#pragma once

#include "kapp.hh"
#include "extra/fromc.hh"

typedef unsigned int packet_t;

// number of pieces
#define N_PIECES 2

struct Physic : KApp {
  // virtual methods
  void invoke();
  void first_run();

private:
  unsigned int frame; // frame number

  // linear addresses of current pieces
  curpos_t linear[N_PIECES];

  // packet data
  packet_t pos[N_PIECES];
  packet_t vel[N_PIECES];

  // euclidean norms of velocity
  unsigned short vel_norm[N_PIECES];

  struct char_packet pieces[N_PIECES];

  /* runtime variables */

  // x and y axis acceleration respectively
  short tilt;
  short gravity;

  bool paused; // true if paused

  void tick_and_draw();
  void title();

  // helper function to paint piece
  static struct char_packet paint_piece(struct char_packet piece, unsigned char bg);

  /* external asm functions. static as they don't reference this pointer, so don't need to be passed it */
  static void init(unsigned int ticks_per_frame, short gravity, short tilt);
  static void compute_pair(packet_t *pos_packet, packet_t *vel_packet);
  static curpos_t get_linear(packet_t pos, bool is_squashed);
  static void euclidean_norm(const packet_t *vel_packet, unsigned short *norms);
  static void deinit();

public:
  Physic();
  ~Physic();
};
