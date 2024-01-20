
// TODO actually implement the ability to shoot

#ifndef BULLET_H
#define BULLET_H

struct bullet{
    // constants
    int team;
    long long move_interval;
    int dmg;
    int delta_x; // describes where the bullet is going to go
    int delta_y;

    // variables
    long long last_move_at_ms;
    int y;
    int x;
};

void bullet_init(
    struct bullet *bullet,
    int team, long long move_interval, int dmg, int delta_y, int delta_x,
    int y, int x
);

#endif
