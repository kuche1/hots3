
#include "player.h"

#include <fcntl.h>
#include <stdio.h>
#include <limits.h>

#include "networking.h"
#include "screen.h"
#include "settings.h"
#include "map.h"

void player_init_mem(struct player *player){
    player->sock_len = sizeof(player->sock);
    player->model = '0';
    player->x = 0;
    player->y = 0;
    player->hp = 100;
    player->basic_attack_distance = 1;
    player->basic_attack_damage = 1;
}

void player_init_telnet(struct player *player){
    // tell telnet client to not send on line but rather on character
    char telnet_mode_character[] = "\377\375\042\377\373\001";
    net_send_single(player->connfd, telnet_mode_character, sizeof(telnet_mode_character));

    // also make the socket nonblocking
    int flags = fcntl(player->connfd, F_GETFL, 0);
    if(flags == -1){
        exit(ERR_CANT_GET_FCNTL_FOR_CLIENT_SOCKET);
    }
    flags = flags | O_NONBLOCK;
    if(fcntl(player->connfd, F_SETFL, flags) == -1){
        exit(ERR_CANT_SET_FCNTL_FOR_CLIENT_SOCKET);
    }
}

void player_spawn(struct player *player, struct player players[PLAYERS_REQUIRED]){

    // set model

    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){ // TODO this is giga shit
        struct player *other_player = &players[player_idx];
        
        if(other_player == player){
            continue;
        }

        if(player->model == other_player->model){
            player->model += 1;
            player_spawn(player, players);
            return;
        }
    }

    // set spawn

    for(;;){
        int done = 0;

        for(int pos_y=0; pos_y < MAP_Y; pos_y++){
            for(int pos_x=0; pos_x < MAP_X; pos_x++){
                if(map_is_tile_empty(players, pos_y, pos_x)){
                    player->y = pos_y;
                    player->x = pos_x;
                    printf("set spawn at %d %d\n", pos_y, pos_x);
                    done = 1;
                    break;
                }
            }
            if(done){
                break;
            }
        }

        if(done){
            break;
        }

        exit(ERR_NOT_ENOUGH_TILES_TO_SPAWN_ALL_PLAYERS);
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

    // attack

    if(action == KEY_BASIC_ATTACK){
        player_basic_attack(player, players);
    }
}

void player_basic_attack(struct player *player, struct player players[PLAYERS_REQUIRED]){
    struct player *closest_player = NULL;
    int closest_distance = INT_MAX;

    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
        struct player *other_player = &players[player_idx];

        if(other_player == player){
            continue;
        }

        int distance = abs(player->x - other_player->x) + abs(player->y - other_player->y);
        if(distance < closest_distance){
            closest_distance = distance;
            closest_player = other_player;
        }
    }

    if(!closest_player){
        return;
    }

    if(closest_distance <= player->basic_attack_distance){
        player_receive_damage(closest_player, player->basic_attack_damage);
    }
}

void player_receive_damage(struct player *player, int amount){
    player->hp -= amount;
    // TODO do something if less than 0
    printf("hp of %p is now %d\n", (void*)player, player->hp);
}
