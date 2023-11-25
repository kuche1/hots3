
#include "settings.h"

int map_is_tile_empty(int pos_y, int pos_x){

    if((pos_y < 0) || (pos_x < 0)){
        return 0;
    }

    if((pos_y > MAP_Y) || (pos_x > MAP_X)){
        return 0;
    }

    return 1;
}
