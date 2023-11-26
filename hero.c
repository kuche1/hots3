
#include "hero.h"

#include "screen.h"

/////////////
///////////// memory
/////////////

void hero_init_mem(struct hero *hero){
    hero_init_regular_guy(hero);
}

/////////////
///////////// drawing
/////////////

void hero_draw_single(struct hero *hero, int connfd){
    screen_print_single(connfd, &hero->model, sizeof(hero->model));
}

/////////////
///////////// other stuff
/////////////

void hero_init_regular_guy(struct hero *hero){
    hero->model = 'B';

    hero->hp_max = 100;
    hero->basic_attack_distance = 1;
    hero->basic_attack_damage = 1;

    hero->legpower = 1;
    hero->weight   = 1;
}

void hero_init_slower_harder_hitting_guy(struct hero *hero){
    hero->model = 'F';

    hero->basic_attack_damage = 2;

    hero->legpower = 4;
    hero->weight   = 7;
}

void hero_init_guy_with_more_range_but_less_hp(struct hero *hero){
    hero->model = 'R';

    hero->hp_max = 60;
    hero->basic_attack_distance = 2;
}

// TODO fast an annoying
// also it would be best if we move the character selection screen here
