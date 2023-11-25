
#include "player.h"

#include "networking.h"
#include "screen.h"
#include "settings.h"
#include "map.h"

#define PLAYER_MODEL_ERROR '0'

void player_init_mem(struct player *player){
    player->sock_len = sizeof(player->sock);
    player->x = 0;
    player->y = 0;
    player->model = PLAYER_MODEL_ERROR;
}

void player_init_telnet(struct player *player){
    // tell telnet client to not send on line but rather on character
    char telnet_mode_character[] = "\377\375\042\377\373\001";
    net_send_single(player->connfd, telnet_mode_character, sizeof(telnet_mode_character));
}

void player_spawn(struct player *player, struct player players[PLAYERS_REQUIRED]){
    if(player->model == PLAYER_MODEL_ERROR){
        player->model += 1;
    }

    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
        struct player *other_player = &players[player_idx];
        
        if(other_player == player){
            continue;
        }

        if((player->x == other_player->x) && (player->y == other_player->y)){
            player->x += 1;
            if(player->x >= MAP_X){
                player->x = 0;
                player->y += 1;
                if(player->y >= MAP_Y){
                    exit(ERR_NOT_ENOUGH_TILES_TO_SPAWN_ALL_PLAYERS);
                }
            }
            player_spawn(player, players);
            return;
        }
    }
}

void player_draw(struct player *player, struct player players[PLAYERS_REQUIRED]){
    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
        struct player *player_receiver = &players[player_idx];

        screen_cur_set_single(player_receiver->connfd, player->y, player->x);
        net_send_single(player_receiver->connfd, &player->model, sizeof(player->model));
    }
}

void player_process_action(struct player *player, char action, struct player players[PLAYERS_REQUIRED]){

    // movement

    int x_desired = player->x;
    int y_desired = player->y;

    switch(action){
        case KEY_MOVE_LEFT:
            x_desired -= 1;
            break;

        case KEY_MOVE_RIGHT:
            x_desired += 1;
            break;

        case KEY_MOVE_UP:
            y_desired -= 1;
            break;

        case KEY_MOVE_DOWN:
            y_desired += 1;
            break;
    }

    if(map_is_tile_empty(players, y_desired, x_desired)){
        screen_cur_set(players, player->y, player->x);
        char empty_tile = MAP_TILE_EMPTY;
        net_send(players, &empty_tile, sizeof(empty_tile));

        player->x = x_desired;
        player->y = y_desired;
        player_draw(player, players);
    }
}
