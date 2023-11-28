
#include "player.h"

#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>

#include "networking.h"
#include "screen.h"
#include "settings.h"
#include "map.h"
#include "hero.h"
#include "util.h"

/////////////
///////////// private
/////////////

static void player_init_bot(struct player *player);

/////////////
///////////// initialising
/////////////

void player_init_mem(struct player *player){
    player->connfd = -1;
    memset(&player->sock, 0, sizeof(player->sock));
    player->sock_len = 0;

    player->health_color = "";
    player->health_color_len = 0;
    player->team_color = "";
    player->team_color_len = 0;
    player->level_color = "";
    player->level_color_len = 0;

    hero_init_mem(&player->hero);

    player->x = -1;
    player->y = -1;
    player->hp = 0;
    player->alive = 0;
    player->level = 0;
    player->xp = 0;
    player->died_at_ms = 0;

    player->team = 0;

    player->bot = MINION;
    player->bot_action_delay_ms = 1e3;
    player->bot_willpower = 1;
    player->bot_schizophrenia = 1;

    player->bot_last_action_at_ms = 0;
}

void player_init(struct player *player, int team, int is_bot, int connfd, struct sockaddr_in sock, int sock_len){
    player_init_mem(player);

    player->connfd = connfd;
    player->sock = sock;
    player->sock_len = sock_len;

    player->team = team;
    if(player->team){
        player->team_color     = STATIC_effect_no_strikethrough;
        player->team_color_len = sizeof(STATIC_effect_no_strikethrough);
    }else{
        player->team_color     = STATIC_effect_strikethrough;
        player->team_color_len = sizeof(STATIC_effect_strikethrough);
    }

    player->bot = is_bot;
    if(player->bot){
        player_init_bot(player);
    }
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

static void player_init_bot(struct player *player){
    player->connfd = -1;

    assert(player->bot);

    player->bot_last_action_at_ms = 0;

    if(player->bot == BOT){

        player->bot_action_delay_ms = BOT_REACTION_TIME_MS;

        player->bot_willpower = BOT_WILLPOWER;
        player->bot_schizophrenia = BOT_SCHIZOPHRENIA;

        player->bot_human_wave_numerator = BOT_HUMAN_WAVE_NUMERATOR;
        player->bot_human_wave_denomintor = BOT_HUMAN_WAVE_DENOMINTOR;

    }else if(player->bot == MINION){

        player->bot_action_delay_ms = MINION_REACTION_TIME_MS;

        player->bot_willpower = MINION_WILLPOWER;
        player->bot_schizophrenia = MINION_SCHIZOPHRENIA;

        player->bot_human_wave_numerator = MINION_HUMAN_WAVE_NUMERATOR;
        player->bot_human_wave_denomintor = MINION_HUMAN_WAVE_DENOMINTOR;

    }else{
        assert(0);
    }
}

void player_spawn(struct player *player, struct player players[PLAYERS_MAX]){

    player->died_at_ms = 0;

    player->hp = player->hero.hp_max;
    // health state recalc will be called later since it can cause a redraw

    player->level = 0;
    player->xp = 0;
    player_gain_xp(player, players, (LEVEL_ON_SPAWN * XP_FOR_LEVEL_UP) + XP_ON_SPAWN);
    assert(player->level >= 1); // otherwise the level color will not be initialised

    // set spawn

    int spawn_area_y = SPAWN_AREA_Y;
    int spawn_area_x = SPAWN_AREA_X;

    if(player->bot == MINION){
        spawn_area_y = MINION_SPAWN_AREA_Y;
        spawn_area_x = MINION_SPAWN_AREA_X;
    }

    // try to spawn

    int spawn_attempts_left = 50;

    for(; spawn_attempts_left>0; spawn_attempts_left--){

        int pos_y;
        int pos_x;

        if(player->team){
            pos_y = rand() % spawn_area_y;
            pos_x = rand() % spawn_area_x;
        }else{
            pos_y = rand() % spawn_area_y + (MAP_Y - spawn_area_y);
            pos_x = rand() % spawn_area_x + (MAP_X - spawn_area_x);
        }

        if(map_is_tile_empty(players, pos_y, pos_x)){
            player->y = pos_y;
            player->x = pos_x;
            break;
        }
    }

    if(spawn_attempts_left <= 0){
        exit(ERR_COULD_NOT_FIND_SPAWN_FOR_PLAYER);
    }

    // draw

    player_recalculate_health_state(player, players);
    player_draw(player, players);
}

void player_select_hero(struct player *player){
    hero_select_player_hero(&player->hero, player->connfd, player->bot);
}

/////////////
///////////// actions
/////////////


void player_process_action(struct player *player, char action, struct player players[PLAYERS_MAX]){

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

    // basic attack

    if(action == KEY_BASIC_ATTACK){
        player_basic_attack(player, players);
    }

    // heal ability

    if(action == KEY_HEAL_ABILITY){
        player_heal_ability(player, players);
    }
}

void player_basic_attack(struct player *player, struct player players[PLAYERS_MAX]){
    struct player *closest_player = NULL;
    int closest_distance = INT_MAX;

    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *other_player = &players[player_idx];

        if(other_player == player){
            continue;
        }

        if(!other_player->alive){
            continue;
        }

        if(player->team == other_player->team){
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
        int damage = player->hero.basic_attack_damage * player->level;
        player_receive_damage(closest_player, damage, players);
    }
}

void player_heal_ability(struct player *player, struct player players[PLAYERS_MAX]){
    struct player *heal_target = NULL;

    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *other_player = &players[player_idx];

        if(other_player == player){
            continue;
        }

        if(!other_player->alive){
            continue;
        }

        if(player->team != other_player->team){
            continue;
        }

        if(other_player->hp >= other_player->hero.hp_max){
            continue;
        }

        int distance = abs(player->x - other_player->x) + abs(player->y - other_player->y);
        if(distance > player->hero.heal_ability_range){
            continue;
        }

        if((heal_target == NULL) || (heal_target->hp > other_player->hp)){
            heal_target = other_player;
        }
    }

    if(heal_target != NULL){
        int heal = player->hero.heal_ability_amount * player->level;
        player_receive_damage(heal_target, -heal, players);
    }
}

