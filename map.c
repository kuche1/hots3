
#include "settings.h"

#include "map.h"

int map_is_tile_empty(struct player players[PLAYERS_REQUIRED], int pos_y, int pos_x){

    if((pos_y < 0) || (pos_x < 0)){
        return 0;
    }

    if((pos_y > MAP_Y) || (pos_x > MAP_X)){
        return 0;
    }

    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
        struct player *player = &players[player_idx];
        
        if((player->x == pos_x) && (player->y == pos_y)){
            return 0;
        }
    }

    return 1;
}
