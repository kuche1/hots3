
#include "networking.h"

#include <string.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> // read(), write(), close()

#include "errors.h"

int create_server(int *sockfd, struct sockaddr_in *servaddr, int port, int listen_max){
    *sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if(*sockfd == -1){
        return ERR_CANT_CREATE_SOCKET;
    }

    memset(servaddr, 0, sizeof(*servaddr));

    if(setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
        return ERR_CANT_SET_REUSEADDR;
    }

    servaddr->sin_family = AF_INET;
    servaddr->sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr->sin_port = htons(port);

    if(bind(*sockfd, (struct sockaddr *) servaddr, sizeof(*servaddr))){ 
        return ERR_CANT_BIND_SOCET;
    }

    if(listen(*sockfd, listen_max)){ 
        return ERR_CANT_LISTEN;
    }

    return 0;
}

// sending

void net_send_single(int connfd, char *data, int data_len){
    write(connfd, data, data_len); // TODO check how much bytes were sent
}

void net_send(struct player players[PLAYERS_MAX], char *data, int data_len){
    for(int player_idx=0; player_idx < PLAYERS_MAX; ++player_idx){
        struct player *player = &players[player_idx];

        if(player->bot){
            continue;
        }

        net_send_single(player->connfd, data, data_len);
    }
}

// receiving

int net_recv_1B(int connfd, char *data){
    return read(connfd, data, 1);
}
