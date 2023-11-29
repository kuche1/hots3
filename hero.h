
#ifndef HERO_H
#define HERO_H

struct hero{
    // graphics
    char model;

    // hp
    int hp_max;

    // basic attacks
    int basic_attack_distance;
    int basic_attack_damage;

    // movement - the change for the move action to work will be `legpower / weight`
    int legpower;
    int weight;

    // healing ability
    int heal_ability_range;
    int heal_ability_amount;
};

// initialisations
void hero_init_mem(struct hero *hero);
void hero_select_player_hero(struct hero *hero, int connfd, int is_bot);
// drawing
void hero_draw_single(struct hero *hero, int connfd);
// heroes
void hero_init_regular_guy(struct hero *hero);
void hero_init_varian(struct hero *hero);
void hero_init_valla(struct hero *hero);
void hero_init_stiches(struct hero *hero);
void hero_init_lili(struct hero *hero);
// special
void hero_init_minion(struct hero *hero);
void hero_init_tower(struct hero *hero);

#endif