/////////////
///////////// deal with status
/////////////

void player_receive_damage(struct player *player, int amount, struct player players[PLAYERS_MAX]){

    if(!player->alive){
        return;
    }

    if(amount > 0){
        // if being damaged
        amount /= player->level;
    }

    player->hp -= amount;

    if(player->hp <= 0){

        // reward opposite team

        int team = !player->team;
        int xp = player->xp;

        for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
            struct player *team_member = &players[player_idx];
            if(team_member->team != team){
                continue;
            }
            player_gain_xp(team_member, players, xp);
        }

        // deal with dying player

        player->alive = 0;
        player->died_at_ms = get_time_ms();
        screen_cur_set(players, player->y, player->x);
        net_send(players, STATIC_map_tile_empty, sizeof(STATIC_map_tile_empty));
        player->x = -1;
        player->y = -1;
    
        // exit

        return;
    }

    if(player->hp > player->hero.hp_max){ // if overhealed
        player->hp = player->hero.hp_max;
    }

    player_recalculate_health_state(player, players);
}

void player_recalculate_health_state(struct player *player, struct player players[PLAYERS_MAX]){

    static char health_state_palette[HEALTH_STATES][20]; // 0=healthy last=lowhp
    static int health_state_palette_generated = 0;

    if(!health_state_palette_generated){
        health_state_palette_generated = 1;

        for(int color_idx=0; color_idx < HEALTH_STATES; color_idx++){
            int red = (color_idx * 255) / (HEALTH_STATES-1);
            int green = 255 - red;
            int blue = 0;

            int written = snprintf(health_state_palette[color_idx], sizeof(health_state_palette[color_idx]), "\033[38;2;%d;%d;%dm", red, green, blue);
            assert(written >= 0);
            assert((long unsigned int)written < sizeof(health_state_palette[color_idx])); // buffer is too small
        }
    }

    player->alive = player->hp > 0;

    char *old_color = player->health_color;

    // if      (player->hp >= player->hero.hp_max){
    //     player->health_color     =        STATIC_col_green_dark;
    //     player->health_color_len = sizeof(STATIC_col_green_dark);

    // }else if(player->hp >= player->hero.hp_max * 6 / 7){
    //     player->health_color     =        STATIC_col_green_bright;
    //     player->health_color_len = sizeof(STATIC_col_green_bright);

    // }else if(player->hp >= player->hero.hp_max * 5 / 7){
    //     player->health_color     =        STATIC_col_yellow_dark;
    //     player->health_color_len = sizeof(STATIC_col_yellow_dark);

    // }else if(player->hp >= player->hero.hp_max * 4 / 7){
    //     player->health_color     =        STATIC_col_yellow_bright;
    //     player->health_color_len = sizeof(STATIC_col_yellow_bright);

    // }else if(player->hp >= player->hero.hp_max * 3 / 7){
    //     player->health_color     =        STATIC_col_magenta_dark;
    //     player->health_color_len = sizeof(STATIC_col_magenta_dark);

    // }else if(player->hp >= player->hero.hp_max * 2 / 7){
    //     player->health_color     =        STATIC_col_magenta_bright;
    //     player->health_color_len = sizeof(STATIC_col_magenta_bright);

    // }else if(player->hp >= player->hero.hp_max * 1 / 7){
    //     player->health_color     =        STATIC_col_red_dark;
    //     player->health_color_len = sizeof(STATIC_col_red_dark);

    // }else{
    //     player->health_color     =        STATIC_col_red_bright;
    //     player->health_color_len = sizeof(STATIC_col_red_bright);
    // }

    int health_amount = ((HEALTH_STATES-1) * player->hp) / player->hero.hp_max;
    int health_amount_idx = (HEALTH_STATES-1) - health_amount;

    if(health_amount_idx < 0){ // perhaps the hp is below 0
        health_amount_idx = 0;
    }

    assert((long unsigned int)health_amount_idx < (sizeof(health_state_palette) / sizeof(*health_state_palette)));

    player->health_color = health_state_palette[health_amount_idx];
    player->health_color_len = strlen(player->health_color);

    if(old_color != player->health_color){
        player_draw(player, players);
    }
}

