
#include "hero.h"

#include <string.h>
#include <assert.h>

#include "screen.h"
#include "networking.h"

#define NUMBER_OF_HEROES 4

/////////////
///////////// initialisations
/////////////

void hero_init_mem(struct hero *hero){
    hero_init_regular_guy(hero);
}

void hero_select_player_hero(struct hero *hero, int connfd, int is_bot){

    char *choices_str[NUMBER_OF_HEROES] = {
        "basic bitch\n",
        "varian  -           ; slower ; higher dmg ;             \n",
        "valla   - lower  hp ;        ;            ; higher range\n",
        "stiches - higher hp ; slower ;            ;             \n",
    };

    void (*choices_fnc[NUMBER_OF_HEROES])(struct hero *) = {
        hero_init_regular_guy,
        hero_init_varian,
        hero_init_valla,
        hero_init_stiches,
    };

    if(is_bot){
        void (*choice)(struct hero *) = choices_fnc[rand() % NUMBER_OF_HEROES];
        choice(hero);
        return;
    }

    char msg_select_hero[] = "\nSelect hero and press enter:\n";

    while(1){

        screen_print_single(connfd, msg_select_hero, sizeof(msg_select_hero)-1);

        for(long unsigned int choice_idx=0; choice_idx < NUMBER_OF_HEROES; choice_idx++){
            char *choice = choices_str[choice_idx];

            char choice_num_as_char = choice_idx + '0';
            screen_print_single(connfd, &choice_num_as_char, sizeof(choice_num_as_char));
            screen_print_single(connfd, ": ", 2);
            screen_print_single(connfd, choice, strlen(choice));
        }

        char choice;
        int received = net_recv_1B(connfd, &choice);
        if(!received){
            choice = '0';
        }
        choice = choice - '0';

        if((choice < 0) || (choice >= NUMBER_OF_HEROES)){
            char msg_bad_choice[] = "invalid choice\n";
            screen_print_single(connfd, msg_bad_choice, sizeof(msg_bad_choice)-1);
            continue;
        }

        int choice_int = choice;

        void (*hero_init)(struct hero *) = choices_fnc[choice_int];
        hero_init(hero);   

        break;
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
///////////// heroes
/////////////

void hero_init_regular_guy(struct hero *hero){
    hero->model = 'B';

    hero->hp_max = 200;
    hero->basic_attack_distance = 1;
    hero->basic_attack_damage = 2;

    hero->legpower = 1;
    hero->weight   = 1;
}

void hero_init_varian(struct hero *hero){
    hero->model = 'V';

    hero->basic_attack_damage = 3;

    hero->legpower = 5;
    hero->weight   = 7;
}

void hero_init_valla(struct hero *hero){
    hero->model = 'v';

    hero->hp_max = 120;
    hero->basic_attack_distance = 2;
}

void hero_init_stiches(struct hero *hero){
    hero->model = 'S';

    hero->hp_max = 310;

    hero->legpower = 3;
    hero->weight   = 7;
}

// TODO fast an annoying
// also it would be best if we move the character selection screen here
