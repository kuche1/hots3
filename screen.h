
#ifndef SCREEN_H
#define SCREEN_H

#include "settings.h"
#include "player.h"

void screen_clear_single(int connfd);
void screen_clear(struct player players[PLAYERS_REQUIRED]);
void screen_cur_set_single(int connfd, int pos_y, int pos_x);
void screen_cur_set(struct player players[PLAYERS_REQUIRED], int pos_y, int pos_x);

#endif