void player_gain_xp(struct player *player, struct player players[PLAYERS_MAX], int xp){
    player->xp += xp;
    while(player->xp >= XP_FOR_LEVEL_UP){
        player->xp -= XP_FOR_LEVEL_UP;
        player->level += 1;
        
        if((player->level & 1) == (LEVEL_ON_SPAWN & 1)){
            player->level_color = STATIC_effect_no_inverse_reverse;
            player->level_color_len = sizeof(STATIC_effect_no_inverse_reverse);
        }else{
            player->level_color = STATIC_effect_inverse_reverse;
            player->level_color_len = sizeof(STATIC_effect_inverse_reverse);
        }

        int health_restored = (player->hp * LEVEL_UP_HEALTH_RESTORED_NUMERATOR) / LEVEL_UP_HEALTH_RESTORED_DENOMINATOR;
        player_receive_damage(player, -health_restored, players);

        player_draw(player, players); // TODO? draw too many times?
    }
}

/////////////
///////////// drawing
/////////////

void player_draw(struct player *player, struct player players[PLAYERS_MAX]){
    if(!player->alive){
        return;
    }

    if((player->x < 0) || (player->y < 0)){
        return;
    }

    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *player_receiver = &players[player_idx];

        screen_cur_set_single(player_receiver->connfd, player->y, player->x);

        screen_print_single(player_receiver->connfd, player->level_color, player->level_color_len);

        screen_print_single(player_receiver->connfd, player->health_color, player->health_color_len);

        screen_print_single(player_receiver->connfd, player->team_color, player->team_color_len);

        if(player_receiver == player){
            // indicate that this is the player itself
            screen_print_single(player_receiver->connfd, STATIC_effect_italic, sizeof(STATIC_effect_italic));
        }

        hero_draw_single(&player->hero, player_receiver->connfd);

        if(player_receiver == player){
            screen_print_single(player_receiver->connfd, STATIC_effect_no_italic, sizeof(STATIC_effect_no_italic));
        }
    }
}

/////////////
///////////// bot stuff
/////////////

