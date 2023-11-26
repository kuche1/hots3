
#ifndef HERO_H
#define HERO_H

struct hero{
    int basic_attack_distance;
    int basic_attack_damage;
    int hp_max;
    int weight; // TODO use this to determine how many times you have to press a button to move to a direction
};

void hero_init_mem(struct hero *hero);

void hero_init_regular_guy(struct hero *hero);
void hero_init_slower_harder_hitting_guy(struct hero *hero);
void hero_init_gut_with_more_range_but_less_hp(struct hero *hero);

#endif
