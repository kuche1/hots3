
#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>
#include <netdb.h>

#include "settings.h"
#include "errors.h"
#include "hero.h"

// https://ss64.com/nt/syntax-ansi.html
// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
// foreground colors
static char STATIC_col_green_bright[]   __attribute__((unused)) = {'\033', '[', '9', '2', 'm'};
static char STATIC_col_green_dark[]     __attribute__((unused)) = {'\033', '[', '3', '2', 'm'};
static char STATIC_col_yellow_bright[]  __attribute__((unused)) = {'\033', '[', '9', '3', 'm'};
static char STATIC_col_yellow_dark[]    __attribute__((unused)) = {'\033', '[', '3', '3', 'm'};
static char STATIC_col_magenta_bright[] __attribute__((unused)) = {'\033', '[', '9', '5', 'm'};
static char STATIC_col_magenta_dark[]   __attribute__((unused)) = {'\033', '[', '3', '5', 'm'};
static char STATIC_col_red_bright[]     __attribute__((unused)) = {'\033', '[', '9', '1', 'm'};
static char STATIC_col_red_dark[]       __attribute__((unused)) = {'\033', '[', '3', '1', 'm'};
// background colors
static char STATIC_col_bg_red_dark[]      __attribute__((unused)) = {'\033', '[', '4', '1',      'm'};
static char STATIC_col_bg_blue_dark[]     __attribute__((unused)) = {'\033', '[', '4', '4',      'm'};
static char STATIC_col_bg_white_dark[]    __attribute__((unused)) = {'\033', '[', '4', '6',      'm'};
static char STATIC_col_bg_white_bright[]  __attribute__((unused)) = {'\033', '[', '1', '0', '7', 'm'};
static char STATIC_col_bg_magenta_dark[]  __attribute__((unused)) = {'\033', '[', '4', '5',      'm'};
static char STATIC_col_bg_black_dark[]    __attribute__((unused)) = {'\033', '[', '4', '0',      'm'};
// effects
static char STATIC_effect_underline[]    __attribute__((unused)) = {'\033', '[', '4', 'm'};
static char STATIC_effect_no_underline[] __attribute__((unused)) = {'\033', '[', '2', '4', 'm'};

static char STATIC_map_tile_empty[] __attribute__((unused)) = {'\033', '[', '0', 'm', ' '};

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

    // selected character stats
    struct hero hero;

    // variable data
    int x;
    int y;
    int hp;
    int alive;

    // TODO add levels

    // other
    int team;

    // bot data
    int bot;
    long long bot_action_delay_ms;

    // bot variable data
    long long bot_last_action_at_ms;
};

// initialising
void player_init_mem(struct player *player);
void player_init(struct player *player, int team, int is_bot, int connfd, struct sockaddr_in sock, int sock_len);
void player_init_telnet(struct player *player);
void player_spawn(struct player *player, struct player players[PLAYERS_MAX]);
void player_select_hero(struct player *player, int is_minion);
// actions
void player_process_action(struct player *player, char action, struct player players[PLAYERS_MAX]);
void player_basic_attack(struct player *player, struct player players[PLAYERS_MAX]);
void player_heal_ability(struct player *player, struct player players[PLAYERS_MAX]);
void player_receive_damage(struct player *player, int amount, struct player players[PLAYERS_MAX]);
void player_recalculate_health_state(struct player *player, struct player players[PLAYERS_MAX]);
// bot stuff
int player_bot_select_action(struct player *player, struct player players[PLAYERS_MAX], char *action);
// other stuff
void player_draw(struct player *player, struct player players[PLAYERS_MAX]);

#endif
