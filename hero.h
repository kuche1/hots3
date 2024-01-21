
#ifndef HERO_H
#define HERO_H

#include "entity_type.h"

struct hero{
    // graphics
    char model;

    // hp
    int hp_max;

    // basic attacks
    int basic_attack_distance;
    int basic_attack_damage;

    // movement - the chance for the move action to work will be `legpower / weight`
    int legpower;
    int weight;

    // healing ability // TODO? make separate ability for healing towers
    int heal_ability_range;
    int heal_ability_amount;

    // context-dependent data (example: determine bullet travel direction)
    int context;
};

// initialisations
void hero_init_mem(struct hero *hero);
void hero_select_player_hero(struct hero *hero, int connfd, enum entity_type entity_type);
// drawing
void hero_draw_single(struct hero *hero, int connfd);
// heroes
void hero_init_regular_guy(struct hero *hero);
void hero_init_varian(struct hero *hero);
void hero_init_valla(struct hero *hero);
void hero_init_stiches(struct hero *hero);
void hero_init_lili(struct hero *hero);
void hero_init_alarak(struct hero *hero);
// special
void hero_init_minion(struct hero *hero);
void hero_init_tower(struct hero *hero);
void hero_init_wall(struct hero *hero);
void hero_init_bullet(struct hero *hero);

#endif
