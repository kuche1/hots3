
#include "entities.h"

struct player *get_entity_at(int pos_y, int pos_x, struct player entities[ENTITIES_MAX]){
    for(int entity_idx=0; entity_idx<ENTITIES_MAX; ++entity_idx){
        struct player *entity = &entities[entity_idx];

        if(!entity->alive){
            continue;
        }
        if(entity->x != pos_x){
            continue;
        }
        if(entity->y != pos_y){
            continue;
        }

        return entity;
    }

    return NULL;
}
