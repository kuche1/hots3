
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
#include "color.h"

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
    player->christmas_lights_on = 0;

    hero_init_mem(&player->hero);

    player->x = -1;
    player->y = -1;
    player->hp = 0;
    player->alive = 0;
    player->level = 0;
    player->xp = 0;
    player->died_at_ms = 0;

    player->team = 0;
    player->et = ET_MINION;

    player->bot_action_delay_ms = 1e3;
    player->bot_willpower = 1;
    player->bot_schizophrenia = 1;

    player->bot_last_action_at_ms = 0;
}

void player_init(struct player *player, int team, int entity_type, int connfd, struct sockaddr_in sock, int sock_len){
    player_init_mem(player);

    player->connfd = connfd;
    player->sock = sock;
    player->sock_len = sock_len;

    player->team = team;
    if(player->team){
        static char team_color[] = EFFECT_NO_INVERSE_REVERSE;
        player->team_color       = team_color;
        player->team_color_len   = sizeof(team_color);
    }else{
        static char team_color[] = EFFECT_INVERSE_REVERSE;
        player->team_color       = team_color;
        player->team_color_len   = sizeof(team_color);
    }

    player->et = entity_type;
    if(player->et){
        player_init_bot(player);
    }
}

void player_init_telnet(struct player *player){
    if(player->et){
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

    assert(player->et);

    player->bot_last_action_at_ms = 0;

    switch(player->et){
        case ET_HERO_BOT:

            player->bot_action_delay_ms = BOT_REACTION_TIME_MS;

            player->bot_willpower = BOT_WILLPOWER;
            player->bot_schizophrenia = BOT_SCHIZOPHRENIA;

            player->bot_human_wave_numerator = BOT_HUMAN_WAVE_NUMERATOR;
            player->bot_human_wave_denomintor = BOT_HUMAN_WAVE_DENOMINTOR;

            break;

        case ET_MINION:

            player->bot_action_delay_ms = MINION_REACTION_TIME_MS;

            player->bot_willpower = MINION_WILLPOWER;
            player->bot_schizophrenia = MINION_SCHIZOPHRENIA;

            player->bot_human_wave_numerator = MINION_HUMAN_WAVE_NUMERATOR;
            player->bot_human_wave_denomintor = MINION_HUMAN_WAVE_DENOMINTOR;

            break;
        
        case ET_TOWER:

            player->bot_action_delay_ms = TOWER_REACTION_TIME_MS;

            player->bot_willpower = TOWER_WILLPOWER;
            player->bot_schizophrenia = TOWER_SCHIZOPHRENIA;

            player->bot_human_wave_numerator = TOWER_HUMAN_WAVE_NUMERATOR;
            player->bot_human_wave_denomintor = TOWER_HUMAN_WAVE_DENOMINTOR;

            break;
        
        case ET_HERO_HUMAN:

            assert(0);

            break;
    }
}

void player_spawn(struct player *player, struct player players[PLAYERS_MAX]){

    player->died_at_ms = 0;

    // set spawn

    int spawn_area_y = 0;
    int spawn_area_x = 0;

    int is_tower = 0;

    switch(player->et){
        case ET_HERO_HUMAN:
        case ET_HERO_BOT:
            spawn_area_y = SPAWN_AREA_Y;
            spawn_area_x = SPAWN_AREA_X;
            break;

        case ET_MINION:
            spawn_area_y = MINION_SPAWN_AREA_Y;
            spawn_area_x = MINION_SPAWN_AREA_X;
            break;
        
        case ET_TOWER:
            is_tower = 1;
            spawn_area_y = TOWER_SPAWN_Y;
            spawn_area_x = TOWER_SPAWN_X;
            break;
    }

    // try to find a spot

    if(is_tower){

        int pos_y;
        int pos_x;

        if(player->team){
            pos_y = spawn_area_y;
            pos_x = spawn_area_x;
        }else{
            pos_y = (MAP_Y-1) - spawn_area_y;
            pos_x = (MAP_X-1) - spawn_area_x;
        }

        if(!map_is_tile_empty(players, pos_y, pos_x)){
            exit(ERR_COULD_NOT_SPAWN_TOWER);
        }

        player->y = pos_y;
        player->x = pos_x;

    }else{

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
            exit(ERR_COULD_NOT_FIND_SPAWN_SPOT);
        }

    }

    // fix health

    player->hp = player->hero.hp_max;
    player_recalculate_health_state(player, players);

    player->level = 0;
    player->xp = 0;
    player_gain_xp(player, players, (LEVEL_ON_SPAWN * XP_FOR_LEVEL_UP) + XP_ON_SPAWN);
    assert(player->level >= 1); // otherwise the level color will not be initialised

    // draw

    player_draw(player, players);
}

