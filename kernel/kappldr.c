#include "include/kappldr.h"
#include "include/err.h"
#include "include/text.h"

struct k_app *app_db[AVAILABLE_KAPPS];
app_handle curr_kapp = -1;

app_handle instantiate(struct k_app *app, app_handle parent) {
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
  return found;
}

void focus_app(app_handle which) {
  curr_kapp = which;
  set_page(which); // move to the page on which app is operating
}

void terminate_app(app_handle which) {
  // smth involving delete
}
