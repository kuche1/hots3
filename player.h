
#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>
#include <netdb.h>

#include "settings.h"
#include "errors.h"

static char STATIC_col_green[]  __attribute__((unused)) = {'\033', '[', '9', '2', 'm'};
static char STATIC_col_yellow[] __attribute__((unused)) = {'\033', '[', '9', '3', 'm'};
static char STATIC_col_red[]    __attribute__((unused)) = {'\033', '[', '9', '1', 'm'};

static char STATIC_map_tile_empty[] __attribute__((unused)) = {' '};

struct player {
    // networking
    int connfd;
    struct sockaddr_in sock;
    unsigned int sock_len;

    // graphics
    char model;

    // variable data
    int x;
    int y;
    int hp;
    int health_state;
    char *health_color;
    int health_color_len;
    int alive;

    // character stats
    int basic_attack_distance;
    int basic_attack_damage;
    int hp_max;

    // bot data
    int bot;
};

void player_init_mem(struct player *player);
void player_init_telnet(struct player *player);
void player_init_bot(struct player *player);
void player_spawn(struct player *player, struct player players[PLAYERS_REQUIRED]);
void player_draw(struct player *player, struct player players[PLAYERS_REQUIRED]);
int player_bot_select_action(struct player *player, struct player players[PLAYERS_REQUIRED], char *action);
void player_process_action(struct player *player, char action, struct player players[PLAYERS_REQUIRED]);
void player_basic_attack(struct player *player, struct player players[PLAYERS_REQUIRED]);
void player_receive_damage(struct player *player, int amount, struct player players[PLAYERS_REQUIRED]);

#endif
