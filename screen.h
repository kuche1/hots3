
#ifndef SCREEN_H
#define SCREEN_H

#include "settings.h"
#include "player.h"

void screen_clear_single(int connfd);
void screen_clear(struct player players[ENTITIES_MAX]);
void screen_switch_to_draw_mode_single(struct player * player);
void screen_switch_to_draw_mode(struct player players[ENTITIES_MAX]);
void screen_cur_set_single(int connfd, int pos_y, int pos_x);
void screen_cur_set(struct player players[ENTITIES_MAX], int pos_y, int pos_x);
void screen_print_single(int connfd, char * msg, int msg_len);
void screen_print(struct player players[ENTITIES_MAX], char * msg, int msg_len);
void screen_print_empty_tile(struct player players[ENTITIES_MAX]);

#endif
