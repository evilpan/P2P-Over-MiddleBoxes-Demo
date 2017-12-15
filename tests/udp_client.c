#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#define RECV_BUFSIZE 512
void *udp_receive_loop(void *sockfd)
{
    struct sockaddr_in addr;
    socklen_t addrlen;
    char data[RECV_BUFSIZE];
    for(;;) {
        addrlen = sizeof(addr);
        memset(&addr, 0, addrlen);
        memset(data, 0, RECV_BUFSIZE);
        int rd_size = recvfrom(*(int*)sockfd, data, RECV_BUFSIZE, 0,
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
    return 0;
}
int udp_send(int sockfd,
        const char *ip, int port,
        const char *data, int data_len)
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    memset(&addr, 0, addrlen);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    ssize_t sent = sendto(sockfd, data, data_len, MSG_DONTWAIT,
                   (const struct sockaddr *)&addr, addrlen);
    printf("Sent %zd bytes to %s:%d\n", sent, ip, port);
    return sent;
}
void print_help()
{
    const char *help_message = ""
        "Usage:"
        "\n\n sendto host:port data"
        "\n     Send text [data] to [host:port] through UDP protocol."
        "\n     Example:"
        "\n     >>> sendto 114.114.114.114:53 hello"
        "\n\n help"
        "\n     Print this help message."
        "\n\n quit"
        "\n     Quit this program.";
    printf("%s\n", help_message);
}

void console_loop(int sockfd)
{
    char *line = NULL;
    size_t len;
    ssize_t read;
    while(fprintf(stdout, ">>> ") && (read = getline(&line, &len, stdin)) != -1)
    {
        if (read == 1)
            continue;
        char *cmd = strtok(line, " ");
        if (strncmp(cmd, "sendto", 5) == 0) {
            char *host_port = strtok(NULL, " ");
            char *data = strtok(NULL, " ");

            char *host = strtok(host_port, ":");
            char *s_port = strtok(NULL, ":");
            int port = atoi(s_port);
            udp_send(sockfd, host, port, data, strlen(data));
        } else if (strncmp(cmd, "quit", 4) == 0) {
            printf("Quiting...\n");
            break;
        } else {
            printf("Unknown command %s\n", cmd);
            print_help();
        }
    }
    free(line);
}
int main()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
    }
    pthread_t pid;
    pthread_create(&pid, NULL, &udp_receive_loop ,&sock);
    console_loop(sock);
    close(sock);
    return 0;
}

