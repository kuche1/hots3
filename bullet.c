
#include "bullet.h"

#include <stdio.h>

void bullet_init(
    struct bullet *bullet,
    int team, long long move_interval, int dmg, int delta_y, int delta_x,
    int y, int x
){

    bullet->team = team;
    bullet->move_interval = move_interval;
    bullet->dmg = dmg;
    bullet->delta_x = delta_x;
    bullet->delta_y = delta_y;

    bullet->last_move_at_ms = 0; // this could either be 0 or the current time
    bullet->y = y;
    bullet->x = x;
}

void bullet_process(
    struct bullet *bullet
){
    // TODO
    printf("%p", (void *)bullet);
}
