
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

static int player_process_action(struct player *player, char action, struct player players[PLAYERS_MAX]);

static void player_init_bot(struct player *player);
static int player_bot_select_action(struct player *player, struct player players[PLAYERS_MAX], char *action);

/////////////
///////////// initialising
/////////////

void player_init_mem(struct player *player){
    player->connfd = -1;
    memset(&player->sock, 0, sizeof(player->sock));
    player->sock_len = 0;

    player->health_color = "";
    player->health_color_len = 0;
    // player->team_color = "";
    // player->team_color_len = 0;
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
    player->last_action_at_ms = 0;

    player->team = 0;
    player->et = ET_MINION;

    player->bot_action_delay_ms = 1e3;
    player->bot_willpower = 1;
    player->bot_schizophrenia = 1;

    player->bot_last_action_at_ms = 0;
}

void player_init(struct player *player, int team, enum entity_type entity_type, int connfd, struct sockaddr_in sock, int sock_len){
    player_init_mem(player);

    player->connfd = connfd;
    player->sock = sock;
    player->sock_len = sock_len;

    player->team = team;
    // if(player->team){
    //     static char team_color[] = EFFECT_NO_INVERSE_REVERSE;
    //     player->team_color       = team_color;
    //     player->team_color_len   = sizeof(team_color);
    // }else{
    //     static char team_color[] = EFFECT_INVERSE_REVERSE;
    //     player->team_color       = team_color;
    //     player->team_color_len   = sizeof(team_color);
    // }

    player->et = entity_type;
    switch(player->et){
        case ET_HERO_HUMAN:
            break;
        case ET_HERO_BOT:
        case ET_MINION:
        case ET_TOWER:
            player_init_bot(player);
            break;
    }

    player_select_hero(player);
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
            break;
    }

    // try to find a spot

    if(is_tower){

        int pos_ys[TOWERS_PER_TEAM] = {TOWER_SPAWN_Y, TOWER_SPAWN_Y,   TOWER_SPAWN_Y,   TOWER_SPAWN_Y*3, TOWER_SPAWN_Y*2, TOWER_SPAWN_Y*5, TOWER_SPAWN_Y*3};
        int pos_xs[TOWERS_PER_TEAM] = {TOWER_SPAWN_X, TOWER_SPAWN_X*3, TOWER_SPAWN_X*5, TOWER_SPAWN_X,   TOWER_SPAWN_X*2, TOWER_SPAWN_X,   TOWER_SPAWN_X*3};

        int y = 0;
        int x = 0;

        int empty_spot_found = 0;

        for(int idx=0; idx<TOWERS_PER_TEAM; ++idx){
            y = pos_ys[idx];
            x = pos_xs[idx];

            if(!player->team){
                y = (MAP_Y-1) - y;
                x = (MAP_X-1) - x;
            }

            if(map_is_tile_empty(players, y, x)){
                empty_spot_found = 1;
                break;
            }
        }

        if(!empty_spot_found){
            exit(ERR_COULD_NOT_SPAWN_TOWER);
        }

        player->y = y;
        player->x = x;
        
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

    // health

    player->hp = player->hero.hp_max;
    player_recalculate_health_state(player, players);

    // level

    int xp = player->xp;
    if(XP_IS_LOST_ON_DEATH){
        xp = 0;
    }

    int level = player->level - LEVELS_LOST_ON_DEATH;
    if(level < LEVEL_ON_SPAWN){
        level = LEVEL_ON_SPAWN;
    }

    player->xp = 0;
    player->level = 0;

    player_gain_xp(player, players, level * XP_FOR_LEVEL_UP + xp);
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

void player_select_action(struct player *player, struct player players[PLAYERS_MAX]){

    long long now = get_time_ms();

    if(player->last_action_at_ms + ACTION_INTERVAL_MS > now){
        return;
    }

    char action;

    switch(player->et){
        case ET_HERO_HUMAN:
            {
                int bytes = net_recv_1B(player->connfd, &action);
                if(bytes <= 0){
                    return;
                }
            }
            break;
        case ET_HERO_BOT:
        case ET_MINION:
        case ET_TOWER:
            {
                int skip = player_bot_select_action(player, players, &action);
                if(skip){
                    return;
                }
            }
            break;
    }

    int request_was_valid = player_process_action(player, action, players);
    if(!request_was_valid){
        return;
    }

    player->last_action_at_ms = now;
}

// returns 1 when player REQUESTED ANYTHING VALID, not when anything was actuallydone
static int player_process_action(struct player *player, char action, struct player players[PLAYERS_MAX]){

    if(!player->alive){
        return 0;
    }

    // movement

    int action_is_movement = 0;

    int x_desired = player->x;
    int y_desired = player->y;

    switch(action){
        case KEY_MOVE_LEFT:
            x_desired -= 1;
            action_is_movement = 1;
            break;
        case KEY_MOVE_RIGHT:
            x_desired += 1;
            action_is_movement = 1;
            break;
        case KEY_MOVE_UP:
            y_desired -= 1;
            action_is_movement = 1;
            break;
        case KEY_MOVE_DOWN:
            y_desired += 1;
            action_is_movement = 1;
            break;
    }

    if(rand() % player->hero.weight < player->hero.legpower){
        if(map_is_tile_empty(players, y_desired, x_desired)){
            screen_cur_set(players, player->y, player->x);
            screen_print_empty_tile(players);

            player->x = x_desired;
            player->y = y_desired;
            player_draw(player, players);
            return 1;
        }
    }

    if(action_is_movement){
        return 1;
    }

    // basic attack

    if((action == KEY_BASIC_ATTACK_1) || (action == KEY_BASIC_ATTACK_2)){
        player_basic_attack(player, players);
        return 1;
    }

    // heal ability

    if(action == KEY_HEAL_ABILITY){
        player_heal_ability(player, players);
        return 1;
    }

    // nothing

    return 0;
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
        int heal = (player->hero.heal_ability_amount * player->level) / LEVEL_ON_SPAWN;
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
        // if being damaged reduce damage based on level
        amount /= player->level;
    }

    player->hp -= amount;

    if(player->hp <= 0){

        // reward opposite team

        int team = !player->team;

        for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
            struct player *team_member = &players[player_idx];
            if(team_member->team != team){
                continue;
            }
            switch(team_member->et){
                case ET_HERO_HUMAN:
                case ET_HERO_BOT:
                    break;
                case ET_MINION:
                case ET_TOWER:
                    if(!MINIONS_AND_TOWERS_CAN_LEVEL_UP){
                        continue;
                    }
                    break;
            }
            player_gain_xp(team_member, players, KILL_REWARD_XP);
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

    static char health_state_palette[HEALTH_STATES*2][20]; // idx_0=healthy idx_last=lowhp // once for each team
    static int health_state_palette_generated = 0;

    if(!health_state_palette_generated){
        health_state_palette_generated = 1;

        for(int team=0; team<=1; ++team){
            int color_idx_ofs = HEALTH_STATES * team;

            for(int color_idx=0; color_idx < HEALTH_STATES; color_idx++){
                // indicate team
                int red = 0;
                int green = 0;
                int blue = 0;
                if(team){
                    red = 255;
                    green = (200 * color_idx) / HEALTH_STATES;
                    blue = 0;
                }else{
                    red = 0;
                    green = 65 + (190 * color_idx) / HEALTH_STATES;
                    blue = 255;
                }

                // draw
                int written = snprintf(health_state_palette[color_idx+color_idx_ofs], sizeof(health_state_palette[color_idx+color_idx_ofs]), "\033[38;2;%d;%d;%dm", red, green, blue);
                assert(written >= 0);
                assert((long unsigned int)written < sizeof(health_state_palette[color_idx+color_idx_ofs])); // buffer is too small
            }

        }
    }

    player->alive = player->hp > 0;

    char *old_color = player->health_color;

    int health_state_idx_reversed = ((HEALTH_STATES-1) * player->hp) / player->hero.hp_max;
    int health_state_idx = (HEALTH_STATES-1) - health_state_idx_reversed;

    assert(health_state_idx >= 0);
    assert(health_state_idx < HEALTH_STATES);

    player->health_color = health_state_palette[health_state_idx + (HEALTH_STATES*player->team)];
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
    while(player->xp >= XP_FOR_LEVEL_UP){
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

        // restore hp

        int restore_hp = 0;
        switch(player->et){
            case ET_HERO_HUMAN:
            case ET_HERO_BOT:
            case ET_MINION:
                restore_hp = 1;
                break;
            case ET_TOWER:
                restore_hp = TOWERS_RESTORE_HP_ON_LEVEL_UP;
                break;
        }

        if(restore_hp){
            int health_restored = (player->hp * LEVEL_UP_HEALTH_RESTORED_NUMERATOR) / LEVEL_UP_HEALTH_RESTORED_DENOMINATOR;
            player_receive_damage(player, -health_restored, players);
        }

        // draw player model

        player_draw(player, players); // TODO? draw too many times (since you can level up more than once)?
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

        // screen_print_single(player_receiver->connfd, player->team_color, player->team_color_len);

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
            static char player_is_self_color_off[] = EFFECT_NO_BOLD;
            screen_print_single(player_receiver->connfd, player_is_self_color_off, sizeof(player_is_self_color_off));
        }

        if(player->christmas_lights_on){
            // sample effect used for various shits
            static char christmas_lights_effect_off[] = EFFECT_NO_ITALIC;
            screen_print_single(player_receiver->connfd, christmas_lights_effect_off, sizeof(christmas_lights_effect_off));
        }
    }
}

