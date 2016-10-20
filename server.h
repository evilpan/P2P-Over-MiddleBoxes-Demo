#ifndef _P2P_SERVER_H
#define _P2P_SERVER_H
#include <netinet/in.h>
typedef struct client_info_t client_info_t;
struct client_info_t {
    int             fd;
    char            ip[INET6_ADDRSTRLEN];   /* the ip address seen by server(ie. maybe NAT address) */
    int             port;
    client_info_t  *next;
};

void list_client(client_info_t *head);

#endif
