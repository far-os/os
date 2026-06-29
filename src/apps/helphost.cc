#include "include/helphost.hh"
#include "include/extra/fromc.hh"

// the amount of entries per page
#define ENTRY_COUNT (VP_HEIGHT - 4)
#define SUB_ENTRY_INDENT 2

void HelpHost::invoke() {
  // no real input, tui is set up in first_run()

  unsigned n_entries = this->tally_entries(); // keep the entry count
  bool to_redraw = false; // boolean. make redrawing more concise

  for (int i = 0; i < QUEUE_LEN && !!this->ctrl_q[i]; ++i) { // loop over each arrow key
    switch (this -> ctrl_q[i]) {
      case NO_CTRL: break; // already dealt with, should never happen
      case UP: { // up by 1
        // if is more than 0, i.e. not alrady at top of screen
        if (this->top_entry > 0) {
          this->top_entry--;
          to_redraw = true;
        }
        break;
      }
      case PGUP: { // up by page
        if (this->top_entry > 0) { // prevent scrolling too far up, see above
          if (this->top_entry < ENTRY_COUNT) this->top_entry = 0; // go to top
          else this->top_entry -= ENTRY_COUNT; // move up entire page

          to_redraw = true;
        }
        break;
      }
      case DOWN: {
        // maximum possible top_entry before overflow, i.e. where the last page must begin.
        // if we can afford to, this is n_entries - ENTRY_COUNT, otherwise just 0
        unsigned max_top = (n_entries > ENTRY_COUNT) ? (n_entries - ENTRY_COUNT) : 0;

        // if we can, move forward one
        if (this->top_entry < max_top) {
          this->top_entry++; // move forward a page
          to_redraw = true;
        }

        break;
      }
      case PGDOWN: {
        // maximum possible top_entry before overflow, i.e. where the last page must begin.
        // if we can afford to, this is n_entries - ENTRY_COUNT, otherwise just 0
        unsigned max_top = (n_entries > ENTRY_COUNT) ? (n_entries - ENTRY_COUNT) : 0;

        // if we can, move forward a page
        if (this->top_entry < max_top) {
          this->top_entry += ENTRY_COUNT; // move forward a page
          to_redraw = true;
        }

        // have we gone too far? rubber band back to the maximum we are allowed
        // also runs if it's too far in the first place (unlikely, but still)
        if (this->top_entry > max_top) {
          this->top_entry = max_top;
          to_redraw = true;
        }

        break;
      }
      default: break;
    }

    ctrl_q[i] = NO_CTRL;
  }

  // if we want to redraw, then do
  if (to_redraw) this->put_entries();

  if (this -> key_q[0]) { // press any non-control key to continue...
    // kills itself if there is anything at all in the non-control buffer
    terminate_app(this->app_id & 0xf);
  }
}

void HelpHost::first_run() {
  clear_scr();
  set_cur(0);
  //cur_off();

  this->top_entry = 0;
  this->put_entries();
}

#define TOP_EDGE false
#define BOTTOM_EDGE true

void HelpHost::put_entries() const {
  clear_scr();

  // titlebar. edges are as so: double on top
  set_cur(POS(0, 0));
  this->draw_edge(TOP_EDGE);

  // calculates which column to start the description - loops through all of them to avoid alignment issue on scroll
  unsigned char desc_indent = 0;
  for (unsigned total = 0; this->ents[total].type != -1; ++total) {
    unsigned int name_len = strlen(this->ents[total].name);
    if (desc_indent < name_len) desc_indent = name_len;
  }

  // for each entry (only show a set few, from this->top_entry onwards showing at most ENTRY_COUNT)
  for (unsigned int entry_n = top_entry; entry_n < (top_entry + ENTRY_COUNT) && this->ents[entry_n].type != -1; ++entry_n) {
    put_single_entry(this->ents[entry_n], (entry_n - this->top_entry) + 1, desc_indent);
  }

  // draw rest of box (if we haven't filled up the window yet)
  while (ln_nr() < (ENTRY_COUNT + 1)) {
    write_cell_cur('\263', COLOUR(BLUE, B_BLACK));
    paint_row(BLUE);
    write_cell('\263', POS(-1, ln_nr()+1), COLOUR(BLUE, B_BLACK));
    line_feed();
  }

  this->draw_edge(BOTTOM_EDGE);

  set_cur(POS(0, VP_HEIGHT - 1));
  // an extra space plus a backspace at the end to ensure cursor shows as red
  write_str("\020 any non-control key to continue...  \b", COLOUR(BLACK, RED));
}

// can only be one edge
void HelpHost::draw_edge(bool edge) const {
  if (edge == TOP_EDGE) {
    // will cut off at 79, i.e. one before end, to make room for corner cell at end
    nprintf(VGA_WIDTH - 1, "%$\325%$ ? %$%s %$%*r",
      COLOUR(BLUE, B_BLACK), // border colour
      COLOUR(BLUE, B_RED),   // question mark colour
      COLOUR(BLUE, B_WHITE), // name colour
      this->name,
      COLOUR(BLUE, B_BLACK), // border colour
      VGA_WIDTH, // maximum number of times to repeat border
      '\315' // top edge
    );
    write_cell_cur('\270', COLOUR(BLUE, B_BLACK)); // top-right corner
    // will overflow with cursor

    if (this->top_entry != 0) { // if can scroll up, write over with MORE
      // ^ [MORE] ^ 10
      write_str_at("\030 [MORE] \030", POS((VGA_WIDTH - 10) / 2, 0), COLOUR(BLUE, B_CYAN));
    }
  } else if (edge == BOTTOM_EDGE) {
    // see above.
    // will cut off at 79
    nprintf(VGA_WIDTH - 1, "%$\324%*r",
      COLOUR(BLUE, B_BLACK), // border colour
      VGA_WIDTH, // maximum number of times to repeat border
      '\315' // bottom edge
    );
    write_cell_cur('\276', COLOUR(BLUE, B_BLACK)); // bottom-right corner

    if ((this->top_entry + ENTRY_COUNT) < this->tally_entries()) { // if can scroll down
      // v [MORE] v 10
      write_str_at("\031 [MORE] \031", POS((VGA_WIDTH - 10) / 2, ENTRY_COUNT + 1), COLOUR(BLUE, B_CYAN));
    }
  }
};

