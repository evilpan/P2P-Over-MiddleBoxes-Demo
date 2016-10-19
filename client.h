#ifndef _P2P_CLINET_H
#define _P2P_CLINET_H

#include <pthread.h>
typedef struct peer_info_t peer_info_t;
struct peer_info_t {
    int             fd;
    pthread_t      *handle_thread;
    peer_info_t    *next;
};


int connect_p2p_server(const char *host, int port);
void handle_server(int serverfd);
void list_server(const char *id);
int punch_peer(int serverfd, int peerid);
int send_to_peer(int peer_fd, const char *data);

#endif
