#ifndef _P2P_CLINET_H
#define _P2P_CLINET_H

#include <pthread.h>
typedef struct peer_info_t peer_info_t;
typedef struct remote_info_t remote_info_t;
struct peer_info_t {
    int             fd;
    peer_info_t    *next;
};

struct remote_info_t {
    peer_info_t *peer_head;
    int          peer_nums;
    pthread_t    handle_thread;
};


int connect_p2p_server(const char *host, int port);
void list_server(const char *id);
int punch_peer(int serv_fd, int peerid);

void on_server_data(int serv_fd, char *buf, unsigned int read_size);
void on_sever_connect(int serv_fd);
void add_server(peer_info_t *head, int serv_fd);
void handle_server(peer_info_t* head);
void on_server_disconnect(int serv_fd);
int send_to_serv(int serv_fd, const char *data);

void on_peer_data(int peer_id);
int send_to_peer(int peer_fd, const char *data);
void handle_peer(int peer_fd);

#endif
