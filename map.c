
#include "map.h"

#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>

#include "settings.h"
#include "util.h"

static int tiles_pathfind_coeff[MAP_Y][MAP_X] = {{INT_MIN}}; // this should be overwritten

int map_is_tile_empty(struct player players[ENTITIES_MAX], int pos_y, int pos_x){

    if((pos_y < 0) || (pos_x < 0)){
        return 0;
    }

    if((pos_y >= MAP_Y) || (pos_x >= MAP_X)){
        return 0;
    }

    for(int player_idx=0; player_idx < ENTITIES_MAX; ++player_idx){
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

struct map_get_empty_tiles_near_return map_get_empty_tiles_near(struct player players[ENTITIES_MAX], int pos_y, int pos_x){
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

/////////////
///////////// closely related to pathfinding
/////////////

void map_clear_pathfind_data(void){
    memset(tiles_pathfind_coeff, -1, sizeof(tiles_pathfind_coeff)); // this just so happens to work because `-1` is really just 0b11111...
}

void map_mark_pathfind_tile_coeff(int y, int x, int coeff){
    assert(y >= 0);
    assert(y < MAP_Y);
    assert(x >= 0);
    assert(x < MAP_X);
    tiles_pathfind_coeff[y][x] = coeff;
}

int map_is_tile_coeff_ok(struct player players[ENTITIES_MAX], int pos_y, int pos_x, int caller_coeff){
    if((pos_y < 0) || (pos_x < 0)){
        return 0;
    }

    if((pos_y >= MAP_Y) || (pos_x >= MAP_X)){
        return 0;
    }

    if(tiles_pathfind_coeff[pos_y][pos_x] >= caller_coeff){
        return 0;
    }

    if(!map_is_tile_empty(players, pos_y, pos_x)){
        return 0;
    }

    return 1;
}

struct direction_and_distance map_pathfind_depth(struct player players[ENTITIES_MAX], int start_y, int start_x, int dest_y, int dest_x, enum check_start check_start, int depth){

    {
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
                if(!map_is_tile_coeff_ok(players, start_y, start_x, depth)){
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

    switch(check_start){
        case DONT_CHECK_START:
            map_clear_pathfind_data();
            break;
        case CHECK_START:
            break;
    }

    map_mark_pathfind_tile_coeff(start_y, start_x, depth);\

        struct direction_and_distance dnd_left;
        struct direction_and_distance dnd_right;
        struct direction_and_distance dnd_up;
        struct direction_and_distance dnd_down;

        assert(depth >= 1);

        switch(depth){
            case 1:
                dnd_left.direction = D_NONE;
                dnd_left.distance = INT_MAX;
                if(map_is_tile_coeff_ok(players, start_y,   start_x-1, depth-1)){
                    dnd_left.direction = D_LEFT;
                    dnd_left.distance = map_calc_dist(start_y,   start_x-1, dest_y, dest_x);
                }

                dnd_right.direction = D_NONE;
                dnd_right.distance = INT_MAX;
                if(map_is_tile_coeff_ok(players, start_y,   start_x+1, depth-1)){
                    dnd_right.direction = D_RIGHT;
                    dnd_right.distance = map_calc_dist(start_y,   start_x+1, dest_y, dest_x);
                }

                dnd_up.direction = D_NONE;
                dnd_up.distance = INT_MAX;
                if(map_is_tile_coeff_ok(players, start_y-1,   start_x, depth-1)){
                    dnd_up.direction = D_UP;
                    dnd_up.distance = map_calc_dist(start_y-1,   start_x, dest_y, dest_x);
                }

                dnd_down.direction = D_NONE;
                dnd_down.distance = INT_MAX;
                if(map_is_tile_coeff_ok(players, start_y+1,   start_x, depth-1)){
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

    switch(check_start){
        case DONT_CHECK_START:
            map_clear_pathfind_data();
            break;
        case CHECK_START:
            break;
    }

    // choose best path

    enum direction best_directions[4];
    int best_directions_len = 0;
    int best_direction_distance = INT_MAX;

    if(dnd_left.distance <= best_direction_distance){
        if(dnd_left.distance < best_direction_distance){
            best_direction_distance = dnd_left.distance;
            best_directions_len = 0;
        }
        best_directions[best_directions_len++] = D_LEFT;
    }

    if(dnd_right.distance <= best_direction_distance){
        if(dnd_right.distance < best_direction_distance){
            best_direction_distance = dnd_right.distance;
            best_directions_len = 0;
        }
        best_directions[best_directions_len++] = D_RIGHT;
    }

    if(dnd_up.distance <= best_direction_distance){
        if(dnd_up.distance < best_direction_distance){
            best_direction_distance = dnd_up.distance;
            best_directions_len = 0;
        }
        best_directions[best_directions_len++] = D_UP;
    }

    if(dnd_down.distance <= best_direction_distance){
        if(dnd_down.distance < best_direction_distance){
            best_direction_distance = dnd_down.distance;
            best_directions_len = 0;
        }
        best_directions[best_directions_len++] = D_DOWN;
    }

    // choose a path among the equally-long paths

    if(best_directions_len <= 0){
        struct direction_and_distance dnd = {
            .direction = D_NONE,
            .distance = INT_MAX,
        };
        return dnd;
    }

    int best_direction_idx = rand() % best_directions_len;
    enum direction best_direction = best_directions[best_direction_idx];

    // return

    if(best_direction_distance == INT_MAX){
        struct direction_and_distance dnd = {
            .direction = D_NONE,
            .distance = INT_MAX,
        };
        return dnd;
    }else{
        struct direction_and_distance dnd = {
            .direction = best_direction,
            .distance = best_direction_distance + 1,
        };
        return dnd;
    }

    assert(0);
}

/////////////
///////////// map loading
/////////////

void map_load(
    int *walls_x, int walls_x_len,
    int *walls_y, int walls_y_len,
    int *walls_team, int walls_team_len,
    struct player players[ENTITIES_MAX]
){

    assert(walls_x_len == walls_y_len);
    assert(walls_y_len == walls_team_len);
    int len = walls_x_len;

    for(int wall_idx=0; wall_idx<len; ++wall_idx){
        int x = walls_x[wall_idx];
        int y = walls_y[wall_idx];
        int team = walls_team[wall_idx];

        if(!map_is_tile_empty(players, y, x)){
            exit(ERR_CANT_SPAWN_WALL_SINCE_SPOT_IS_ALREADY_TAKEN); // probably duplicate walls or there are already players spawned there
        }

        struct player *wall = generate_new_entity(players);
        if(!wall){
            exit(ERR_CANT_SPAWN_WALL_SINCE_ENTITY_LIMIT_REACHED);
        }

        enum entity_type et = ET_WALL;
        int connfd = -1;
        struct sockaddr_in sock = {0};
        int sock_len = 0;

        player_init(wall, team, et, connfd, sock, sock_len);
        player_spawn(wall, players, y, x);
    }
}

int map_custom_map_exists(char *name){
    char path[512];
    int written = snprintf(path, sizeof(path), "maps/%s", name);
    assert(written >= 0);
    assert((long unsigned int)written < sizeof(path)); // buffer is too small

    DIR* dir = opendir(path);
    if (dir) {
        // it exists
        closedir(dir);
        return 1;
    }

    // it does not exist
    return 0;
}