// thunk to get the number of entries
unsigned int HelpHost::tally_entries() const {
  unsigned count = 0;
  for (; this->ents[count].type != -1; ++count);
  return count;
}

void HelpHost::put_single_entry(const Entry& ent, unsigned int row_num, unsigned int desc_indent) const {
  // sets whether to be bold or not. is ored with final colour
  unsigned char brightness = (ent.type & DEBUG_ENTRY) ? BLACK : B_BLACK;

  // left border
  write_cell_cur('\263', COLOUR(BLUE, B_BLACK));
  adv_cur();

  if (ent.type & DIVIDER) {
    // draw divider
    if (ent.type & SUB_ENTRY) {
      adv_cur_by(SUB_ENTRY_INDENT);
    }

    for (int i = get_cur() % VGA_WIDTH; i < (VGA_WIDTH - 2); ++i) {
      // force cast because c++ is stupid
      write_cell_cur('\304', (unsigned char) (COLOUR(BLUE, BLACK) | brightness));
    }
  } else if (ent.type & SYNOPSIS) {
    // simple synopsis
    if (ent.desc) { // check that description is non-null
      unsigned int len = strlen(ent.desc);
      if (len > (VGA_WIDTH - 4)) { // if over the width allowed (i.e. borders AND padding)
        // minus 7, not just 4, to make room for ellipsis
        nprintf(VGA_WIDTH - 7, "%$%s", COLOUR(BLUE, WHITE) | brightness, ent.desc);
        write_str("...", COLOUR(BLUE, WHITE));
      } else {
        write_str(ent.desc, COLOUR(BLUE, WHITE) | brightness);
      }
    }
  } else { // otherwise...
    // sub-entry indents name by 2 and adds colon, and writes name in green not yellow
    if (ent.type & SUB_ENTRY) {
      adv_cur_by(SUB_ENTRY_INDENT);
      write_cell_cur(':', COLOUR(BLUE, GREEN));
      write_str(ent.name, COLOUR(BLUE, GREEN) | brightness);
    } else {
      write_str(ent.name, COLOUR(BLUE, YELLOW) | brightness);
    }

    // uses optional suffix, delimited by '\xff' at end of description.
    int opt_ix = -1;
    if (ent.desc) { // check description is non-nullish
      for (int i = 0; ent.desc[i]; ++i) {
        if (ent.desc[i] == '\xff') {
          opt_ix = i;
          break;
        }
      }
    }

    // calculate column it's supposed to start at. supposed to be consistent across all of them
    unsigned int desc_col = desc_indent + ( // SUB_ENTRY_INDENT +1 to include colon
      (ent.type & SUB_ENTRY) ? SUB_ENTRY_INDENT + 1 : 0
    ) + 3; // +3 because len(" - ") == 3

    set_cur(POS(desc_col, row_num)); // jump forward to description
    write_str("- ", COLOUR(BLUE, WHITE)); // dash delimiter

    if (opt_ix != -1) { // if we have an optional suffix?
      // write what we're supposed to up to the '\xff' delimiter, up to a specific amount
      nprintf(opt_ix, "%$%s", COLOUR(BLUE, WHITE) | brightness, ent.desc);

      // process optional suffix
      const char* suffix = ent.desc + opt_ix + 1;
      switch (suffix[0]) {
      case '[': // alternative
        adv_cur();
        write_str(suffix, COLOUR(BLUE, MAGENTA));
        break;
      case '<': // arguments
        set_cur(POS(VGA_WIDTH - 8 - strlen(suffix), row_num));
        write_str("args: ", COLOUR(BLUE, RED));
        write_str(suffix, COLOUR(BLUE, B_RED)); // unaffected by BRIGHTNESS
        break;
      case '-': // flags/options
        set_cur(POS(VGA_WIDTH - 11 - strlen(suffix), row_num));
        write_str("options: ", COLOUR(BLUE, YELLOW));
        write_str(suffix, COLOUR(BLUE, B_YELLOW));
        break;
      }
    } else if (ent.desc) { // is non-nullish
      write_str(ent.desc, COLOUR(BLUE, WHITE) | brightness);
    }
  }

  // right border
  write_cell('\263', POS(-1, ln_nr() + 1), COLOUR(BLUE, B_BLACK));
  paint_row(BLUE);
  line_feed();
}

HelpHost::HelpHost(const char* title, const HelpHost::Entry* loads): name(title), ents(loads) {
  // bitflags, or as many together as need be
  config_flags = 0; // enter can exit too, so we dont want it as a CTRL code, but instead as a real key

  app_name = "helphost";
}

HelpHost::~HelpHost() {
  // cur_on();
}
