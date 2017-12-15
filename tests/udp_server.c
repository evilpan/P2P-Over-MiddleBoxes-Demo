#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define RECV_BUFSIZE 512
void udp_server_loop(int listen_sock)
{
    struct sockaddr_in addr;
    socklen_t addrlen;
    char data[RECV_BUFSIZE];
    for(;;) {
        addrlen = sizeof(addr);
        memset(&addr, 0, addrlen);
        memset(data, 0, RECV_BUFSIZE);
        int rd_size = recvfrom(listen_sock, data, RECV_BUFSIZE, 0,
                (struct sockaddr*)&addr, &addrlen);
        if (rd_size == -1) {
            perror("recvfrom");
            break;
        }
        char *ip = inet_ntoa(addr.sin_addr);
        int port = addr.sin_port;
        if (rd_size == 0) {
            printf("disconnect from %s:%d\n", ip, port);
            continue;
        }
        printf("recv %d bytes from [%s:%d]: %s\n",
                rd_size, ip, port, data);
    }
    printf("udp_receive_loop stopped.\n");
}
int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    const char *host = "0.0.0.0";
    int port = atoi(argv[1]);
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    memset(&addr, 0, addrlen);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
    }
    int ret = bind(sock, (struct sockaddr*)&addr, addrlen);
    if (ret == -1) {
        perror("bind");
    }
    //ret = listen(sock, 5);
    //if (ret == -1) {
    //    perror("listen");
    //}
    printf("UDP bind on %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    udp_server_loop(sock);
    return 0;
}
