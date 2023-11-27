
#include <stdio.h>
#include <string.h>
#include <assert.h>

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

    struct player players[PLAYERS_MAX];
    int players_len = 0;
    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *player = &players[player_idx];
        player_init_mem(player);
    }

    int team = 0;

    assert(PLAYERS_REQUIRED <= PLAYERS_MAX);
    while(players_len < PLAYERS_REQUIRED){

        int is_bot = 0;
        int connfd = -1;
        struct sockaddr_in sock;
        unsigned int sock_len = sizeof(sock);

        printf("established connections %d of %d\n", players_len, PLAYERS_REQUIRED);
    
        if(players_len < NUMBER_OF_BOT_PLAYERS){
            is_bot = 1;
            printf("bot connected\n");
        }else{
            connfd = accept(sockfd, (struct sockaddr *) &sock, &sock_len);
            if(connfd < 0){
                printf("player could not connect\n");
                continue;
            }
            printf("player connected\n");
        }

        struct player *player = &players[players_len];
        player_init(player, team, is_bot, connfd, sock, sock_len);

        team = !team;
        players_len += 1;
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

    int winning_team = -1;

    while(1){

        // process input

        for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
            struct player *player = &players[player_idx];
            
            char action;

            if(player->bot){
                int skip = player_bot_select_action(player, players, &action); // TODO this can be replaced with `receive_action` or something like that
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

        // check if an entire team is dead

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
    // (the auto restart script will take care)

    return 0;
}
