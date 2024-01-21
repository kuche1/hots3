
#ifndef ENTITIES_H
#define ENTITIES_H

#include "player.h"

struct player *get_entity_at(int pos_y, int pos_x, struct player entities[ENTITIES_MAX]);

#endif
