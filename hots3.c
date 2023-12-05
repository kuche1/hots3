
// TODO requested by clients
//
// da dobavq alarak

// TODO
//
// da napravq kulite da sa za4erknati i levelup da e underline
//
// local multiplayer
//
// hotjoin ako ima botove?
//
// make it so that the anticheat works in `bursts`
//
// da napravq UI-a da se risuva otdelno (pak s check-ovete ako tova ili onova e razli4no, posibly v nova funkciq)
// i da ostavq map-a da se 4ertae v realtime
//
// allah

// INFO
//
// reverse effect is now not used by anyone

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "errors.h"
#include "networking.h"
#include "player.h"
#include "settings.h"
#include "screen.h"
#include "util.h"

int main(int argc, char **argv){

    // command line args

    if(argc != 2){
        printf("You need to specify exactly 1 arguments - number of bot players\n");
        return ERR_BAD_COMMAND_LINE_ARGS;
    }

    int number_of_bot_players = atoi(argv[1]);
    assert(number_of_bot_players != 0);
    if(number_of_bot_players >= PLAYERS_REQUIRED){
        printf("ERROR: number of bot players is too high\n");
        return ERR_BAD_COMMAND_LINE_ARGS;
    }

    // init socket

    int sockfd;
    struct sockaddr_in servaddr;

    {
        int err;
        if((err = create_server(&sockfd, &servaddr, PORT, LISTEN))){
            return err;
        }
    }

    // lobby

    struct player players[PLAYERS_MAX];
    int players_len = 0;
    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *player = &players[player_idx];
        player_init_mem(player);
    }

    int team = 0;

    assert(PLAYERS_REQUIRED <= PLAYERS_MAX);
    while(players_len < PLAYERS_REQUIRED){

        enum entity_type et = ET_HERO_HUMAN;
        int connfd = -1;
        struct sockaddr_in sock;
        unsigned int sock_len = sizeof(sock);

        printf("established connections %d of %d\n", players_len, PLAYERS_REQUIRED);
    
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
        }

        printf("initialising player...\n");
        struct player *player = &players[players_len];
        player_init(player, team, et, connfd, sock, sock_len);
        printf("init done\n");

        team = !team;
        players_len += 1;
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

    // spawn towers

    for(int team=0; team<=1; ++team){
        for(int tower_idx=0; tower_idx<TOWERS_PER_TEAM; ++tower_idx){
            struct player *tower = generate_new_entity(players);
            assert(tower); // entity limit reached

            enum entity_type et = ET_TOWER;
            int connfd = -1;
            struct sockaddr_in sock = {0};
            int sock_len = 0;

            player_init(tower, team, et, connfd, sock, sock_len);
            player_spawn(tower, players);
        }
    }

    // spawn players

    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *player = &players[player_idx];
        switch(player->et){
            case ET_HERO_HUMAN:
            case ET_HERO_BOT:
                player_spawn(player, players);
                break;
            case ET_MINION:
            case ET_TOWER:
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
                                player_spawn(player, players);
                            }
                        }
                        break;
                    case ET_MINION:
                    case ET_TOWER:
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
                    player_spawn(minion, players);

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
