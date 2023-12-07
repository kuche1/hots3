
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

struct direction_and_distance map_pathfind_depth_1(struct player players[PLAYERS_MAX], int start_y, int start_x, int dest_y, int dest_x){

    // check if we're already "touching" the destination
    // this will only work if you're trying to find a path to a player (or a wall)
    // otherwise we'll have to add some more login
    {
        int dist = map_calc_dist(start_y, start_x, dest_y, dest_x);
        if(dist <= 1){
            struct direction_and_distance dnd = {
                .direction = D_NONE,
                .distance = dist,
            };
            return dnd;
        }

    }

    struct map_get_empty_tiles_near_return tiles = map_get_empty_tiles_near(players, start_y, start_x);

    enum direction available[4];
    int available_len = 0;

    if(tiles.left){
        available[available_len] = D_LEFT;
        available_len += 1;
    }
    if(tiles.right){
        available[available_len] = D_RIGHT;
        available_len += 1;
    }
    if(tiles.up){
        available[available_len] = D_UP;
        available_len += 1;
    }
    if(tiles.down){
        available[available_len] = D_DOWN;
        available_len += 1;
    }


    int closest_distance = INT_MAX;
    enum direction closest_direction = D_NONE;

    for(int available_idx=0; available_idx<available_len; ++available_idx){
        enum direction direction = available[available_idx];

        int y_ofs = 0;
        int x_ofs = 0;

        switch(direction){
            case D_LEFT:
                x_ofs = -1;
                break;
            case D_RIGHT:
                x_ofs = 1;
                break;
            case D_UP:
                y_ofs = -1;
                break;
            case D_DOWN:
                y_ofs = 1;
                break;
            case D_NONE:
                assert(0);
                break;
        }

        int distance = map_calc_dist(start_y+y_ofs, start_x+x_ofs, dest_y, dest_x);

        if(distance < closest_distance){
            closest_distance = distance;
            closest_direction = direction;
        }
    }

    struct direction_and_distance dnd = {
        .direction = closest_direction,
        .distance = closest_distance,
    };

    return dnd;
}

struct direction_and_distance map_pathfind_depth_2(struct player players[PLAYERS_MAX], int start_y, int start_x, int dest_y, int dest_x){

    map_mark_tile_as_unpassable(start_y, start_x);
        // TODO what if the starting position is a wall
        struct direction_and_distance dnd_left  = map_pathfind_depth_1(players, start_y,   start_x-1, dest_y, dest_x);
        struct direction_and_distance dnd_right = map_pathfind_depth_1(players, start_y,   start_x+1, dest_y, dest_x);
        struct direction_and_distance dnd_up    = map_pathfind_depth_1(players, start_y-1, start_x,   dest_y, dest_x);
        struct direction_and_distance dnd_down  = map_pathfind_depth_1(players, start_y+1, start_x,   dest_y, dest_x);
    map_mark_tile_as_passable(start_y, start_x);

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

struct direction_and_distance map_pathfind_depth(struct player players[PLAYERS_MAX], int start_y, int start_x, int dest_y, int dest_x, int depth){

    assert(depth >= 1);

    switch(depth){
        case 1:
            return map_pathfind_depth_1(players, start_y, start_x, dest_y, dest_x);
        case 2:
            return map_pathfind_depth_2(players, start_y, start_x, dest_y, dest_x);
    }

    // do the job

    map_mark_tile_as_unpassable(start_y, start_x);
        struct direction_and_distance dnd_left  = map_pathfind_depth(players, start_y,   start_x-1, dest_y, dest_x, depth-1);
        struct direction_and_distance dnd_right = map_pathfind_depth(players, start_y,   start_x+1, dest_y, dest_x, depth-1);
        struct direction_and_distance dnd_up    = map_pathfind_depth(players, start_y-1, start_x,   dest_y, dest_x, depth-1);
        struct direction_and_distance dnd_down  = map_pathfind_depth(players, start_y+1, start_x,   dest_y, dest_x, depth-1);
    map_mark_tile_as_passable(start_y, start_x);

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
