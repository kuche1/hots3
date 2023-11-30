
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
    char *health_color;
    int health_color_len;
    char *team_color;
    int team_color_len;
    char *level_color;
    int level_color_len;
    int christmas_lights_on;

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

    // other
    int team;
    enum entity_type et;

    // bot data
    long long bot_action_delay_ms;
    int bot_willpower;
    int bot_schizophrenia;
    int bot_human_wave_numerator;
    int bot_human_wave_denomintor;

    // bot variable data
    long long bot_last_action_at_ms;
};

// initialising
void player_init_mem(struct player *player);
void player_init(struct player *player, int team, enum entity_type entity_type, int connfd, struct sockaddr_in sock, int sock_len);
void player_spawn(struct player *player, struct player players[PLAYERS_MAX]);
void player_select_hero(struct player *player);
// actions
void player_process_action(struct player *player, char action, struct player players[PLAYERS_MAX]);
void player_basic_attack(struct player *player, struct player players[PLAYERS_MAX]);
void player_heal_ability(struct player *player, struct player players[PLAYERS_MAX]);
// deal with status
void player_receive_damage(struct player *player, int amount, struct player players[PLAYERS_MAX]);
void player_recalculate_health_state(struct player *player, struct player players[PLAYERS_MAX]);
void player_gain_xp(struct player *player, struct player players[PLAYERS_MAX], int xp);
// drawing
void player_toggle_christmas_lights(struct player *player, struct player players[PLAYERS_MAX]);
void player_draw(struct player *player, struct player players[PLAYERS_MAX]);
// bot stuff
int player_bot_select_action(struct player *player, struct player players[PLAYERS_MAX], char *action);

#endif
