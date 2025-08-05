#include "include/helphost.hh"
#include "include/extra/fromc.hh"

// the amount of entries per page
#define ENTRY_COUNT (VP_HEIGHT - 4)

void HelpHost::invoke() {
  // no real input, tui is set up in first_run()

  for (int i = 0; i < QUEUE_LEN && !!this->ctrl_q[i]; ++i) { // loop over each arrow key
    switch (this -> ctrl_q[i]) {
      case NO_CTRL: break; // already dealt with, should never happen
      case UP:
      case PGUP: {
        if (this->start_ix < ENTRY_COUNT) this->start_ix = 0;
        else this->start_ix -= ENTRY_COUNT;

        this->put_entries();
        break;
      }
      case DOWN:
      case PGDOWN: {
        unsigned int total = this->start_ix; // find how many entries. starting from start_ix for efficiency, as we know that it's less than the last
        for (; this->ents[total].type != -1; ++total);
        
        if ((this->start_ix + 2*ENTRY_COUNT) > total) this->start_ix = total - ENTRY_COUNT;
        else this->start_ix += ENTRY_COUNT;

        this->put_entries();
        break;
      }
      default: break;
    }

    ctrl_q[i] = 0;
  }

  if (this -> key_q[0]) { // press any key to continue...
    terminate_app(this->app_id & 0xf);
  }
}

void HelpHost::first_run() {
  clear_scr();
  set_cur(0);
  //cur_off();

  this->start_ix = 0;
  this->put_entries();
}

void HelpHost::put_entries() {
  clear_scr();
  // titlebar
  write_cell(0xd5, POS(0, 0), COLOUR(BLUE, B_BLACK)); // corner
  write_cell(0xb8, POS(-1, 1), COLOUR(BLUE, B_BLACK)); // box
  for (int p = 1; p < (VGA_WIDTH - 1); ++p) write_cell(0xcd, POS(p, 0), COLOUR(BLUE, B_BLACK)); // box

  set_cur(POS(1, 0));
  write_str(" ? ", COLOUR(BLUE, B_RED));
  write_str(this->name, COLOUR(BLUE, B_WHITE));
  write_cell_cur(' ', COLOUR(BLUE, 0));

  if (start_ix != 0) { // if can scroll up
    // ^ [MORE] ^ 10
    write_str_at("\030 [MORE] \030", POS((VGA_WIDTH-10)/2, 0), COLOUR(BLUE, B_CYAN));
  }

  line_feed();

  // calculates which column to start the desc - loops through all of them to avoid alignement issue on scroll
  unsigned int total = 0;
  short furthest = 0;
  for (; this->ents[total].type != -1; ++total) {
    int hh = strlen(this->ents[total].name);
    if (furthest < hh) furthest = hh;
  }
  // total is len(this->ents)

  // for each entry (only show a set few
  for (unsigned int ent = start_ix; ent < (start_ix + ENTRY_COUNT) && this->ents[ent].type != -1; ++ent) {
    unsigned char brightness = this->ents[ent].type & DEBUG_ENTRY ? BLACK : B_BLACK;
    write_cell_cur(0xb3, COLOUR(BLUE, B_BLACK)); // left box edge
    adv_cur();

    // sub entry (indent)
    if (this->ents[ent].type & SUB_ENTRY) {
      adv_cur_by(2);
      write_cell_cur(':', COLOUR(BLUE, GREEN));
      write_str(this->ents[ent].name, COLOUR(BLUE, GREEN) | brightness);
    } else {
      write_str(this->ents[ent].name, COLOUR(BLUE, YELLOW) | brightness);
    }

    int cix = 0; // index of optional (if at all). i.e., end of main desc
    for (; this->ents[ent].desc[cix] && (this->ents[ent].desc[cix] != '\xff'); ++cix);
    if (this->ents[ent].desc[cix] != '\xff') { // we can't edit the string to contain a null, because it would break it for future help runs.
                                                // don't even think about fixing it afterwards. c++ "can't edit constants" or smth
      cix = -1;
    }

    set_cur(POS(furthest + (this->ents[ent].type & SUB_ENTRY) << 1, (ent - start_ix) + 1));
    write_str("- ", COLOUR(BLUE, WHITE));

    if (cix != -1) {
      // budget write_str (until either null or \xff, because optionals come after \xff)
      for (int q = 0; q < cix; ++q) {
        write_cell_cur(this->ents[ent].desc[q], COLOUR(BLUE, B_WHITE));
      }

      // types of optionals
      if (this->ents[ent].desc[cix + 1] == '[') { // alternatives, take magenta
        adv_cur();
        write_str(this->ents[ent].desc + cix + 1, COLOUR(BLUE, MAGENTA));
      } else if (this->ents[ent].desc[cix+1] == '<') { // args, take bright red
        set_cur(POS(VGA_WIDTH - 8 - strlen(this->ents[ent].desc + cix + 1), (ent - start_ix) + 1));
        write_str("args: ", COLOUR(BLUE, RED));
        write_str(this->ents[ent].desc + cix + 1, COLOUR(BLUE, B_RED));
      }
    } else { // sanity, at long last
      write_str(this->ents[ent].desc, COLOUR(BLUE, B_WHITE));
    }

    // bottom right
    write_cell(0xb3, POS(-1, ln_nr()+1), COLOUR(BLUE, B_BLACK));
    paint_row(BLUE);
    line_feed();
  }

  while (ln_nr() < (ENTRY_COUNT + 1)) {
    write_cell_cur(0xb3, COLOUR(BLUE, B_BLACK));
    paint_row(BLUE);
    write_cell(0xb3, POS(-1, ln_nr()+1), COLOUR(BLUE, B_BLACK));
    line_feed();
  }

  write_cell(0xd4, POS(0, ENTRY_COUNT + 1), COLOUR(BLUE, B_BLACK)); // corner
  write_cell(0xbe, POS(-1, ENTRY_COUNT + 2), COLOUR(BLUE, B_BLACK)); // box
  for (int p = 1; p < (VGA_WIDTH - 1); ++p) write_cell(0xcd, POS(p, ENTRY_COUNT + 1), COLOUR(BLUE, B_BLACK)); // box

  if (start_ix + ENTRY_COUNT < total) { // if can scroll down
    // v [MORE] v 10
    write_str_at("\031 [MORE] \031", POS((VGA_WIDTH-10)/2, ENTRY_COUNT + 1), COLOUR(BLUE, B_CYAN));
  }


  set_cur(POS(0, VP_HEIGHT - 1));
  write_str("\020 any non-control key to continue...  ", COLOUR(BLACK, RED));
  adv_cur_by(-1);
}

HelpHost::HelpHost(char* what, HelpHost::Entry* loads): ents(loads), name(what) {
  // bitflags, or as many together as need be
  config_flags = 0; // enter can exit too alright, so we dont want it as a CTRL code

  app_name = "helphost";
}

HelpHost::~HelpHost() {
  // cur_on();
}
