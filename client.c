#include "utils.h"
#include "client.h"

#include <netdb.h> 
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

static remote_info_t G_Servers;
static remote_info_t G_Peers;

/* TODO: make those callbacks thread safe */
void on_server_data(int serv_fd, char *buf, unsigned int read_size)
{
    LOG_TRACE("Receive %d bytes from server[%d].", read_size, serv_fd);
    LOG_TRACE("%s", buf);
    char *command = strtok(buf, " ");
    if(0 == strncmp(command, "PUNCH_REQUEST", 12))
    {
        char *params = strtok(NULL, " ");
        if(params == NULL)
        {
            LOG_ERROR("No params follow PUNCH_REQUEST");
            return;
        }
        else
        {
            /* ex. params = 192.168.1.1:52333 */
            do {
                char *ip = strtok(params, ":");
                if(ip == NULL) break;
                char *str_port = strtok(NULL, ":");
                if(str_port == NULL) break;
                int port = atoi(str_port);
                LOG_TRACE("Receive PUNCH_REQUEST from %s:%d", ip, port);
                int peerfd = udp_send(ip, port, "PONG", 4);
                return ;

            } while(false);
            LOG_ERROR("Error PUNCH_REQUEST parameters");
        }
    }
}
void add_node(remote_info_t *head, peer_info_t *server)
{
    for(peer_info_t *s = head->peer_head; s != NULL; s = s->next)
    {
        if(s->next == NULL)
        {
            s->next = server;
            head->peer_nums++;
            break;
        }
    }
    if(head->peer_nums == 1)
    {
        pthread_create(&G_Servers.handle_thread, NULL, handle_server, &G_Servers);
    }
}
void remove_node(remote_info_t *head, int serv_fd)
{
    /* delete peer_info */
    peer_info_t *server;
    for(server = head->peer_head; server != NULL; server = server->next)
    {
        if(server->next && server->next->fd == serv_fd)
        {
            peer_info_t *temp = server->next;
            server->next = temp->next;
            free(temp);
            head->peer_nums--;
            break;
        }
    }
}
void on_sever_connect(int serv_fd)
{
    LOG_TRACE("Connected to server[%d].", serv_fd);

    peer_info_t *server;
    server = (peer_info_t *)malloc(sizeof(peer_info_t));
    server->fd = serv_fd;
    server->next = NULL;
    
    add_node(&G_Servers, server);
}
void on_server_disconnect(int serv_fd)
{
    LOG_TRACE("Disconnected from server[%d].", serv_fd);
    
    remove_node(&G_Servers, serv_fd);
}
void list_server(const char *id)
{
    if(id == NULL)
    {
        if(G_Servers.peer_nums > 0)
        {
            LOG_TRACE("Current connected servers:");
            for(peer_info_t *s = G_Servers.peer_head; s != NULL; s = s->next)
            {
                if(s->fd != -1)
                    LOG_TRACE("server[%d]", s->fd);
            }
        }
        else
            LOG_TRACE("No server connected.");
    }
    else
    {
        int serv_fd = atoi(id);
        const char *request = "LIST_REQUEST";
        send_to_serv(serv_fd, request);
    }
}
void print_help()
{
    const char *help_message = ""
        "Usage:"
        "\n\n- list [ID|all]:"
        "\n     List available peer(s) of connected p2p server ID,"
        "\n     If no params specified, list connected server id(s)."
        "\n\n- connect [host] [port]:"
        "\n     Connect the new p2p server."
        "\n\n- punch [server-id] [host:port]:"
        "\n     Send udp message to host:port(seen by server) as well as"
        "\n     sending punch requet to peer through server."
        "\n     For example:"
        "\n         punch 5 192.168.1.110:52333"
        "\n     This will get a tunnel over NAT(s) to 192.168.1.110:52333 if successed"
        "\n\n- send [peer-fd] [message]:"
        "\n     Send [message] to [peer-fd] which is already punched."
        "\n\n- help:"
        "\n     Print this message."
        "\n\n- quit: Quit the application.\n";
    LOG_TRACE("%s", help_message);
}
void punch_peer(int serv_fd, char *params)
{
    do {
        /* send udp packet to peer*/
        char *host = strtok(params, ":");
        if(host == NULL) break;
        char *s_port = strtok(NULL, ":");
        if(s_port == NULL) break;
        int port = atoi(s_port);
        int peer_fd = udp_send(host, port, "PING", 4);

        /* send punch request to server */
        char request[128] = {0};
        sprintf(request, "PUNCH_REQUEST %s:%d", host, port);
        LOG_TRACE("Sending punch request to %s", request);
        send_to_serv(serv_fd, request);

        /* receive PONG from peer */
        udp_recv(peer_fd, host, port);


    }while(false);

}
int send_to_peer(int peer_fd, const char *data)
{
    for(peer_info_t *s = G_Peers.peer_head; s != NULL; s = s->next)
    {
        if(s->fd == peer_fd)
        {
            write(peer_fd, data, strlen(data));
            return 0;
        }
    }
    LOG_ERROR("Send Error : no such peer is punched.");
    return 1;
}
int send_to_serv(int serv_fd, const char *data)
{
    for(peer_info_t *s = G_Servers.peer_head; s != NULL; s = s->next)
    {
        if(s->fd == serv_fd)
        {
            write(serv_fd, data, strlen(data));
            return 0;
        }
    }
    LOG_ERROR("Send Error : no such server is connected.");
    return 1;
}
void run_console()
{
    char *line = NULL;
    size_t len;
    ssize_t read;
    while(printf(">>>") && (read = getline(&line, &len, stdin)) != -1)
    {
        if(read == 1)
            continue;
        char *tokens[3] = {NULL};
        int token_nums = 0;
        for(int i = 0; i < 3; i++)
        {
            if(i == 0)
                tokens[i] = strtok(line, " ");
            else
                tokens[i] = strtok(NULL, " ");

            if(tokens[i] == NULL)
                break;
            token_nums++;
            //LOG_TRACE("params[%d]:%s", i, tokens[i]);
        }
        if((0 == strncmp(tokens[0], "list", 4)) && (token_nums < 3))
            list_server(tokens[1]);
        else if( (0 == strncmp(tokens[0], "connect", 7)) && (3 == token_nums) )
        {
            int serv_fd = connect_p2p_server(tokens[1], atoi(tokens[2]));
            if(serv_fd != -1)
                on_sever_connect(serv_fd);
        }
        else if( (0 == strncmp(tokens[0], "punch", 5)) && (3 == token_nums) )
            punch_peer(atoi(tokens[1]), tokens[2]);
        else if( (0 == strncmp(tokens[0], "send", 4)) && (3 == token_nums) )
            send_to_peer(atoi(tokens[1]), tokens[2]);
        else if( 0 == strncmp(tokens[0], "help", 4) )
            print_help();
        else if( 0 == strncmp(tokens[0], "quit", 4) )
            break;
        else
            print_help();

        ;
        continue;
    }
    free(line);
}
void *handle_server(void* tcp_servers)
{
    remote_info_t *head = (remote_info_t *)tcp_servers;
    struct timeval timeout;
    fd_set  rfds;
    int retval;
    char buf[RECV_BUFSIZE];
    
    LOG_TRACE("Server handler begin...");
    while(true)
    {
        int max_sockfd = 0;
        FD_ZERO(&rfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 500 * 1000;
        for(peer_info_t *s = head->peer_head; s != NULL; s = s->next)
        {
            if(s->fd != -1)
            {
                int sockfd = s->fd;
                set_nonblocking(sockfd);
                FD_SET(sockfd, &rfds);
                max_sockfd = max_sockfd > sockfd ? max_sockfd : sockfd;
            }
        }
        if(max_sockfd == 0) break;
        
        retval = select(max_sockfd+1, &rfds, NULL, NULL, &timeout); 
        if(retval == -1)
        {
            if(errno != EBADF)
                LOG_ERROR("ERROR on select: %s", strerror(errno));
            break;
        }
        else if(retval)
        {
            /* loop rfds but we have only one */
            for(peer_info_t *s = head->peer_head; s != NULL; s = s->next)
            {
                if(s->fd == -1) continue;
                if( !FD_ISSET(s->fd, &rfds) ) continue;
                int sockfd = s->fd;
                int read_size;
                bool disconnected = false;
                while(true)
                {
                    bzero(buf, RECV_BUFSIZE);
                    read_size = read(sockfd, buf, RECV_BUFSIZE);
                    if(read_size == -1)
                    {
                        if(errno != EAGAIN)
                            disconnected = true;
                        break;
                    }
                    else if(read_size == 0)
                    {
                        /* EOF */
                        disconnected = true;
                        break;
                    }
                    else
                    {
                        on_server_data(sockfd, buf, read_size);
                        continue;
                    }
                }
                if(disconnected)
                {
                    close(sockfd);
                    FD_CLR(sockfd, &rfds);
                    on_server_disconnect(sockfd);
                    continue;
                }
            }
        }
    }
    LOG_TRACE("Server handler exit...");
    return NULL;
}
int connect_p2p_server(const char *host, int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    do{
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd < 0)
        {
            LOG_ERROR("ERROR opening socket");
            break;
        }

        server = gethostbyname(host);
        if(!server)
        {

            LOG_ERROR("ERROR: no such host");
            close(sockfd);
            sockfd = -1;
            break;
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        bcopy((char*)server->h_addr_list[0],
                (char*)&serv_addr.sin_addr.s_addr,
                server->h_length);

        //set_nonblocking(sockfd);
        if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        {
            if( errno != EINPROGRESS) {
                LOG_ERROR("ERROR: Failed to connect to %s:%d", host, port);
                close(sockfd);
                sockfd = -1;
                break;
            }
        }
    }while(false);

    return sockfd;
}

int udp_send(const char *host, int port, const void *data, unsigned int size)
{
    struct sockaddr_in peer_addr;
    socklen_t slen = sizeof(peer_addr);
    bzero((void *)&peer_addr, slen);
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    do{
        if(sockfd == -1) break;
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_addr.s_addr = inet_addr(host);
        peer_addr.sin_port = htons(port);
        //if( bind(sockfd, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0 ) 
        //{
        //    LOG_ERROR("ERROR bind udp address");
        //    sockfd = -1;
        //    break;
        //}
        sendto(sockfd, data, size, MSG_DONTWAIT, (const struct sockaddr *)&peer_addr, slen);


    }while(false);

    return sockfd;
}
int udp_recv(int sockfd, const char *host, int port)
{
    struct sockaddr_in peer_addr;
    socklen_t slen = sizeof(peer_addr);
    bzero((void *)&peer_addr, slen);
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = inet_addr(host);
    peer_addr.sin_port = htons(port);
    char recv_buf[RECV_BUFSIZE] = {0};
    ssize_t rd_size = recvfrom(sockfd, recv_buf, RECV_BUFSIZE, 0, (struct sockaddr*)&peer_addr, &slen);
    if(rd_size == -1)
        LOG_ERROR("Read error");
    else
        LOG_TRACE("Recv %ld bytes from peer:%s", rd_size, recv_buf);
    return rd_size;
}
void *handle_peer(void *udp_peers)
{
    remote_info_t *head = (remote_info_t *)udp_peers;
    return NULL;
}
int main()
{
    peer_info_t *serv_head = (peer_info_t *)malloc(sizeof(peer_info_t));
    serv_head->fd = -1;
    serv_head->next = NULL;
    G_Servers.peer_head = serv_head;
    G_Servers.peer_nums = 0;

    peer_info_t *peer_head = (peer_info_t *)malloc(sizeof(peer_info_t));
    peer_head->fd = -1;
    peer_head->next = NULL;
    G_Servers.peer_head = peer_head;
    G_Servers.peer_nums = 0;


    run_console();
    return 0;
}
