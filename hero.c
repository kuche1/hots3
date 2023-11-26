
#include "hero.h"

void hero_init_mem(struct hero *hero){
    hero_init_regular_guy(hero);
}

void hero_init_regular_guy(struct hero *hero __attribute__((unused))){
    hero->hp_max = 100;
    hero->basic_attack_distance = 1;
    hero->basic_attack_damage = 1;

    hero->legpower = 1;
    hero->weight   = 1;
}

void hero_init_slower_harder_hitting_guy(struct hero *hero){
    hero->basic_attack_damage = 2;

    hero->legpower = 4;
    hero->weight   = 7;
}

void hero_init_gut_with_more_range_but_less_hp(struct hero *hero){
    hero->hp_max = 60;
    hero->basic_attack_distance = 2;
}
