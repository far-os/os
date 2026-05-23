#include "include/physic.hh"

#include "include/extra/fromc.hh"
#include "include/helphost.hh"

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

const HelpHost::Entry help_data[] = {
  { .name = "<Esc>", .desc = "Quit", .type = HelpHost::PLAIN_ENTRY },
  { .name = "^H", .desc = "Show this help menu", .type = HelpHost::PLAIN_ENTRY },
  { .name = "^W", .desc = "Reset screen (if some pieces stuck)", .type = HelpHost::PLAIN_ENTRY },
  { .type = HelpHost::DIVIDER },
  { .name = "[HJKL]", .desc = "Control gravity", .type = HelpHost::PLAIN_ENTRY },
  { .name = "J/K", .desc = "Set gravity to point down/up", .type = HelpHost::SUB_ENTRY },
  { .name = "H/L", .desc = "Increase left/right tilt by one", .type = HelpHost::SUB_ENTRY },
  { .name = "Space", .desc = "Play/pause", .type = HelpHost::PLAIN_ENTRY },
  { .type = HelpHost::DIVIDER },
  { .desc = "Topbar shows co-ords and speed (in 65536ths of arena, i.e. \xeb) of each piece,", .type = HelpHost::SYNOPSIS },
  { .desc = "and then shows the current tilt factor, then whether gravity is up or down.", .type = HelpHost::SYNOPSIS },
  { .desc = "Tilt can be changed and gravity can be flipped with HJKL, as above.", .type = HelpHost::SYNOPSIS },
  { .desc = "A flashing red \xfe indicates paused, and the frame number is in the top left.", .type = HelpHost::SYNOPSIS },
  { .type = HelpHost::TERMINATE }
};

// 4/128sec in 9.7 fixed point. see above
#define TICKS_PER_FRAME 4
#define CENTISEC_PER_FRAME 3

// approximation of -0.0981 hm/s^2 in 9.7 fixed point.
// -0.0981hm/s^2 * 128 = 12.552515...
#define GRAVITY -12

void Physic::invoke() {
  for (int i = 0; i < QUEUE_LEN && !!this->key_q[i]; ++i) { // loop over each arrow key
    switch (this -> key_q[i]) {
    case 'H': // tilt left
    case 'h':
      this->tilt--;
      goto reset;
    case 'J': // set gravity down
    case 'j':
      if (this->gravity > 0) gravity = -gravity;
      goto reset;
    case 'K': // set gravity up
    case 'k':
      if (this->gravity < 0) gravity = -gravity;
      goto reset;
    case 'L': // tilt right
    case 'l':
      this->tilt++;
      goto reset;
    case ' ': // space to pause
      this->paused = !paused;
      this->title(); // update pause indicator
      break;
    default:
      break;
    reset: // call init again with new settings
      this->init(TICKS_PER_FRAME, gravity, tilt);
      this->title(); // fix title
    }

    key_q[i] = NO_CTRL;
  }

  bool drawing = true;

  for (int i = 0; i < QUEUE_LEN && !!this->ctrl_q[i]; ++i) { // loop over each arrow key
    switch (this -> ctrl_q[i]) {
      case NO_CTRL: break; // already dealt with, should never happen
      case CTRL(H): {
        app_handle help = instantiate(
          new HelpHost("physics simulator", help_data),
          this->app_id & 0xf,
          true
        );
        drawing = false;
        break;
      }
      case CTRL(W): {
        clear_scr(); // clear screen
        break;
      }
      case ESC: {
        terminate_app(this->app_id & 0xf);
        return;
      }
      default: break; // nothing else
    }

    ctrl_q[i] = NO_CTRL;
  }

  if (drawing) this->tick_and_draw();

  next_step:
  this->invoke_after_centisecs(CENTISEC_PER_FRAME);
}

void Physic::first_run() {
  this->invoke(); // trigger tick and loop
}

void Physic::tick_and_draw() {
  // do nothing if paused
  if (paused) return;

  this->frame++;

  this->compute_pair(this->pos, this->vel);

  // every other frame, calculate velocity
  if (this->frame & 1) {
    this->euclidean_norm(this->vel, this->vel_norm);
  }

  // over each piece
  for (int piece = 0; piece < N_PIECES; ++piece) {
    // wipe old location. style = 0.
    write_cell(' ', linear[piece] + VGA_WIDTH, 0);

    // get new linear location
    linear[piece] = this->get_linear(pos[piece], (VP_HEIGHT < VGA_HEIGHT));

    // write piece
    write_cell_packet(linear[piece] + VGA_WIDTH, pieces[piece], false);
  }

  this->title();
}

// helper function to paint piece
struct char_packet Physic::paint_piece(struct char_packet piece, unsigned char bg) {
  struct char_packet mod = piece;
  mod.style &= 0xf;
  mod.style |= bg << 4;
  return mod;
}

void Physic::title() {
  set_cur(POS(0, 0));
  printf("\r%$ %u %$\xb3%$ %~: (%2d,%2d) %u \xeb/s \xb3 %~: (%2d,%2d) %u \xeb/s %$\xb3 T: %~ %u, G: %~ \xb3 %$^H %$= help",
    COLOUR(BLUE, B_RED),
    frame,
    COLOUR(BLUE, WHITE),
    COLOUR(BLUE, B_WHITE),
    this->paint_piece(pieces[0], BLUE),
    linear[0] % 80,
    linear[0] / 80,
    vel_norm[0],
    this->paint_piece(pieces[1], BLUE),
    linear[1] % 80,
    linear[1] / 80,
    vel_norm[1],
    COLOUR(BLUE, WHITE),
    (struct char_packet) { tilt > 0 ? '\x1a' : tilt == 0 ? '\xf9' : '\x1b', COLOUR(BLUE, B_CYAN) },
    tilt > 0 ? tilt : -tilt,
    (struct char_packet) { gravity > 0 ? '\x18' : gravity == 0 ? '\xf9' : '\x19', COLOUR(BLUE, B_GREEN)},
    COLOUR(BLUE, B_CYAN),
    COLOUR(BLUE, CYAN)
  );

  for (curpos_t cur = get_cur(); cur < VGA_WIDTH; ++cur) {
    write_cell(0, cur, COLOUR(BLUE, BLUE)); // empty rest of line
  }

  // flashing red square when paused, otherwise blank (doesn't work too well on qemu)
  write_cell(paused ? '\xfe' : '\xff', POS(VGA_WIDTH - 2, 0), COLOUR(B_BLUE, B_RED));
}

Physic::Physic() {
  // bitflags, or as many together as need be
  config_flags = 0; // we frankly don't care about enter, so its flag is just your favourite bit

  app_name = "physic";

  // setup
  {
    frame = 0;

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
