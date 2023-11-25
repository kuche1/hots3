
#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>
#include <netdb.h>

#include "settings.h"
#include "errors.h"

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

    // character stats
    int basic_attack_distance;
    int basic_attack_damage;
};

void player_init_mem(struct player *player);
void player_init_telnet(struct player *player);
void player_spawn(struct player *player, struct player players[PLAYERS_REQUIRED]);
void player_draw(struct player *player, struct player players[PLAYERS_REQUIRED]);
void player_process_action(struct player *player, char action, struct player players[PLAYERS_REQUIRED]);
void player_basic_attack(struct player *player, struct player players[PLAYERS_REQUIRED]);
void player_receive_damage(struct player *player, int amount);

#endif
