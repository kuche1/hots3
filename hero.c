
#include "hero.h"

#include <string.h>
#include <assert.h>

#include "screen.h"
#include "networking.h"

/////////////
///////////// initialisations
/////////////

void hero_init_mem(struct hero *hero){
    hero_init_regular_guy(hero);
}

void hero_select_player_hero(struct hero *hero, int connfd, int is_bot){

    char *choices[] = {
        "0: regular guy\n",
        "1: slower harder hitting guy\n",
        "2: guy with more range but less HP\n",
    };
    long unsigned int choices_len = sizeof(choices) / sizeof(*choices);

    char choice;


    if(is_bot){
        choice = rand() % choices_len;
        goto hero_selection_switch;
    }

    char msg_select_hero[] = "Select hero:\n";

back_to_the_start:
    assert(!is_bot);

    screen_print_single(connfd, msg_select_hero, sizeof(msg_select_hero)-1);

    for(long unsigned int choice_idx=0; choice_idx < choices_len; choice_idx++){
        char *choice = choices[choice_idx];

        screen_print_single(connfd, choice, strlen(choice));
    }

    int received = net_recv_1B(connfd, &choice);
    if(!received){
        choice = '0';
    }
    choice = choice - '0';

hero_selection_switch:

    switch(choice){
        case 0:
            hero_init_regular_guy(hero);
            break;
        case 1:
            hero_init_slower_harder_hitting_guy(hero);
            break;
        case 2:
            hero_init_guy_with_more_range_but_less_hp(hero);
            break;
        default:
            goto back_to_the_start;
    }

    char msg_done[] = "hero selected\n";
    screen_print_single(connfd, msg_done, sizeof(msg_done)-1);
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
