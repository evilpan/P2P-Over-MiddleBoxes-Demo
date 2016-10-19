#include "utils.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <strings.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#define MAX_EVENTS 64
void on_client_connect(int client_fd)
{
    /* new client is accepted */
    fprintf(stdout, "Client[%d] is connected.\n", client_fd);
}
void on_client_disconnect(int client_fd)
{
    fprintf(stdout, "Client[%d] is disconnected.\n", client_fd);
    close(client_fd);
}
void on_client_data(int client_fd, char *data, unsigned int size)
{
    fprintf(stdout, "Receive [%zd bytes] from client[%d]: %s\n",size, client_fd, data);
}
void start_read(int connfd)
{
    ssize_t read_size;
    char read_buf[RECV_BUFSIZE] = {0};
    bool disconnected = false;
    while(true)
    {
        read_size = read(connfd, read_buf, sizeof(read_buf));
        if (read_size == -1)
        {
            /* If errno == EAGAIN, that means we have read all
               data. So go back to the main loop. */
            if (errno != EAGAIN)
            {
                if(errno == EBADF)
                    fprintf(stdout, "BAD socket, maybe the server has closed it.\n");
                else
                {
                    fprintf(stderr, "ERROR on read client data. reason:%s\n", strerror(errno));
                    disconnected = true;
                }
            }
            break;
        }
        else if (read_size == 0)
        {
            /* End of file. The remote has closed the
               connection. */
            disconnected = true;
            break;
        }

        /* handle the read data */
        on_client_data(connfd, read_buf, read_size);
    }
    if(disconnected)
    {
        on_client_disconnect(connfd);
    }
}
void start_accept(int listen_fd, int epoll_fd, struct epoll_event* pEvent)
{
    int conn_sock;
    struct sockaddr_in client_addr;
    socklen_t client_len;
    while(1)
    {
        client_len = sizeof(client_addr);
        bzero((void *)&client_addr, client_len);
        conn_sock = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len);
        if(conn_sock == -1)
        {
            if( (errno == EAGAIN) || (errno == EWOULDBLOCK)) 
            {
                /* All incoming connections are processed. */
                break;
            }
            else {
                fprintf(stderr, "ERROR: accept\n");
                continue;
            }
        }

        /* Make the incoming socket non-blocking and add it to the
           list of fds to monitor. */
        set_nonblocking(conn_sock);
        pEvent->events = EPOLLIN | EPOLLET;
        pEvent->data.fd = conn_sock;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock, pEvent) == -1)
        {
            fprintf(stderr, "epoll_ctl: conn_sock\n");
            close(conn_sock);
            continue;
        }
        on_client_connect(conn_sock);
    }
}
void start_receive(int listen_fd)
{
    int epollfd;
    struct epoll_event ev, events[MAX_EVENTS];

    bzero(&ev, sizeof(struct epoll_event));
    bzero(events, MAX_EVENTS * sizeof(struct epoll_event));

    set_nonblocking(listen_fd);
    epollfd = epoll_create(MAX_EVENTS);
    if(epollfd == -1) {
        fprintf(stderr, "ERROR: epoll_create\n");
        return ;
    }
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
        fprintf(stderr, "epoll_ctl: listen_fd\n");
        return ;
    }

    fprintf(stdout, "Start to receive incoming client(s)...\n");
    while(true)
    {
        int fd_nums;
        fd_nums = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if(fd_nums == -1){
            fprintf(stderr, "ERROR: epoll_wait\n");
            break ;
        }

        for(int i = 0; i < fd_nums; i++)
        {
            if( (events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!events[i].events & EPOLLIN) )
            {
                fprintf(stderr, "ERROR: epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            else if(events[i].data.fd == listen_fd)
            {

                /* One or more incoming connections */
                start_accept(listen_fd, epollfd, &ev);
                continue;
            }
            else
            {
                /* We have data on the fd waiting to be read. 
                 * We must read whatever data is available completely,
                 * as we are running in edge-triggered mode
                 * and won't get a notification again for the same data.
                 */
                start_read(events[i].data.fd);
                continue;
            }
        }
    }
}

void start_server(const char *host, int port)
{
    struct sockaddr_in serv_addr;
    int listen_fd;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0) {
        fprintf(stderr, "ERROE opening socket");
        return;
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(host);
    serv_addr.sin_port = htons(port);

    if(bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {

        fprintf(stderr, "ERROR on binding\n");
        return;
    }

    listen(listen_fd, SOMAXCONN);
    start_receive(listen_fd);
}
int main(int argc, char**argv)
{
    const char *host = "0.0.0.0";
    int port = 4444;
    if(argc == 2)
    {
        port = atoi(argv[1]);
    }
    else if(argc == 3)
    {
        host = argv[1];
        port = atoi(argv[2]);
    }
    else
    {
        fprintf(stderr, "Usage: %s [host] port\n", argv[0]);
        return 1;
    }

    fprintf(stdout, "Server start listening on %s:%d \n", host, port);
    start_server(host, port);
    return 0;
}
