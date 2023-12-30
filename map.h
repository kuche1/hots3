
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

// closely related to pathfinding

enum check_start{
    DONT_CHECK_START = 0,
    CHECK_START,
};

void map_clear_pathfind_data(void);
void map_mark_pathfind_tile_coeff(int y, int x, int coeff);
int map_is_tile_coeff_ok(struct player players[PLAYERS_MAX], int pos_y, int pos_x, int caller_coeff);

struct direction_and_distance map_pathfind_depth(struct player players[PLAYERS_MAX], int start_y, int start_x, int dest_y, int dest_x, enum check_start check_start, int depth);

// map loading

void map_load(
    int *walls_x, int walls_x_len,
    int *walls_y, int walls_y_len,
    int *walls_team, int walls_team_len,
    struct player players[PLAYERS_MAX]
);

#endif
