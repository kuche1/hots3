
#include "util.h"

#include <sys/time.h>
#include <stddef.h>

long long get_time_ms(void){
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long now_ms = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    return now_ms;
}

struct player * generate_new_entity(struct player players[PLAYERS_MAX]){

    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *player = &players[player_idx];
        if((!player->alive) && (player->et == MINION)){
            return player;
        }
    }

    return NULL;
}
