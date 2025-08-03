#include "include/kappldr.h"
#include "include/err.h"
#include "include/misc.h"
#include "include/text.h"
#include "include/util.h"
#include "include/memring.h"

struct k_app *app_db[AVAILABLE_KAPPS];
app_handle curr_kapp = -1;

app_handle instantiate(struct k_app *app, app_handle parent, char is_fg) {
  app_handle found = -1;
  for (app_handle i = 0; i < AVAILABLE_KAPPS; ++i) {
    if (!app_db[i]) {
      found = i;
      break;
    }
  }
  
  if (found == -1) {
    msg(KERNERR, E_HANDLEALLOC, "Error allocating app");
    return -1;
  }

  // tell the app who it is and who its parent is
  app->app_id = (parent << 4) | (found & 0xf);
  app_db[found] = app; // put app in table

  char *g = malloc(32);
  sprintf(g, "Running app on @%d", found);
  msg(INFO, NONE, g);
  free(g);
  
  if (is_fg) {
    focus_app(found); // background apps dont need to do this
    clear_scr();
    set_cur(0);
  }

  (*app->virts->first_run)(app); // initialise
  return found;
}

void focus_app(app_handle which) {
  if ((which & 0xf) >= AVAILABLE_KAPPS || !(app_db[which])) { // is a valid app that is running, if no
    char *g = malloc(32);
    sprintf(g, "Cannot switch to app @%d", which);
    msg(WARN, E_BADADDR, g);
    free(g);
    return;
  }
  curr_kapp = which;
//  (*app_db[curr_kapp]->virts->invoke)(app_db[curr_kapp]);
  
  set_page(which); // move to the page on which app is operating
}

void terminate_app(app_handle which) {
  app_handle parent = app_db[which]->app_id >> 4;
  kapp_destroy(app_db[which]);
  app_db[which] = NULL; // no switching back

  if (curr_kapp == which) {
    if (is_split) { split_scr(0); } // unsplit
    focus_app(parent);
  }

  (*app_db[curr_kapp]->virts->invoke)(app_db[curr_kapp]);
}
