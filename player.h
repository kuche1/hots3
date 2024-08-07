
#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>
#include <netdb.h>

#include "settings.h"
#include "errors.h"
#include "hero.h"
#include "entity_type.h"

struct player {
    // networking
    int connfd;
    struct sockaddr_in sock;
    unsigned int sock_len;

    // graphics
    char * health_color;
    int health_color_len;
    int christmas_lights_on;
    int spawn_effect_on;

    // selected character stats
    struct hero hero;

    // variable data
    int x;
    int y;
    int hp;
    int alive;
    int level;
    int xp;
    long long died_at_ms;

    // action limit
    // every "tick" the time passed gets stored, then it can get used by the player to execute a certain action
    long long action_last_tick_at_ms;
    long long action_storage_ms;

    // UI
    int ui_hp;
    long long ui_action;
    int ui_level;
    int ui_xp;
    int ui_help;

    // other
    int team;
    enum entity_type et;

    // bot data
    long long bot_action_delay_ms;
    int bot_willpower;
    int bot_schizophrenia;
    int bot_pathfind_depth;

    // bot variable data
    long long bot_last_action_at_ms;
};

// initialising
void player_init_mem(struct player * player);
void player_init(struct player * player, int team, enum entity_type entity_type, int connfd, struct sockaddr_in sock, int sock_len);
void player_select_hero(struct player * player);
void player_spawn(struct player * player, struct player players[ENTITIES_MAX], int pos_y, int pos_x);
// checks
int player_your_schizophrenia_is_trolling_you(struct player * player);
// actions
int player_move_to(struct player * player, int y_desired, int x_desired, struct player players[ENTITIES_MAX]);
void player_select_action(struct player * player, struct player players[ENTITIES_MAX]);
void player_basic_attack_an_entity(struct player * player, struct player * target, struct player entities[ENTITIES_MAX]);
int player_basic_attack(struct player * player, struct player players[ENTITIES_MAX]);
int player_heal_ability(struct player * player, struct player players[ENTITIES_MAX]);
void player_kill_yourself(struct player * player, struct player players[ENTITIES_MAX]);
// deal with status
int player_receive_damage(struct player * player, int amount, struct player players[ENTITIES_MAX]);
void player_recalculate_health_state(struct player * player, struct player players[ENTITIES_MAX]);
void player_gain_xp(struct player * player, struct player players[ENTITIES_MAX], int xp);
// drawing
void player_toggle_christmas_lights(struct player * player, struct player players[ENTITIES_MAX]);
void player_draw(struct player * player, struct player players[ENTITIES_MAX]);
void player_draw_ui(struct player * player);

#endif
