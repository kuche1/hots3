
#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

#include "settings.h"
#include "player.h"

#define LENOF(compiletime_array) (sizeof(compiletime_array)/sizeof(*(compiletime_array)))

long long get_time_ms(void);
struct player * generate_new_entity(struct player players[ENTITIES_MAX]); // not the most appropriate place but it will have to do
void read_file_into_buffer(FILE * fd, char * buf, int * arg_p_buf_len, int buf_maxlen, int buf_elem_size);

#endif
