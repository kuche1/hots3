
#include "map.h"

#include <stdio.h>
#include <limits.h>
#include <assert.h>

#include "settings.h"

static int unpassable_tiles[MAP_Y][MAP_X] = {0};

int map_is_tile_empty(struct player players[PLAYERS_MAX], int pos_y, int pos_x){

    if((pos_y < 0) || (pos_x < 0)){
        return 0;
    }

    if((pos_y >= MAP_Y) || (pos_x >= MAP_X)){
        return 0;
    }

    if(unpassable_tiles[pos_y][pos_x]){
        return 0;
    }

    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *player = &players[player_idx];
        
        if(!player->alive){
            continue;
        }

        if((player->x == pos_x) && (player->y == pos_y)){
            return 0;
        }
    }

    return 1;
}

struct map_get_empty_tiles_near_return map_get_empty_tiles_near(struct player players[PLAYERS_MAX], int pos_y, int pos_x){
    struct map_get_empty_tiles_near_return ret = {0};

    if(map_is_tile_empty(players, pos_y-1, pos_x)){
        ret.up = 1;
    }
    if(map_is_tile_empty(players, pos_y+1, pos_x)){
        ret.down = 1;
    }
    if(map_is_tile_empty(players, pos_y, pos_x-1)){
        ret.left = 1;
    }
    if(map_is_tile_empty(players, pos_y, pos_x+1)){
        ret.right = 1;
    }
    
    return ret;
}

int map_calc_dist(int start_y, int start_x, int dest_y, int dest_x){
    return abs(start_y - dest_y) + abs(start_x - dest_x);
}

void map_mark_tile_as_unpassable(int y, int x){
    assert(y >= 0);
    assert(y < MAP_Y);
    assert(x >= 0);
    assert(x < MAP_X);
    // if((y < 0) || (y >= MAP_Y) || (x < 0) || (x >= MAP_X)){
    //     return;
    // }
    unpassable_tiles[y][x] += 1;
    assert(unpassable_tiles[y][x] <= 1);
}

void map_mark_tile_as_passable(int y, int x){
    assert(y >= 0);
    assert(y < MAP_Y);
    assert(x >= 0);
    assert(x < MAP_X);
    // if((y < 0) || (y >= MAP_Y) || (x < 0) || (x >= MAP_X)){
    //     return;
    // }
    unpassable_tiles[y][x] -= 1;
    assert(unpassable_tiles[y][x] >= 0);
}

struct direction_and_distance map_pathfind_depth(struct player players[PLAYERS_MAX], int start_y, int start_x, int dest_y, int dest_x, enum check_start check_start, int depth){

    // TODO optimise by checking if a given path has already been analysed

    { // TODO it fucking sucks that I have to copy this
        int dist = map_calc_dist(start_y, start_x, dest_y, dest_x);

        // if we have already reached the destination

        if(dist <= 0){
            struct direction_and_distance dnd = {
                .direction = D_NONE,
                .distance = dist,
            };
            return dnd;
        }

        // if the destination is a solid object and we are touching it

        if(dist <= 1){
            if(!map_is_tile_empty(players, dest_y, dest_x)){
                struct direction_and_distance dnd = {
                    .direction = D_NONE,
                    .distance = dist,
                };
                return dnd;
            }
        }

        // if the starting position is invalid
        // and the flag for checking the start position is set
        // (at the very beginning this will be equal to the player that tried to pathfind)

        switch(check_start){
            case DONT_CHECK_START:
                break;
            case CHECK_START:
                if(!map_is_tile_empty(players, start_y, start_x)){
                    struct direction_and_distance dnd = {
                        .direction = D_NONE,
                        .distance = INT_MAX,
                    };
                    return dnd;
                }
                break;
        }
    }

    // do the job

    map_mark_tile_as_unpassable(start_y, start_x);

        struct direction_and_distance dnd_left;
        struct direction_and_distance dnd_right;
        struct direction_and_distance dnd_up;
        struct direction_and_distance dnd_down;

        assert(depth >= 1);

        switch(depth){
            case 1:
                dnd_left.direction = D_NONE;
                dnd_left.distance = INT_MAX;
                if(map_is_tile_empty(players, start_y,   start_x-1)){
                    dnd_left.direction = D_LEFT;
                    dnd_left.distance = map_calc_dist(start_y,   start_x-1, dest_y, dest_x);
                }

                dnd_right.direction = D_NONE;
                dnd_right.distance = INT_MAX;
                if(map_is_tile_empty(players, start_y,   start_x+1)){
                    dnd_right.direction = D_RIGHT;
                    dnd_right.distance = map_calc_dist(start_y,   start_x+1, dest_y, dest_x);
                }

                dnd_up.direction = D_NONE;
                dnd_up.distance = INT_MAX;
                if(map_is_tile_empty(players, start_y-1,   start_x)){
                    dnd_up.direction = D_UP;
                    dnd_up.distance = map_calc_dist(start_y-1,   start_x, dest_y, dest_x);
                }

                dnd_down.direction = D_NONE;
                dnd_down.distance = INT_MAX;
                if(map_is_tile_empty(players, start_y+1,   start_x)){
                    dnd_down.direction = D_DOWN;
                    dnd_down.distance = map_calc_dist(start_y+1,   start_x, dest_y, dest_x);
                }

                break;

            default: // >2
                dnd_left  = map_pathfind_depth(players, start_y,   start_x-1, dest_y, dest_x, CHECK_START, depth-1);
                dnd_right = map_pathfind_depth(players, start_y,   start_x+1, dest_y, dest_x, CHECK_START, depth-1);
                dnd_up    = map_pathfind_depth(players, start_y-1, start_x,   dest_y, dest_x, CHECK_START, depth-1);
                dnd_down  = map_pathfind_depth(players, start_y+1, start_x,   dest_y, dest_x, CHECK_START, depth-1);
                break;
        }

    map_mark_tile_as_passable(start_y, start_x);

    // choose best path

    struct direction_and_distance closest_dnd = {
        .direction = D_LEFT,
        .distance = dnd_left.distance,
    };

    if(dnd_right.distance < closest_dnd.distance){
        closest_dnd.direction = D_RIGHT;
        closest_dnd.distance = dnd_right.distance;
    }
    if(dnd_up.distance < closest_dnd.distance){
        closest_dnd.direction = D_UP;
        closest_dnd.distance = dnd_up.distance;
    }
    if(dnd_down.distance < closest_dnd.distance){
        closest_dnd.direction = D_DOWN;
        closest_dnd.distance = dnd_down.distance;
    }

    if(closest_dnd.distance == INT_MAX){
        closest_dnd.direction = D_NONE;
    }else{
        closest_dnd.distance += 1;
    }

    return closest_dnd;
}
