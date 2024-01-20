
// TODO requested by clients
//
// [2023-12-05] nov geroi: stg hammer (moje da dobavq ne6to kato toggle_christmas_lights za da opi6a dali e kleknala)

// TODO important bugs

// TODO
//
// da napravq botovete da polzvat po-adekvatna funkciq za namiraneto na distanciqta do target-a
// da razdelq funkciqta za namirane na distanciq na 2 - 1 za raw i 1 za pathfinding
//
// da napravq botovete da retreat-vat
// workaround dokato go implementna tova e vseki otbor da ima po 1 lili (possibly 2)
//
// da dobavq steni
//
// kogato nqkoi connect-ne kum survura go pita kolko bota iska, koi map, ...

// TODO low priority or old
//
// local multiplayer
//
// hotjoin ako ima botove?
//
// da napi6a v hud-a sredniq level na 2ta otbora
//
// ?auto attack (but what about auto move)
//
// ?da display-va respawn timer
//
// da napravq da moje character-a ti da e UTF-8 character (i da proverqvam dali naistna e UTF-8 ili nqkolko ASCII char-a)
//
// add cat pic from https://github.com/kuche1/hots3/pull/2 (we can convert the image to ASCII characters and send them to the client)
//
// only apply level effect if certain amount of levels above average
//
// pole za chat (za sega moje i da e samo 1 red)
//
// add vision and use the dim effect for whatever you last saw

// TODO new features
//
// teach bots how to retreat
//
// player models that are not just a single character
// this is going to be difficult for utf-8 characters as no monospace font coveres them all and some of them will fallback to a non-monospace font
//
// [done] convert images to ascii characters and send them to clients (can use `ascii-image-converter --color --complex img.jpg > img-as-text`)
// [not done] you can later use this to create characters icons (and possibly animations) to use in the character selection screen and in-game
// [not done] also try swapping the background and foreground colors (the result seems to look much better)
//
// show character model in character selection screen
//
// add projectiles (that move, static, that auto aim)
//
// possibility to change maps

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "errors.h"
#include "networking.h"
#include "player.h"
#include "settings.h"
#include "screen.h"
#include "util.h"
#include "map.h"

