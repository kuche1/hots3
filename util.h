
#ifndef UTIL_H
#define UTIL_H

#include "settings.h"
#include "player.h"

long long get_time_ms(void);
struct player * generate_new_entity(struct player players[PLAYERS_MAX]); // not the most appropriate place but it will have to do

#endif
