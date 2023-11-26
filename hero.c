
#include "hero.h"

void hero_init_mem(struct hero *hero){
    hero->hp_max = 100;
    hero->basic_attack_distance = 1;
    hero->basic_attack_damage = 1;
}
