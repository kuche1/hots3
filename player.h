
#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>

#include "networking.h"
#include "settings.h"
#include "errors.h"

struct player {
    // networking
    int connfd;
    struct sockaddr_in sock;
    unsigned int sock_len;

    // graphics
    char model;

    // location
    int x;
    int y;
};

void player_init_mem(struct player *player);
void player_init_telnet(struct player *player);
void player_spawn(struct player *player, struct player players[PLAYERS_REQUIRED]);
void player_draw(struct player *player, struct player players[PLAYERS_REQUIRED]);
void player_process_action(struct player *player, char action, struct player players[PLAYERS_REQUIRED]);

#endif
