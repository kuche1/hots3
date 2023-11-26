
#ifndef HERO_H
#define HERO_H

struct hero{
    char model;

    int basic_attack_distance;
    int basic_attack_damage;
    int hp_max;

    int legpower;
    int weight;
    // the change for the move action to work will be `legpower / weight`
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

#endif
