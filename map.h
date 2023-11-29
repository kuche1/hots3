
#ifndef MAP_H
#define MAP_H

#include "player.h"

int map_is_tile_empty(struct player players[PLAYERS_MAX], int pos_y, int pos_x);

struct map_get_empty_tiles_near_return{
    int up;
    int down;
    int left;
    int right;
};
struct map_get_empty_tiles_near_return map_get_empty_tiles_near(struct player players[PLAYERS_MAX], int pos_y, int pos_x);

#endif
