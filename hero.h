
#ifndef HERO_H
#define HERO_H

struct hero{
    int basic_attack_distance;
    int basic_attack_damage;
    int hp_max;
    int weight; // TODO use this to determine how many times you have to press a button to move to a direction
};

void hero_init_mem(struct hero *hero);

#endif
