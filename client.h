#ifndef _P2P_CLINET_H
#define _P2P_CLINET_H

#include <pthread.h>
#include <netinet/in.h>
typedef struct peer_info_t peer_info_t;
typedef struct remote_info_t remote_info_t;
struct peer_info_t {
    int             fd;
    char            ip[INET6_ADDRSTRLEN];
    int             port;
    peer_info_t    *next;
};

struct remote_info_t {
    peer_info_t *peer_head;
    int          peer_nums;
    pthread_t    handle_thread;
};

void add_node(remote_info_t *head, peer_info_t *node);
void remove_node(remote_info_t *head, int node_fd);

int connect_p2p_server(const char *host, int port);
void list_server(const char *id);
void punch_peer(int serv_fd, char *params);

void on_server_data(int serv_fd, char *buf, unsigned int read_size);
void on_sever_connect(int serv_fd);
void *handle_server(void *head);
void on_server_disconnect(int serv_fd);
int send_to_serv(int serv_fd, const char *data);//TCP

void on_peer_data(int peer_id);
void on_peer_punch(int peer_id);
void *handle_peer(void *head);
int send_to_peer(int peer_fd, const char *data);//UDP


/* return the fd */
int udp_send(const char *host, int port, const void *data, unsigned int size);
int udp_recv(int sockfd, const char *host, int port);

#endif
