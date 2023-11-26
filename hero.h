
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
// drawing
void hero_draw_single(struct hero *hero, int connfd);
// other stuff
void hero_init_regular_guy(struct hero *hero);
void hero_init_slower_harder_hitting_guy(struct hero *hero);
void hero_init_guy_with_more_range_but_less_hp(struct hero *hero);

#endif
