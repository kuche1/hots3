
#include "player.h"

#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <sys/time.h>

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
    player->hp_max = player->hp;
    player->health_state = 0;
    player->health_color = STATIC_col_green_bright;
    player->health_color_len = sizeof(STATIC_col_green_bright);
    player->basic_attack_distance = 1;
    player->basic_attack_damage = 1;
    player->alive = 1;
    player->bot = 0;
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

void player_init_bot(struct player *player){
    player->bot = 1;
    player->bot_action_delay_ms = BOT_REACTION_TIME_MS;
    player->bot_last_action_at_ms = 0;
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
    if(!player->alive){
        return;
    }

    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
        struct player *player_receiver = &players[player_idx];

        screen_cur_set_single(player_receiver->connfd, player->y, player->x);
        net_send_single(player_receiver->connfd, player->health_color, player->health_color_len);
        net_send_single(player_receiver->connfd, &player->model, sizeof(player->model));
    }
}

int player_bot_select_action(struct player *player, struct player players[PLAYERS_REQUIRED], char *action){

    {
        struct timeval te; 
        gettimeofday(&te, NULL); // get current time
        long long now_ms = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds

        if((player->bot_last_action_at_ms + player->bot_action_delay_ms) < now_ms){
            player->bot_last_action_at_ms = now_ms;
        }else{
            return 1;
        }
    }

    int lowest_dist = INT_MAX;
    struct player *target = NULL;

    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
        struct player *other_player = &players[player_idx];

        if(player == other_player){
            continue;
        }

        if(!player->alive){
            continue;
        }

        int dist = abs(player->y - other_player->y) + abs(player->x - other_player->x);

        if(dist < lowest_dist){
            lowest_dist = dist;
            target = other_player;
        }
    }

    if(lowest_dist <= player->basic_attack_distance){
        *action = KEY_BASIC_ATTACK;
        return 0;
    }

    if(abs(player->y - target->y) > abs(player->x - target->x)){
        if(player->y < target->y){
            *action = KEY_MOVE_DOWN;
        }else{
            *action = KEY_MOVE_UP;
        }
    }else{
        if(player->x < target->x){
            *action = KEY_MOVE_RIGHT;
        }else{
            *action = KEY_MOVE_LEFT;
        }
    }

    return 0;
}

void player_process_action(struct player *player, char action, struct player players[PLAYERS_REQUIRED]){

    if(!player->alive){
        return;
    }

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
        net_send(players, STATIC_map_tile_empty, sizeof(STATIC_map_tile_empty));

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
        player_receive_damage(closest_player, player->basic_attack_damage, players);
    }
}

void player_receive_damage(struct player *player, int amount, struct player players[PLAYERS_REQUIRED]){

#ifdef DEBUG
    amount *= 30;
#endif

    player->hp -= amount;
    // printf("hp of %p is now %d\n", (void*)player, player->hp);

    if(player->hp <= 0){
        player->alive = 0;
        screen_cur_set(players, player->y, player->x);
        net_send(players, STATIC_map_tile_empty, sizeof(STATIC_map_tile_empty));
        player->x = -1;
        player->y = -1;
        return;
    }

    // int health_state = (2*player->hp_max) / player->hp;
    // 1 = 1/1 hp = 100%
    // 2 = 1/2 hp =  50%
    // 3 = 1/3 hp =  33%

    int health_state = (2*player->hp_max) / player->hp;
    //  2 = 2/ 2 hp = 100%
    //  3 = 2/ 3 hp =  66%
    //  4 = 2/ 4 hp =  50%
    //  5 = 2/ 5 hp =  40%
    //  6 = 2/ 6 hp =  33%
    //  7 = 2/ 7 hp =  29%
    //  8 = 2/ 8 hp =  25%
    //  9 = 2/ 9 hp =  22%
    // 10 = 2/10 hp =  20%
    // 11 = 2/11 hp =  18%

    if(health_state != player->health_state){
        player->health_state = health_state;

        switch(player->health_state){
            case 0:
            case 1:
            case 2:
            case 3: // [100:66]
                player->health_color = STATIC_col_green_bright;
                player->health_color_len = sizeof(STATIC_col_green_bright);
                break;
            case 4: // (66:50]
                player->health_color = STATIC_col_green_dark;
                player->health_color_len = sizeof(STATIC_col_green_dark);
                break;
            case 5:// (50:40]
                player->health_color = STATIC_col_yellow_bright;
                player->health_color_len = sizeof(STATIC_col_yellow_bright);
                break;
            case 6: // (40:33]
                player->health_color = STATIC_col_yellow_dark;
                player->health_color_len = sizeof(STATIC_col_yellow_dark);
                break;
            case 7: // (33:29]
                player->health_color = STATIC_col_red_bright;
                player->health_color_len = sizeof(STATIC_col_red_bright);
                break;
            case 8: // (29:0]
                player->health_color = STATIC_col_red_dark;
                player->health_color_len = sizeof(STATIC_col_red_dark);
                break;
        }

        player_draw(player, players);
    }
}
