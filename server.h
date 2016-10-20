#ifndef _P2P_SERVER_H
#define _P2P_SERVER_H
#include <netinet/in.h>
#include <stdlib.h>
typedef struct client_info_t client_info_t;
struct client_info_t {
    int             fd;
    char            ip[INET6_ADDRSTRLEN];   /* the ip address seen by server(ie. maybe NAT address) */
    int             port;       /* the port we used to punch since it is cone NAT */
    client_info_t  *next;
};

void on_client_connect(int client_fd);
void on_client_disconnect(int client_fd);
void on_client_data(int client_fd, char *data, unsigned int size);
void send_to_client(int client_fd, char *data);
void send_punch_request(int from, int to);
void list_client(int client_fd, client_info_t *head, char *peer_list);

#endif
