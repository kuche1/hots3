
#ifndef MAP_H
#define MAP_H

#include "player.h"

enum direction{
    D_NONE = 0,
    D_UP,
    D_DOWN,
    D_LEFT,
    D_RIGHT,
};

struct direction_and_distance{
    enum direction direction;
    int distance;
};

int map_is_tile_empty(struct player players[PLAYERS_MAX], int pos_y, int pos_x);

struct map_get_empty_tiles_near_return{
    int up;
    int down;
    int left;
    int right;
};
struct map_get_empty_tiles_near_return map_get_empty_tiles_near(struct player players[PLAYERS_MAX], int pos_y, int pos_x);

int map_calc_dist(int start_y, int start_x, int dest_y, int dest_x);

void map_mark_tile_as_unpassable(int y, int x);
void map_mark_tile_as_passable(int y, int x);

struct direction_and_distance map_pathfind_depth_1(struct player players[PLAYERS_MAX], int start_y, int start_x, int dest_y, int dest_x);
struct direction_and_distance map_pathfind_depth_2(struct player players[PLAYERS_MAX], int start_y, int start_x, int dest_y, int dest_x);
struct direction_and_distance map_pathfind_depth  (struct player players[PLAYERS_MAX], int start_y, int start_x, int dest_y, int dest_x, int depth);

#endif
