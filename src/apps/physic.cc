#include "include/physic.hh"

#include "include/extra/fromc.hh"

/*
extern void __physic_init(unsigned int ticks_per_frame, short gravity, short tilt);
extern void __physic_compute_pair(packet_t *pos_packet, packet_t *vel_packet);
extern curpos_t __physic_get_linear(packet_t pos);
extern void __physic_euclidean_norm(packet_t *vel_packet, unsigned short *norms);
extern void __physic_deinit();
*/

// all numbers are in 9.7 fixed point.
// the integer part is supposed to be hectometres, which means the arena is 51.2x51.2km wide, but also making one single unit about 78cm.
// this is quite ridiculous, but we need to zoom out as otherwise the gravity is too fast to appreciate.
// this is also somewhat distorted into a 720x400 text mode screen, but it still doesn't look too strange.

// the smallest unit of time that can be represented in 9.7fix is 1/128sec, which i shall call 1 tick.
// 4 ticks per frame is the best tradeoff between least choppy and closest to an integer number of centiseconds (i.e. 3.125cs \approx 3cs)

/*
  for posterity, the list of reasonable errors:
    1 => 0.0021875
    4 => 0.0012499999999999994
    5 => 0.0009375000000000008
    9 => 0.00031249999999999854
    32 => 5.204170427930421e-18

  and the julia code for generating them:
    all = 1:128 .|> x -> x => min(x/128 % 0.01, 0.01 - (x/128 % 0.01))
    max = 0 => 1
    for pair in all
      if last(pair) < last(max)
          max = pair
          println(max)
      end
    end
*/

// 4/128sec in 9.7 fixed point. see above
#define TICKS_PER_FRAME 4
#define CS_PER_FRAME 3

// approximation of -0.0981 hm/s^2 in 9.7 fixed point.
// -0.0981hm/s^2 * 128 = 12.552515...
#define GRAVITY -12

void Physic::invoke() {
  for (int i = 0; i < QUEUE_LEN && !!this->ctrl_q[i]; ++i) { // loop over each arrow key
    switch (this -> ctrl_q[i]) {
      default: break;
    }

    ctrl_q[i] = NO_CTRL;
  }

  if (this -> key_q[0]) { // press any key to continue...
    terminate_app(this->app_id & 0xf);
  }

  this->tick_and_draw();
}

void Physic::first_run() { this->tick_and_draw(); }

void Physic::tick_and_draw() {
  // do nothing if paused
  if (paused) return;

  this->compute_pair(this->pos, this->vel);

  // over each piece
  for (int piece = 0; piece < N_PIECES; ++piece) {
    // wipe old location. style = 0.
    write_cell(' ', linear[piece], 0);

    // get new linear location
    linear[piece] = this->get_linear(pos[piece]);

    // write piece
    write_cell_packet(linear[piece], pieces[piece], false);
  }
}

Physic::Physic() {
  // bitflags, or as many together as need be
  config_flags = 0; // we frankly don't care about enter, so its flag is just your favourite bit

  app_name = "physic";

  // setup
  {
    pos[0] = 0xc000'7fff; // (20, 0)
    vel[0] = 0x0080'0000; // [0.15625, 0]

    pos[1] = 0x4000'5fff; // (60, 10)
    vel[1] = 0xff00'f000; // [-0.3125, -1.5625]

    // yellow hash and pick asterisk
    pieces[0] = { '#', COLOUR(BLACK, B_YELLOW) };
    pieces[1] = { '*', COLOUR(BLACK, B_MAGENTA) };

    // set accelerations
    tilt = 0;
    gravity = GRAVITY;

    paused = false;
  }

  //__physic_init(TICKS_PER_FRAME, gravity, tilt); // init data
  this->init(TICKS_PER_FRAME, gravity, tilt); // init data
}

Physic::~Physic() {
  this->deinit(); // deinit
}