int player_bot_select_action(struct player *player, struct player players[PLAYERS_MAX], char *action){

    // TODO? make healers pussies

    // do nothing if dead

    if(!player->alive){
        return 1;
    }

    // see if bot should wait and do nothing

    {
        long long now_ms = get_time_ms();

        if((player->bot_last_action_at_ms + player->bot_action_delay_ms) < now_ms){
            player->bot_last_action_at_ms = now_ms;
        }else{
            return 1;
        }
    }

    // your schizophrenia is trolling you

    if(!(rand() % player->bot_schizophrenia < player->bot_willpower)){
        char directions[] = {KEY_MOVE_DOWN, KEY_MOVE_UP, KEY_MOVE_LEFT, KEY_MOVE_RIGHT};
        char direction = directions[rand() % sizeof(directions)];
        *action = direction;
        return 0;
    }

    // find closest enemy

    struct player *attack_target = NULL;
    int attack_target_dist = INT_MAX;

    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *other_player = &players[player_idx];

        if(player == other_player){
            continue;
        }

        if(!other_player->alive){
            continue;
        }

        if(player->team == other_player->team){
            continue;
        }

        int dist = abs(player->y - other_player->y) + abs(player->x - other_player->x);

        if(dist < attack_target_dist){
            attack_target_dist = dist;
            attack_target = other_player;
        }
    }

    // find closest damaged ally

    struct player *closest_damaged_ally = NULL;
    int closest_damaged_ally_dist = INT_MAX;

    if((player->hero.heal_ability_range > 0) && (player->hero.heal_ability_amount > 0)){
        for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
            struct player *other_player = &players[player_idx];

            if(player == other_player){
                continue;
            }

            if(!other_player->alive){
                continue;
            }

            if(player->team != other_player->team){
                continue;
            }

            if(other_player->hp >= other_player->hero.hp_max){
                continue;
            }

            int dist = abs(player->y - other_player->y) + abs(player->x - other_player->x);

            if((closest_damaged_ally == NULL) || (closest_damaged_ally_dist > dist)){
                closest_damaged_ally = other_player;
                closest_damaged_ally_dist = dist;
            }
        }
    }

    // heal or attack if in range

    int heal_in_rage    = closest_damaged_ally_dist <= player->hero.heal_ability_range;
    int attack_in_range = attack_target_dist        <= player->hero.basic_attack_distance;

    if(heal_in_rage && attack_in_range){
        if(player->hero.heal_ability_amount > player->hero.basic_attack_damage){
            *action = KEY_HEAL_ABILITY;
            return 0;
        }else{
            *action = KEY_BASIC_ATTACK;
            return 0;
        }
    }else if(heal_in_rage){
        *action = KEY_HEAL_ABILITY;
        return 0;
    }else if(attack_in_range){
        *action = KEY_BASIC_ATTACK;
        return 0;
    }

    // cannot heal nor attack, select target to move to, or quit if no targets

    struct player *target = NULL;

    if((attack_target == NULL) && (closest_damaged_ally == NULL)){
        return 1;
    }else if(attack_target == NULL){
        target = closest_damaged_ally;
    }else if(closest_damaged_ally == NULL){
        target = attack_target;
    }else{
        target = closest_damaged_ally;
    }

    assert(target != NULL);

    // move to closest target // TODO? check if we're moving there for heal and make a b-line to the target

    {

        char human_wave_action;
        char encirclement_action;

        if(abs(player->y - target->y) > abs(player->x - target->x)){

            if(player->y < target->y){
                human_wave_action = KEY_MOVE_DOWN;
            }else{
                human_wave_action = KEY_MOVE_UP;
            }

            if(player->x < target->x){
                encirclement_action = KEY_MOVE_RIGHT;
            }else{
                encirclement_action = KEY_MOVE_LEFT;
            }

        }else{

            if(player->x < target->x){
                human_wave_action = KEY_MOVE_RIGHT;
            }else{
                human_wave_action = KEY_MOVE_LEFT;
            }

            if(player->y < target->y){
                encirclement_action = KEY_MOVE_DOWN;
            }else{
                encirclement_action = KEY_MOVE_UP;
            }

        }

        if(rand() % player->bot_human_wave_denomintor < player->bot_human_wave_numerator){
            *action = human_wave_action;
        }else{
            *action = encirclement_action;
        }

        return 0;
    
    }

    assert(0); // unreachable
}