void player_draw_ui(struct player *player){

    switch(player->et){
        case ET_HERO_HUMAN:
            break;
        case ET_HERO_BOT:
        case ET_MINION:
        case ET_TOWER:
            return;
    }

    int ui_y = MAP_Y + 1;

    // static variables for previous values (they have to be here so that we can check if we need to clear the screen for the hud color)

    // make sure those values are something unreachable so that the UI gets updated when this function gets run for the first time
    static int hp_before    = INT_MIN / 2;
    static int level_before = INT_MIN / 2;
    static int xp_before    = INT_MIN / 2;
    static int help_drawn_before = 0;

    int hp_updated = hp_before != player->hp;
    hp_before = player->hp;

    int level_updated = level_before != player->level;
    level_before = player->level;

    int xp_updated = xp_before != player->xp;
    xp_before = player->xp;

    int help_updated = !help_drawn_before;
    help_drawn_before = 1;

    int anything_updated = hp_updated || level_updated || xp_updated || help_updated;

    // set UI color

    if(anything_updated){
        char color[] = UI_COLOR;
        screen_cur_set_single(player->connfd, ui_y, 0);
        screen_print_single(player->connfd, color, sizeof(color));
    }

    // draw hp

    if(hp_updated){
        assert(player->hp          < 9999);
        assert(player->hero.hp_max < 9999);
        char msg[20];
        int written = snprintf(msg, sizeof(msg), "HP: %4d / %4d", player->hp, player->hero.hp_max);
        assert(written >= 0);
        assert((long unsigned int)written < sizeof(msg)); // buffer is too small

        screen_cur_set_single(player->connfd, ui_y, 0);
        screen_print_single(player->connfd, msg, written);
    }

    ui_y += 1;

    // draw level

    if(level_updated){
        assert(player->level < 99);
        char msg[10];
        int written = snprintf(msg, sizeof(msg), "Level: %02d", player->level);
        assert(written >= 0);
        assert((long unsigned int)written < sizeof(msg)); // buffer is too small

        screen_cur_set_single(player->connfd, ui_y, 0);
        screen_print_single(player->connfd, msg, written);
    }

    ui_y += 1;

    // draw xp

    if(xp_updated){
        assert(player->xp < 99);
        char msg[10];
        int written = snprintf(msg, sizeof(msg), "XP: %02d/%02d", player->xp, XP_FOR_LEVEL_UP);
        assert(written >= 0);
        assert((long unsigned int)written < sizeof(msg)); // buffer is too small

        screen_cur_set_single(player->connfd, ui_y, 0);
        screen_print_single(player->connfd, msg, written);
    }

    ui_y += 1;

    // draw help

    if(help_updated){
        screen_cur_set_single(player->connfd, ui_y, 0);

        char help_msg[180];
        int written = snprintf(help_msg, sizeof(help_msg),
            "--------------------\n\r"
            "Controls:\n\r"
            "%c - move up\n\r"
            "%c - move down\n\r"
            "%c - move left\n\r"
            "%c - move right\n\r"
            "%c - basic attack\n\r"
            "%c - basic attack\n\r"
            "%c - heal ability (not all heroes have one)\n\r"
            ,
            KEY_MOVE_UP, KEY_MOVE_DOWN, KEY_MOVE_LEFT, KEY_MOVE_RIGHT, KEY_BASIC_ATTACK_1, KEY_BASIC_ATTACK_2, KEY_HEAL_ABILITY
        );

        assert(written >= 0);
        assert((long unsigned int)written < sizeof(help_msg)); // buffer is too small

        screen_print_single(player->connfd, help_msg, written);
    }

    // ui_y += ?;

}

