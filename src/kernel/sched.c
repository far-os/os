#include "include/kappldr.h"
#include "include/err.h"
#include "include/sched.h"

// centisec (100ths of sec) since load
tick_t uptime = 0;

// list of ticks by which invokes should be timed. 
tick_t timed_invokes[AVAILABLE_KAPPS];

// check if invoke is due for the current focused app
void tick_focused_timed_invoke() {
  // if its less than uptime, but greater than zero, as zero means none set
  if (timed_invokes[curr_kapp] && timed_invokes[curr_kapp] <= uptime) {
    timed_invokes[curr_kapp] = 0; // reset
    (*app_db[curr_kapp]->virts->invoke)(app_db[curr_kapp]);
  }
}

// defacto wait
void set_timed_invoke(app_handle app_id, tick_t delta) {
  tick_t target = uptime + delta;
  // replace timed invoke if either previous timed invoke has passed, or new timed invoke is sooner than old one
  // this means that there can only ever be one timed invoke, and it is the soonest one.
  // and if the app changes, this will lie waiting until next focus
  if (timed_invokes[app_id] < uptime || timed_invokes[app_id] > target) {
    timed_invokes[app_id] = target;
  }
}

void clear_timed_invokes(app_handle app_id) {
  timed_invokes[app_id] = 0;
}