void player_select_hero(struct player *player){
    hero_select_player_hero(&player->hero, player->connfd, player->et);
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
            screen_print_empty_tile(players);

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
        player_toggle_christmas_lights(player, players); // indicate that an attack was performed
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
        player_toggle_christmas_lights(player, players); // indicate that a heal was performed
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

        int player_idx_max = PLAYERS_REQUIRED;
        if(MINIONS_AND_TOWERS_CAN_LEVEL_UP){
            player_idx_max = PLAYERS_MAX;
        }

        for(int player_idx=0; player_idx < player_idx_max; ++player_idx){
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
        screen_print_empty_tile(players);
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

    static char health_state_palette[HEALTH_STATES+1][20]; // 0=healthy last=lowhp // 1 additional state for when full hp
    static int health_state_palette_generated = 0;

    if(!health_state_palette_generated){
        health_state_palette_generated = 1;

        for(
            int color_idx=0;
            (long unsigned int)color_idx < LENOF(health_state_palette);
            color_idx++
        ){

            int red = (color_idx * 255) / (LENOF(health_state_palette)-1);
            int green = 255 - red;
            int blue = 0;

            int written = snprintf(health_state_palette[color_idx], sizeof(health_state_palette[color_idx]), "\033[38;2;%d;%d;%dm", red, green, blue);
            assert(written >= 0);
            assert((long unsigned int)written < sizeof(health_state_palette[color_idx])); // buffer is too small
        }
    }

    player->alive = player->hp > 0;

    char *old_color = player->health_color;

    int health_amount = ((HEALTH_STATES-1) * player->hp) / player->hero.hp_max;
    int health_amount_idx = (HEALTH_STATES-1) - health_amount;
    health_amount_idx += 1; // compensate for that 1 health states that only shows when full hp

    if(health_amount_idx < 0){ // perhaps the hp is below 0
        health_amount_idx = 0;
    }

    assert((long unsigned int)health_amount_idx < LENOF(health_state_palette));

    player->health_color = health_state_palette[health_amount_idx];
    player->health_color_len = strlen(player->health_color);

    if(old_color != player->health_color){
        player_draw(player, players);
    }
}

void player_gain_xp(struct player *player, struct player players[PLAYERS_MAX], int xp){
    if(!player->alive){
        return;
    }

    player->xp += xp;
    while(player->xp >= XP_FOR_LEVEL_UP){ // TODO? make it so that you give at least 1 xp
        player->xp -= XP_FOR_LEVEL_UP;
        player->level += 1;
        
        if((player->level & 1) == (LEVEL_ON_SPAWN & 1)){
            static char level_color[] = EFFECT_NO_STRIKETHROUGH;
            player->level_color       = level_color;
            player->level_color_len   = sizeof(level_color);
        }else{
            static char level_color[] = EFFECT_STRIKETHROUGH;
            player->level_color       = level_color;
            player->level_color_len   = sizeof(level_color);
        }

        int health_restored = (player->hp * LEVEL_UP_HEALTH_RESTORED_NUMERATOR) / LEVEL_UP_HEALTH_RESTORED_DENOMINATOR;
        player_receive_damage(player, -health_restored, players);

        player_draw(player, players); // TODO? draw too many times?
    }
}

/////////////
///////////// drawing
/////////////

void player_toggle_christmas_lights(struct player *player, struct player players[PLAYERS_MAX]){
    player->christmas_lights_on = !player->christmas_lights_on;
    player_draw(player, players);
}

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

        if(player->christmas_lights_on){
            // sample effect used for various shits
            static char christmas_lights_effect_on[] = EFFECT_ITALIC;
            screen_print_single(player_receiver->connfd, christmas_lights_effect_on, sizeof(christmas_lights_effect_on));
        }

        if(player_receiver == player){
            // indicate that this is the player itself
            static char player_is_self_color[] = EFFECT_BOLD;
            screen_print_single(player_receiver->connfd, player_is_self_color, sizeof(player_is_self_color));
        }

        hero_draw_single(&player->hero, player_receiver->connfd);

        if(player_receiver == player){
            static char player_is_self_color_off[] = EFFECT_NO_ITALIC;
            screen_print_single(player_receiver->connfd, player_is_self_color_off, sizeof(player_is_self_color_off));
        }

        if(player->christmas_lights_on){
            // sample effect used for various shits
            static char christmas_lights_effect_off[] = EFFECT_NO_ITALIC;
            screen_print_single(player_receiver->connfd, christmas_lights_effect_off, sizeof(christmas_lights_effect_off));
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
