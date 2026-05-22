#pragma once

#include "kappldr.h"

typedef unsigned int tick_t;

extern tick_t uptime;
extern tick_t timed_invokes[AVAILABLE_KAPPS];

void tick_focused_timed_invoke();
void set_timed_invoke(app_handle app_id, tick_t delta);
void clear_timed_invokes(app_handle app_id);