int main(int argc, char **argv __attribute__((unused))){

    // command line args

    if(argc != 1){
        printf("You need to specify exactly 0 arguments\n");
        return ERR_BAD_COMMAND_LINE_ARGS;
    }

    // init socket

    int sockfd;
    struct sockaddr_in servaddr;

    {
        printf("trying to start server on port %d\n", PORT);
        int err;
        if((err = create_server(&sockfd, &servaddr, PORT, LISTEN))){
            return err;
        }
    }

    // init players mem

    struct player players[PLAYERS_MAX];
    int players_len = 0;
    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *player = &players[player_idx];
        player_init_mem(player);
    }

    // lobby

    char setting_map_name[512] = {0};

    {

        int number_of_bot_players = -1;

        int team = 0;

        assert(HEROES_REQUIRED <= PLAYERS_MAX);
        while(players_len < HEROES_REQUIRED){

            enum entity_type et = ET_HERO_HUMAN;
            int connfd = -1;
            struct sockaddr_in sock;
            unsigned int sock_len = sizeof(sock);

            printf("established connections %d of %d\n", players_len, HEROES_REQUIRED);
        
            if(players_len < number_of_bot_players){
                et = ET_HERO_BOT;
                printf("bot connected\n");
            }else{
                connfd = accept(sockfd, (struct sockaddr *) &sock, &sock_len);
                if(connfd < 0){
                    printf("player could not connect\n");
                    continue;
                }
                printf("player connected\n");

                // get settings

                if(number_of_bot_players < 0){
                    char msg_enter_number_of_bots[80];
                    int written = snprintf(msg_enter_number_of_bots, sizeof(msg_enter_number_of_bots), "number of heroes required: %d\nenter number of bots (1 char) (example: 7): ", HEROES_REQUIRED);
                    assert(written >= 0);
                    assert((long unsigned int)written < sizeof(msg_enter_number_of_bots)); // buffer is too small
                    net_send_single(connfd, msg_enter_number_of_bots, sizeof(msg_enter_number_of_bots));

                    char choice_char;
                    int received = net_recv_1B(connfd, &choice_char);
                    assert(received == 1);
                    int choice_int = choice_char - '0';
                    assert(choice_int >= 0);
                    assert(choice_int <= 9);

                    number_of_bot_players = choice_int;
                    number_of_bot_players += 1; // compensate for the current connection
                }

                if(!strcmp(setting_map_name, "")){ // strings are equal

                    // I don't show any of the maps IDs and I use 1 char for IDs cuz I'm lazy like that

                    for(;;){

                        char msg[] = "enter ID of map (example: 0): ";
                        net_send_single(connfd, msg, sizeof(msg));

                        char map_id;
                        int received = net_recv_1B(connfd, &map_id);
                        assert(received == 1);

                        char full_name[] = {0, 0};
                        full_name[0] = map_id;
                        full_name[1] = 0;

                        if(!map_custom_map_exists(full_name)){
                            char msg[] = "map does not exist\n";
                            net_send_single(connfd, msg, sizeof(msg));
                            continue;
                        }

                        assert(sizeof(setting_map_name) >= sizeof(full_name));
                        memcpy(setting_map_name, full_name, sizeof(full_name));

                        break;
                    }
                }
            }

            printf("initialising player...\n");
            struct player *player = &players[players_len];
            player_init(player, team, et, connfd, sock, sock_len);
            printf("init done\n");

            team = !team;
            players_len += 1;
        }

    }

    // change to draw mode

    screen_switch_to_draw_mode(players);

    // clear screen

    screen_clear(players);

    // draw map borders

    screen_cur_set(players, MAP_Y, 0);
    for(int x=0; x<MAP_X; ++x){
        screen_print(players, "-", 1);
    }

    for(int y=0; y<MAP_Y; ++y){
        screen_cur_set(players, y, MAP_X);
        screen_print(players, "|", 1);
    }

    // // load custom map

    // {
    //     int walls_x[] = {
    //         2,
    //     };

    //     int walls_y[] = {
    //         2,
    //     };

    //     int walls_team[] = {
    //         0,
    //     };

    //     int walls_x_len    = sizeof(walls_x) / sizeof(*walls_x);
    //     int walls_y_len    = sizeof(walls_y) / sizeof(*walls_y);
    //     int walls_team_len = sizeof(walls_team) / sizeof(*walls_team);

    //     map_load(
    //         walls_x, walls_x_len,
    //         walls_y, walls_y_len,
    //         walls_team, walls_team_len,
    //         players
    //     );
    // }

    {
        FILE *f_walls_x    = NULL;
        FILE *f_walls_y    = NULL;
        FILE *f_walls_team = NULL;

        // I'm savage

        {
            char path[512];
            int written = snprintf(path, sizeof(path), "maps/%s/walls_x", setting_map_name);
            assert(written >= 0);
            assert((long unsigned int)written < sizeof(path)); // buffer is too small

            f_walls_x = fopen(path, "rb");
            if(!f_walls_x){
                exit(ERR_CANT_OPEN_FILE_CONTAINING_CUSTOM_MAP_DATA);
            }
        }

        {
            char path[512];
            int written = snprintf(path, sizeof(path), "maps/%s/walls_y", setting_map_name);
            assert(written >= 0);
            assert((long unsigned int)written < sizeof(path)); // buffer is too small

            f_walls_y = fopen(path, "rb");
            if(!f_walls_y){
                exit(ERR_CANT_OPEN_FILE_CONTAINING_CUSTOM_MAP_DATA);
            }
        }

        {
            char path[512];
            int written = snprintf(path, sizeof(path), "maps/%s/walls_team", setting_map_name);
            assert(written >= 0);
            assert((long unsigned int)written < sizeof(path)); // buffer is too small

            f_walls_team = fopen(path, "rb");
            if(!f_walls_team){
                exit(ERR_CANT_OPEN_FILE_CONTAINING_CUSTOM_MAP_DATA);
            }
        }

        {
            int32_t walls_x[MAX_NUMBER_OF_WALLS_FOR_CUSTOM_MAPS];
            int walls_x_len = 0;
            read_file_into_buffer(f_walls_x, (char *)walls_x, &walls_x_len, LENOF(walls_x), sizeof(*walls_x));

            int32_t walls_y[MAX_NUMBER_OF_WALLS_FOR_CUSTOM_MAPS];
            int walls_y_len = 0;
            read_file_into_buffer(f_walls_y, (char *)walls_y, &walls_y_len, LENOF(walls_y), sizeof(*walls_y));

            int32_t walls_team[MAX_NUMBER_OF_WALLS_FOR_CUSTOM_MAPS];
            int walls_team_len = 0;
            read_file_into_buffer(f_walls_team, (char *)walls_team, &walls_team_len, LENOF(walls_team), sizeof(*walls_team));

            assert(walls_x_len == walls_y_len);
            assert(walls_y_len == walls_team_len);

            // debug
            printf("walls_x[0] == %d\n", walls_x[0]);
            printf("walls_y[0] == %d\n", walls_y[0]);
            printf("walls_team[0] == %d\n", walls_team[0]);

            map_load(
                walls_x, walls_x_len,
                walls_y, walls_y_len,
                walls_team, walls_team_len,
                players
            );
        }

        fclose(f_walls_x);
        fclose(f_walls_y);
        fclose(f_walls_team);
    }

    // spawn towers

    for(int team=0; team<=1; ++team){
        for(int tower_idx=0; tower_idx<NUMBER_OF_TOWERS; ++tower_idx){
            struct player *tower = generate_new_entity(players);
            assert(tower); // entity limit reached

            enum entity_type et = ET_TOWER;
            int connfd = -1;
            struct sockaddr_in sock = {0};
            int sock_len = 0;

            player_init(tower, team, et, connfd, sock, sock_len);
            player_spawn(tower, players, -1, -1);
        }
    }

    // spawn walls

    for(int team=0; team<=1; ++team){
        for(int idx=0; idx<NUMBER_OF_WALLS; ++idx){
            struct player *wall = generate_new_entity(players);
            assert(wall); // entity limit reached

            enum entity_type et = ET_WALL;
            int connfd = -1;
            struct sockaddr_in sock = {0};
            int sock_len = 0;

            player_init(wall, team, et, connfd, sock, sock_len);
            player_spawn(wall, players, -1, -1);
        }
    }

    // spawn players

    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *player = &players[player_idx];
        switch(player->et){
            case ET_HERO_HUMAN:
            case ET_HERO_BOT:
                player_spawn(player, players, -1, -1);
                break;
            case ET_MINION:
            case ET_TOWER:
            case ET_WALL:
                break;
        }
    }

    // game loop

    int winning_team = -1;

    long long last_minion_spawned_at = 0;

    while(1){

        // process input

        for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
            struct player *player = &players[player_idx];
            player_select_action(player, players);
        }

        // respawn players

        {
            long long now = get_time_ms();
            for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
                struct player *player = &players[player_idx];
                switch(player->et){
                    case ET_HERO_HUMAN:
                    case ET_HERO_BOT:
                        if(!player->alive){
                            if(player->died_at_ms + RESPAWN_TIME_MS <= now){
                                player_spawn(player, players, -1, -1);
                            }
                        }
                        break;
                    case ET_MINION:
                    case ET_TOWER:
                    case ET_WALL:
                        break;
                }
            }
        }

        // spawn minions

        {

            long long now = get_time_ms();
            if(now - MINION_SPAWN_INTERVAL_MS > last_minion_spawned_at){
                last_minion_spawned_at = now;

                // spawn if there is enough room

                struct player *minion = generate_new_entity(players);

                if(!minion){
                    printf("entiy limit reached, cannot spawn new minion\n");
                }else{
                    int team = rand() % 2;
                    int et = ET_MINION;
                    int connfd = -1;
                    struct sockaddr_in sock = {0};
                    int sock_len = 0;

                    player_init(minion, team, et, connfd, sock, sock_len);
                    player_spawn(minion, players, -1, -1);

                    // give the minion the average level of all alive players

                    int average_level = 0;
                    int average_level_count = 0;

                    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
                        struct player *player = &players[player_idx];
                        if(!player->alive){
                            continue;
                        }
                        if(player->team != minion->team){
                            continue;
                        }

                        average_level += player->level;
                        average_level_count += 1;
                    }

                    average_level /= average_level_count;

                    int level_diff = average_level - minion->level;
                    player_gain_xp(minion, players, level_diff * XP_FOR_LEVEL_UP);
                }
            }

        }

        // draw ui

        for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
            struct player *player = &players[player_idx];
            player_draw_ui(player);
        }

        // check if win conditions are met

        int players_alive[2] = {0};

        for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
            struct player *player = &players[player_idx];

            if(player->alive){
                players_alive[player->team] += 1;
            }
        }

        if(players_alive[0] <= 0){
            winning_team = 1;
            break;
        }else if(players_alive[1] <= 0){
            winning_team = 0;
            break;
        }
    }

    assert(winning_team != -1);

    printf("game ended; sending end screen\n");

    {
        screen_clear(players);
        screen_cur_set(players, 0, 0);

        char msg_winner[] = "Winning team: ";
        screen_print(players, msg_winner, sizeof(msg_winner)-1);

        char winning_team_as_char = winning_team+'0';
        screen_print(players, &winning_team_as_char, sizeof(winning_team_as_char));

        screen_cur_set(players, 1, 0);

        char msg_team_members[] = "Team members: ";
        screen_print(players, msg_team_members, sizeof(msg_team_members)-1);

        int cur_x = sizeof(msg_team_members)-1;

        for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
            struct player *player = &players[player_idx];

            if(player->team == winning_team){
                player->alive = 1; // TODO create `draw_raw` or something like that instead of this hack
                player->x = cur_x;
                cur_x += 1;
                player->y = 1;

                player_draw(player, players);
            }
        }

        screen_cur_set(players, 2, 0);
    }

    // I don't do any uninitialisations cuz I'm nasty like that

    return 0;
}
