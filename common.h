#ifndef P2P_DEMO_COMMON_H
#define P2P_DEMO_COMMON_H

#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

#define RECV_BUFSIZE 1024

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#define LOG_TRACE(format, args...) fprintf(stdout, format"\n", ##args)
#define LOG_ERROR(format, ...) fprintf(stderr, format" (%s)\n", ##__VA_ARGS__, strerror(errno))

enum {
    LOGIN = 0,
    LOGOUT,
    LIST,
    PUNCH,
    SYN,
    ACK
};

typedef struct endpoint endpoint;
struct endpoint {
    char ip[INET_ADDRSTRLEN];
    int port;
};
typedef void msg_callback(const endpoint *from, char *data);
int create_udp_socket(const char *ip, int port);
//data is NULL terminated
int udp_send(int sockfd, const endpoint *dest, const char *data);
int udp_receive(int sockfd, endpoint *from, char *data);
int start_udp_server(int sockfd, msg_callback on_message);

bool set_nonblocking(int sockfd);



#endif
