#include "utils.h"
#include "server.h"
#include <stdio.h>

#include <string.h>
#include <strings.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#define MAX_EVENTS 64

static client_info_t *g_clients;
void on_client_connect(int client_fd)
{
    /* new client is accepted */
    LOG_TRACE("Client[%d] is connected.", client_fd);
    for(client_info_t *c = g_clients; c != NULL; c = c->next)
    {
        if(c->next == NULL)
        {
            struct sockaddr_in peer_addr;
            socklen_t peer_addr_len = sizeof(peer_addr);
            getpeername(client_fd, (struct sockaddr *)&peer_addr, &peer_addr_len);
            char ipstr[INET6_ADDRSTRLEN];
            int port;
            
            port = ntohs(peer_addr.sin_port);
            inet_ntop(AF_INET, &peer_addr.sin_addr, ipstr, sizeof ipstr);

            client_info_t *node = (client_info_t *)malloc(sizeof(client_info_t));
            node->fd = client_fd;
            sprintf(node->ip, "%s", ipstr);
            node->port = port;
            node->next = NULL;

            c->next = node;
            break;
        }
    }
}
void on_client_disconnect(int client_fd)
{
    LOG_TRACE("Client[%d] is disconnected.", client_fd);
    for(client_info_t *c = g_clients; c != NULL; c = c->next)
    {
        if(c->next && c->next->fd == client_fd)
        {
            close(client_fd);
            client_info_t *temp = c->next;
            c->next = temp->next;
            free(temp);
        }
    }
}
void on_client_data(int client_fd, char *data, unsigned int size)
{
    LOG_TRACE("Receive [%zd bytes] from client[%d]: %s",size, client_fd, data);
    char *cmd = strtok(data, " ");
    if(0 == strncmp(cmd, "LIST_REQUEST", 12))
    {
        char peer_list[1024] = {0};
        list_client(client_fd, g_clients, peer_list);
        LOG_TRACE("%s", peer_list);
        send_to_client(client_fd, peer_list);

    }
    else if(0 == strncmp(cmd, "PUNCH_REQUEST", 13))
    {
        char *params = strtok(NULL, " ");
        if(params == NULL)
            LOG_ERROR("NO params for PUNCN_REQUEST");
        else
        {
            int punch_id = atoi(params);
            send_punch_request(client_fd, punch_id);
        }
    }
}
void send_punch_request(int from, int to)
{
    for(client_info_t *c = g_clients; c != NULL; c = c->next)
    {
        if(c->fd == from)
        {
            char request[128] = {0};
            sprintf(request, "PUNCH_REQUEST %s:%d", c->ip, c->port);
            send_to_client(to, request);
            break;
        }
    }

}
void send_to_client(int client_fd, char *data)
{
    for(client_info_t *c = g_clients; c != NULL; c = c->next)
    {
        if(c->fd == client_fd)
        {
            write(client_fd, data, strlen(data));
            return;
        }
    }
    LOG_ERROR("No client[client_fd]");
}

void list_client(int client_fd, client_info_t *head, char *peer_list)
{
    for(client_info_t *s = head; s != NULL; s = s->next)
    {
        if(s->fd != -1)
        {
            char peer[50] = {0};
            if(s->fd == client_fd)
                sprintf(peer, "[*]Peer %d %s:%d\n", s->fd, s->ip, s->port);
            else
                sprintf(peer, "Peer %d %s:%d\n", s->fd, s->ip, s->port);
            strcat(peer_list, peer);
        }
    }
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
                    LOG_TRACE("BAD socket, maybe the server has closed it.");
                else
                {
                    LOG_ERROR("ERROR on read client data. reason:%s", strerror(errno));
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
                LOG_ERROR("ERROR: accept");
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
            LOG_ERROR("epoll_ctl: conn_sock");
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
        LOG_ERROR("ERROR: epoll_create");
        return ;
    }
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
        LOG_ERROR("epoll_ctl: listen_fd");
        return ;
    }

    LOG_TRACE("Start to receive incoming client(s)...");
    while(true)
    {
        int fd_nums;
        fd_nums = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if(fd_nums == -1){
            LOG_ERROR("ERROR: epoll_wait");
            break ;
        }

        for(int i = 0; i < fd_nums; i++)
        {
            if( (events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!events[i].events & EPOLLIN) )
            {
                LOG_ERROR("ERROR: epoll error");
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
        LOG_ERROR("ERROE opening socket");
        return;
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(host);
    serv_addr.sin_port = htons(port);

    if(bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {

        LOG_ERROR("ERROR on binding");
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
        LOG_ERROR("Usage: %s [host] port", argv[0]);
        return 1;
    }

    /* init an empty client head node */
    g_clients = (client_info_t *)malloc(sizeof(client_info_t));
    g_clients->fd = -1;
    g_clients->port = 0;
    g_clients->next = NULL;

    LOG_TRACE("Server start listening on %s:%d ", host, port);
    start_server(host, port);

    free(g_clients);
    return 0;
}
