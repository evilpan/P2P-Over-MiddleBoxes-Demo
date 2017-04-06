#include "common.h"

#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

const char *CMD[] = {
    "LOGIN",
    "LOGOUT",
    "LIST",
    "PUNCH",
    "SYN",
    "ACK"
};

int create_udp_socket(const char *ip, int port)
{
    int success = 0; 
    int sockfd = -1;
    struct sockaddr_in addr;
    socklen_t slen = sizeof(addr);
    memset(&addr, 0, slen);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    do {
        sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(sockfd == -1) break;
        if( bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0 ) {
            LOG_ERROR("Failed to bind %s:%d",
                    ip, port);
            break;
        }
        success = 1;

    } while(0);
    if (!success && sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
    return sockfd;
}
int udp_send(int sockfd, const endpoint *dest, const char *data)
{
    struct sockaddr_in peer_addr;
    socklen_t slen = sizeof(peer_addr);
    memset(&peer_addr, 0, slen);
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = inet_addr(dest->ip);
    peer_addr.sin_port = htons(dest->port);

    ssize_t send_size = sendto(sockfd, data, strlen(data),
            MSG_DONTWAIT, (const struct sockaddr *)&peer_addr, slen);
    if(send_size == -1)
        LOG_ERROR("Send error");

    return send_size;
}

int udp_receive(int sockfd, endpoint *from, char *data)
{
    struct sockaddr_in peer_addr;
    socklen_t slen = sizeof(peer_addr);
    memset(&peer_addr, 0, slen);

    //char recv_buf[RECV_BUFSIZE] = {0};
    ssize_t rd_size = recvfrom(sockfd, data, RECV_BUFSIZE, 0,
            (struct sockaddr*)&peer_addr, &slen);
    if(rd_size == -1)
        LOG_ERROR("Read error");
    else
    {
        const char *ip = inet_ntoa(peer_addr.sin_addr);
        int port = peer_addr.sin_port;
        strncpy(from->ip, ip, INET_ADDRSTRLEN);
        from->port = ntohs(port);
    }
    return rd_size;
}
int start_udp_server(int sockfd, msg_callback on_message)
{
    char buf[RECV_BUFSIZE];
    endpoint ep;
    ssize_t read_size = 0;
    while(1)
    {
        memset(buf, 0, RECV_BUFSIZE);
        read_size = udp_receive(sockfd, &ep, buf);
        if (read_size != -1) {
            on_message(&ep, buf);
        }
        //LOG_TRACE("Recv %ld bytes from (%s:%d): %s", read_size, ep.ip, ep.port, buf);
    }

}

bool set_nonblocking(int sockfd)
{
    int flags, s;

    flags = fcntl (sockfd, F_GETFL, 0);
    if (flags == -1) {
        fprintf(stderr, "ERROR: fcntl get\n");
        return false;
    }
    flags |= O_NONBLOCK;
    s = fcntl (sockfd, F_SETFL, flags);
    if (s == -1) {
        fprintf(stderr, "ERROR: fcntl set\n");
        return false;
    }
    return true;
}
