
#include "util.h"

#include <sys/time.h>
#include <stddef.h>

#include "entity_type.h"

long long get_time_ms(void){
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long now_ms = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    return now_ms;
}

struct player * generate_new_entity(struct player players[PLAYERS_MAX]){

    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *player = &players[player_idx];

        if(player->alive){
            continue;
        }

        switch(player->et){
            case ET_MINION:
            case ET_TOWER: // if I change my mind about towers not being able to respawn I'll have to move this down
            case ET_WALL:
                return player;

            case ET_HERO_HUMAN:
            case ET_HERO_BOT:
                break;
        }
    }

    return NULL;
}

void read_file_into_buffer(FILE *fd, char *buf, int *arg_p_buf_len, int buf_maxlen, int buf_elem_size){
    int buf_len = *arg_p_buf_len;

    for(;;){
        int read = fread(
            buf + (buf_elem_size * buf_len),
            buf_elem_size,
            buf_maxlen - buf_len,
            fd
        );

        if(read <= 0){
            break;
        }

        buf_len += read;
    }

    *arg_p_buf_len = buf_len;
}
