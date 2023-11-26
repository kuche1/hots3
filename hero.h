
#ifndef HERO_H
#define HERO_H

struct hero{
    int basic_attack_distance;
    int basic_attack_damage;
    int hp_max;

    int legpower;
    int weight;
    // the change for the move action to work will be `legpower / weight`
};

void hero_init_mem(struct hero *hero);

void hero_init_regular_guy(struct hero *hero);
void hero_init_slower_harder_hitting_guy(struct hero *hero);
void hero_init_gut_with_more_range_but_less_hp(struct hero *hero);

#endif
