
#include <stdio.h>
#include <string.h>

#include "errors.h"
#include "networking.h"
#include "player.h"
#include "settings.h"
#include "screen.h"

int main(void){
    int err;

    // init socket

    int sockfd;
    struct sockaddr_in servaddr;

    if((err = create_server(&sockfd, &servaddr, PORT, LISTEN))){
        return err;
    }

    // lobby

    int player_count = 0;
    struct player players[PLAYERS_REQUIRED];
    int team = 0;

    while(player_count < PLAYERS_REQUIRED){
        struct player *player = &players[player_count];
        player_init_mem(player);

        printf("established connections %d of %d\n", player_count, PLAYERS_REQUIRED);
    
        if(player_count < NUMBER_OF_BOT_PLAYERS){

            player_init_bot(player);

            printf("bot connected\n");

        }else{

            player->connfd = accept(sockfd, (struct sockaddr *) &player->sock, &player->sock_len);
            if(player->connfd < 0){
                printf("player could not connect\n");
                continue;
            }

            printf("player connected\n");
        
        }

        player->team = team;
        team = !team;

        player_count += 1;
    }

    // hero selection

    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
        struct player *player = &players[player_idx];
        player_select_hero(player);
    }

    // change to draw mode

    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
        struct player *player = &players[player_idx];
        player_init_telnet(player);
    }

    // spawn players

    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
        struct player *player = &players[player_idx];
        player_spawn(player, players);
    }

    // clear screen

    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
        struct player *player = &players[player_idx];
        screen_clear_single(player->connfd);
    }

    // print players

    for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
        struct player *player = &players[player_idx];

        player_draw(player, players);
    }

    // game loop

    struct player *winner = NULL;

    while(1){

        // process input

        for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
            struct player *player = &players[player_idx];
            
            char action;

            if(player->bot){
                int skip = player_bot_select_action(player, players, &action);
                if(skip){
                    continue;
                }
            }else{
                int bytes = net_recv_1B(player->connfd, &action);
                if(bytes <= 0){
                    continue;
                }
                // printf("received from `%d`: `%c` (%d)\n", player->connfd, action, (int)action);
            }

            player_process_action(player, action, players);
        }

        // check if everyone but 1 is dead

        int players_alive = 0;
        struct player *last_player_alive = NULL;

        for(int player_idx=0; player_idx < PLAYERS_REQUIRED; ++player_idx){
            struct player *player = &players[player_idx];

            if(player->alive){
                players_alive += player->alive;
                last_player_alive = player;
            }
        }

        if(players_alive <= 1){
            winner = last_player_alive;
            break;
        }
    }

    {
        screen_clear(players);
        screen_cur_set(players, 0, 0);
        char msg_winner[] = "Winner: ";
        net_send(players, msg_winner, sizeof(msg_winner)-1);
        winner->x = 0;
        winner->y = 1;
        player_draw(winner, players);
        screen_cur_set(players, 2, 0);
    }

    // I don't do any uninitialisations cuz I'm nasty like that
    // (the auto restart script will take care)

    return 0;
}
