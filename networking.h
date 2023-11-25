
#ifndef NETWORKING_H
#define NETWORKING_H

#include <netdb.h>

#include "settings.h"
#include "player.h"

int create_server(int *sockfd, struct sockaddr_in *servaddr, int port, int listen);
void net_send_single(int connfd, char *data, int data_len);
void net_send(struct player players[PLAYERS_REQUIRED], char *data, int data_len);
int net_recv_1B(int connfd, char *data);

#endif
