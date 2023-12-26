
#include "hero.h"

#include <string.h>
#include <assert.h>

#include "screen.h"
#include "networking.h"

#define NUMBER_OF_HEROES 6

/////////////
///////////// initialisations
/////////////

void hero_init_mem(struct hero *hero){
    hero_init_regular_guy(hero);
}

void hero_select_player_hero(struct hero *hero, int connfd, enum entity_type entity_type){

    char *choices_str[NUMBER_OF_HEROES] = {
        "basic bitch\n\r\tnothing special\n\r",
        "varian\n\r\tslower\n\r\thigher dmg\n\r",
        "valla\n\r\tlower hp\n\r\thigher range\n\r",
        "stiches\n\r\thigher hp\n\r\tslower\n\r",
        "lili\n\r\tlower hp\n\r\tlower damage\n\r\tcan heal\n\r",
        // "minion\n\r\tpiece of shit stats\n\r\tgood for mindgames\n\r",
        "alarak\n\r\tlower hp\n\r\thigher damage\n\r"
    };

    void (*choices_fnc[NUMBER_OF_HEROES])(struct hero *) = {
        hero_init_regular_guy,
        hero_init_varian,
        hero_init_valla,
        hero_init_stiches,
        hero_init_lili,
        // hero_init_minion,
        hero_init_alarak,
    };

    switch(entity_type){
        case ET_HERO_HUMAN:
            break;

        case ET_HERO_BOT:
            {
                void (*choice)(struct hero *) = choices_fnc[rand() % NUMBER_OF_HEROES];
                choice(hero);
            }
            return;

        case ET_MINION:
            hero_init_minion(hero);
            return;
        
        case ET_TOWER:
            hero_init_tower(hero);
            return;
        
        case ET_WALL:
            hero_init_wall(hero);
            return;
    }

    char msg_select_hero[] = "\nSelect a hero and press enter:\n";

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

    // PRINT CAT HERE

    char msg_done[] = "hero selected\nwaiting for other players to ready up...\n";
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

    hero->hp_max = 1120;

    hero->basic_attack_distance = 1;
    hero->basic_attack_damage = 12;

    hero->heal_ability_range = 0;
    hero->heal_ability_amount = 0;

    hero->legpower = 1;
    hero->weight   = 1;
}

void hero_init_varian(struct hero *hero){
    hero->model = 'V';

    hero->basic_attack_damage = (hero->basic_attack_damage * 13) / 10;

    hero->legpower = 6;
    hero->weight   = 7;
}

void hero_init_valla(struct hero *hero){
    hero->model = 'v';

    hero->hp_max = (hero->hp_max * 8) / 10;

    hero->basic_attack_distance += 1;
}

void hero_init_stiches(struct hero *hero){
    hero->model = 'S';

    hero->hp_max = (hero->hp_max * 13) / 10;

    hero->legpower = 4;
    hero->weight   = 7;
}

void hero_init_lili(struct hero *hero){
    int base_attack = hero->basic_attack_damage;

    hero->model = 'l';

    hero->hp_max = (hero->hp_max * 8) / 10;

    hero->basic_attack_damage = (base_attack * 4) / 10;

    hero->heal_ability_range = 2;
    hero->heal_ability_amount = (base_attack * 6) / 10;
}

void hero_init_alarak(struct hero *hero){
    hero->model = 'A';

    hero->hp_max = (hero->hp_max * 8) / 10;

    hero->basic_attack_damage = (hero->basic_attack_damage * 12) / 10;
}

/////////////
///////////// special
/////////////

void hero_init_minion(struct hero *hero){
    hero->model = 'm';

    hero->hp_max = hero->hp_max / 10;

    hero->basic_attack_damage = hero->basic_attack_damage / 3;

    hero->legpower =  7;
    hero->weight   = 10;
}

void hero_init_tower(struct hero *hero){
    int base_attack = hero->basic_attack_damage;

    hero->model = 'T';

    hero->hp_max = (hero->hp_max * 30) / 10;

    hero->basic_attack_distance += 6;
    hero->basic_attack_damage = (base_attack * 30) / 10;

    hero->heal_ability_range = 1;
    hero->heal_ability_amount = (base_attack * 2) / 3;

    hero->legpower = 0;
    hero->weight = 1;
}

void hero_init_wall(struct hero *hero){
    hero->model = 'W';

    hero->hp_max = (hero->hp_max * 15) / 10;

    hero->basic_attack_distance = 0;
    hero->basic_attack_damage = 0;

    hero->legpower = 0;
    hero->weight = 1;
}
