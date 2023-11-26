
#include "player.h"

#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <sys/time.h>
#include <string.h>

#include "networking.h"
#include "screen.h"
#include "settings.h"
#include "map.h"
#include "hero.h"

/////////////
///////////// initialising
/////////////

void player_init_mem(struct player *player){
    player->connfd = -1;
    player->sock_len = sizeof(player->sock);
    player->x = 0;
    player->y = 0;
    player->health_color = STATIC_col_green_bright;
    player->health_color_len = sizeof(STATIC_col_green_bright);
    player->alive = 1;
    player->bot = 0;
    hero_init_mem(&player->hero);
    player->hp = player->hero.hp_max;
    player->team_color = STATIC_col_bg_red_dark;
    player->team_color_len = sizeof(STATIC_col_bg_red_dark);
    player->team = 0;
}

void player_init_telnet(struct player *player){
    if(player->bot){
        return;
    }

    // tell telnet client to not send on line but rather on character
    char telnet_mode_character[] = "\377\375\042\377\373\001";
    screen_print_single(player->connfd, telnet_mode_character, sizeof(telnet_mode_character));

    // use ansi escape code to hide the cursor
    // https://notes.burke.libbey.me/ansi-escape-codes/
    char hide_cur_code[] = "\x1b[?25l";
    screen_print_single(player->connfd, hide_cur_code, sizeof(hide_cur_code));

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

    player->hp = player->hero.hp_max;
    player_recalculate_health_state(player, players);

    if(player->team){
        player->team_color = STATIC_col_bg_black_dark;
        player->team_color_len = sizeof(STATIC_col_bg_black_dark);
    }else{
        player->team_color = STATIC_effect_underline;
        player->team_color_len = sizeof(STATIC_effect_underline);
    }

    // set spawn

    for(int loop_count=0; loop_count<50; loop_count++){

        int pos_y = rand() % MAP_Y;
        int pos_x = rand() % MAP_X;
        if(map_is_tile_empty(players, pos_y, pos_x)){
            player->y = pos_y;
            player->x = pos_x;
            return;
        }
    }

    exit(ERR_COULD_NOT_FIND_SPAWN_FOR_PLAYER);
}

void player_select_hero(struct player *player){
    hero_select_player_hero(&player->hero, player->connfd, player->bot);
}

/////////////
///////////// actions
/////////////


void player_process_action(struct player *player, char action, struct player players[PLAYERS_REQUIRED]){

    if(!player->alive){
        return;
    }

    // movement

    if(rand() % player->hero.weight < player->hero.legpower){

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

        if(!other_player->alive){
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

    if(closest_distance <= player->hero.basic_attack_distance){
        player_receive_damage(closest_player, player->hero.basic_attack_damage, players);
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

    player_recalculate_health_state(player, players);
}

void player_recalculate_health_state(struct player *player, struct player players[PLAYERS_REQUIRED]){

    char *old_color = player->health_color;

    if(player->hp >= player->hero.hp_max){
        player->health_color = STATIC_col_green_bright;
        player->health_color_len = sizeof(STATIC_col_green_bright);
    }else if(player->hp >= player->hero.hp_max * 5 / 6){
        player->health_color = STATIC_col_green_dark;
        player->health_color_len = sizeof(STATIC_col_green_dark);
    }else if(player->hp >= player->hero.hp_max * 4 / 6){
        player->health_color = STATIC_col_yellow_bright;
        player->health_color_len = sizeof(STATIC_col_yellow_bright);
    }else if(player->hp >= player->hero.hp_max * 3 / 6){
        player->health_color = STATIC_col_yellow_dark;
        player->health_color_len = sizeof(STATIC_col_yellow_dark);
    }else if(player->hp >= player->hero.hp_max * 2 / 6){
        player->health_color = STATIC_col_red_bright;
        player->health_color_len = sizeof(STATIC_col_red_bright);
    }else{
        player->health_color = STATIC_col_red_dark;
        player->health_color_len = sizeof(STATIC_col_red_dark);
    }

    if(old_color != player->health_color){
        player_draw(player, players);
    }
}

/////////////
///////////// bot stuff
/////////////

int player_bot_select_action(struct player *player, struct player players[PLAYERS_REQUIRED], char *action){

    // TODO make the bot do random things sometimes

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

        if(!other_player->alive){
            continue;
        }

        int dist = abs(player->y - other_player->y) + abs(player->x - other_player->x);

        if(dist < lowest_dist){
            lowest_dist = dist;
            target = other_player;
        }
    }

    if(lowest_dist <= player->hero.basic_attack_distance){
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

/////////////
///////////// other stuff
/////////////

void player_draw(struct player *player, struct player players[PLAYERS_REQUIRED]){
    if(!player->alive){
        return;
    }

    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
        struct player *player_receiver = &players[player_idx];

        screen_cur_set_single(player_receiver->connfd, player->y, player->x);
        screen_print_single(player_receiver->connfd, player->health_color, player->health_color_len);
        screen_print_single(player_receiver->connfd, player->team_color, player->team_color_len);
        hero_draw_single(&player->hero, player_receiver->connfd);
    }
}
