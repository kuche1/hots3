
#include "map.h"

#include <stdio.h>
#include <limits.h>
#include <assert.h>

#include "settings.h"

int map_is_tile_empty(struct player players[PLAYERS_MAX], int pos_y, int pos_x){

    if((pos_y < 0) || (pos_x < 0)){
        return 0;
    }

    if((pos_y >= MAP_Y) || (pos_x >= MAP_X)){
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

struct direction_and_distance map_pathfind_depth_1(struct player players[PLAYERS_MAX], int start_y, int start_x, int dest_y, int dest_x){
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

// enum direction map_pathfind_depth_2(struct player players[PLAYERS_MAX], int start_y, int start_x, int dest_y, int dest_x){
//     enum direction_choice_left
// }