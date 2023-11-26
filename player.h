
#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>
#include <netdb.h>

#include "settings.h"
#include "errors.h"
#include "hero.h"

// foreground colors
// https://ss64.com/nt/syntax-ansi.html
static char STATIC_col_green_bright[]  __attribute__((unused)) = {'\033', '[', '9', '2', 'm'};
static char STATIC_col_green_dark[]    __attribute__((unused)) = {'\033', '[', '3', '2', 'm'};
static char STATIC_col_yellow_bright[] __attribute__((unused)) = {'\033', '[', '9', '3', 'm'};
static char STATIC_col_yellow_dark[]   __attribute__((unused)) = {'\033', '[', '3', '3', 'm'};
static char STATIC_col_red_bright[]    __attribute__((unused)) = {'\033', '[', '9', '1', 'm'};
static char STATIC_col_red_dark[]      __attribute__((unused)) = {'\033', '[', '3', '1', 'm'};

static char STATIC_map_tile_empty[] __attribute__((unused)) = {' '};

struct player {
    // networking
    int connfd;
    struct sockaddr_in sock;
    unsigned int sock_len;

    // graphics
    char *health_color;
    int health_color_len;
    // TODO add team color (can be the background color)

    // variable data
    int x;
    int y;
    int hp;
    int health_state;
    int alive;

    // selected character stats
    struct hero hero;

    // bot data
    int bot;
    long long bot_action_delay_ms;

    // bot variable data
    long long bot_last_action_at_ms;
};

// initialising
void player_init_mem(struct player *player);
void player_init_telnet(struct player *player);
void player_init_bot(struct player *player);
void player_spawn(struct player *player, struct player players[PLAYERS_REQUIRED]);
void player_select_hero(struct player *player);
// actions
void player_process_action(struct player *player, char action, struct player players[PLAYERS_REQUIRED]);
void player_basic_attack(struct player *player, struct player players[PLAYERS_REQUIRED]);
void player_receive_damage(struct player *player, int amount, struct player players[PLAYERS_REQUIRED]);
void player_recalculate_health_state(struct player *player, struct player players[PLAYERS_REQUIRED]);
// bot stuff
int player_bot_select_action(struct player *player, struct player players[PLAYERS_REQUIRED], char *action);
// other stuff
void player_draw(struct player *player, struct player players[PLAYERS_REQUIRED]);

#endif