/////////////
///////////// bot stuff
/////////////

static int player_bot_select_action(struct player *player, struct player players[PLAYERS_MAX], char *action){

    // TODO teach bots how to retreat

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
        struct map_get_empty_tiles_near_return empty_tile = map_get_empty_tiles_near(players, player->y, player->x);

        char choices[4];
        int choices_len = 0;

        if(empty_tile.left){
            choices[choices_len++] = KEY_MOVE_LEFT;
        }
        if(empty_tile.right){
            choices[choices_len++] = KEY_MOVE_RIGHT;
        }
        if(empty_tile.up){
            choices[choices_len++] = KEY_MOVE_UP;
        }
        if(empty_tile.down){
            choices[choices_len++] = KEY_MOVE_DOWN;
        }

        if(choices_len <= 0){
            // no spece to move to
            return 1;
        }

        int choice = rand() % choices_len;

        *action = choices[choice];
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
            *action = KEY_BASIC_ATTACK_1;
            return 0;
        }
    }else if(heal_in_rage){
        *action = KEY_HEAL_ABILITY;
        return 0;
    }else if(attack_in_range){
        *action = KEY_BASIC_ATTACK_1;
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

    // move to closest target

    if(rand() % player->bot_human_wave_denomintor < player->bot_human_wave_numerator){ // if human wave
        enum direction direction = map_pathfind_depth_1(players, player->y, player->x, target->y, target->x);
        switch(direction){
            case D_NONE:
                return 1;
            case D_LEFT:
                *action = KEY_MOVE_LEFT;
                return 0;
            case D_RIGHT:
                *action = KEY_MOVE_RIGHT;
                return 0;
            case D_UP:
                *action = KEY_MOVE_UP;
                return 0;
            case D_DOWN:
                *action = KEY_MOVE_DOWN;
                return 0;
        }
        assert(0);

    }else{ // if encirclement // TODO this still does not take into account non-empty tiles

        char encirclement_action;

        if(abs(player->y - target->y) > abs(player->x - target->x)){
            if(player->x < target->x){
                encirclement_action = KEY_MOVE_RIGHT;
            }else{
                encirclement_action = KEY_MOVE_LEFT;
            }
        }else{
            if(player->y < target->y){
                encirclement_action = KEY_MOVE_DOWN;
            }else{
                encirclement_action = KEY_MOVE_UP;
            }
        }

        *action = encirclement_action;
        return 0;

    }

    assert(0);
}
