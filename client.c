#include "utils.h"
#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <netinet/in.h>
#include <netdb.h> 
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

peer_info_t   *g_servers;
peer_info_t   *g_peers;

void on_server_data(int sockfd, const char *buf, unsigned int read_size)
{
    fprintf(stdout, "Receive %d bytes from server[%d].\n", read_size, sockfd);
}
void on_sever_connect(int serv_fd)
{
    fprintf(stdout, "Connected to server[%d].\n", serv_fd);
    pthread_t *thread = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(thread, NULL, handle_server, serv_fd);
    peer_info_t *server;
    server = (peer_info_t *)malloc(sizeof(peer_info_t));
    server->fd = serv_fd;
    server->handle_thread = thread;
    server->next = NULL;

    if(!g_servers)
        g_servers = server;
    else
    {
        for(peer_info_t *s = g_servers; s != NULL; s = s->next)
            if(s->next == NULL)
            {
                s->next = server;
                break;
            }
    }
}
void on_server_disconnect(int serv_fd)
{
    fprintf(stdout, "Disconnected from server[%d].\n", serv_fd);
    close(serv_fd);

    /* delete peer_info */
    peer_info_t *server;
    for(server = g_servers; server != NULL; server = server->next)
    {
        if(server->next && server->next->fd == serv_fd)
        {
            peer_info_t *temp = server->next;
            server->next = temp->next;
            free(temp);
            break;
        }
    }
}
void list_server(const char *id)
{
    if(id == NULL)
    {
        fprintf(stdout, "Current connected servers:\n");
        for(peer_info_t *s = g_servers; s != NULL; s = s->next)
        {
            fprintf(stdout, "server[%d]\n", s->fd);
        }
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
        "\n\n- punch [server-id] [peer-id]:"
        "\n     Send punch requet to [peer-id] which is connected to [server-id]."
        "\n\n- sendmsg [peer-fd] [message]:"
        "\n     Send [message] to [peer-fd] which is already punched."
        "\n\n- help:"
        "\n     Print this message."
        "\n\n- quit: Quit the application.\n";
    fprintf(stdout, "%s", help_message);
}
int send_to_peer(int peer_fd, const char *data)
{
    fprintf(stdout, "Send to peer[%d]:%s\n", peer_fd, data);
    return 0;
}
int punch_peer(int serverfd, int peerid)
{
    int peer_fd = 9;
    fprintf(stdout, "Punching ->server[%d]->peer[%d].\n", serverfd, peerid);
    return peer_fd;
}
void run_console()
{
    char *line = NULL;
    size_t len;
    ssize_t read;
    while( (read = getline(&line, &len, stdin)) != -1)
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
            printf("%d : %s\n", i, tokens[i]);
        }
        if((0 == strncmp(tokens[0], "list", 4)) && (token_nums < 2))
            list_server(tokens[1]);
        else if( (0 == strncmp(tokens[0], "connect", 7)) && (3 == token_nums) )
        {
            int serv_fd = connect_p2p_server(tokens[1], atoi(tokens[2]));
            on_sever_connect(serv_fd);
        }
        else if( (0 == strncmp(tokens[0], "punch", 5)) && (3 == token_nums) )
            punch_peer(atoi(tokens[1]), atoi(tokens[2]));
        else if( (0 == strncmp(tokens[0], "sendmsg", 7)) && (3 == token_nums) )
            send_to_peer(atoi(tokens[1]), tokens[2]);
        else if( 0 == strncmp(tokens[0], "help", 4) )
            print_help();
        else if( 0 == strncmp(tokens[0], "quit", 4) )
            break;
        else
            print_help();

        printf(">>>");
        continue;
    }
    free(line);
}
void handle_server(int sockfd)
{
    struct timeval timeout;
    fd_set  rfds;
    int retval;
    char buf[RECV_BUFSIZE];
    
    set_nonblocking(sockfd);
    while(true)
    {
        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        
        retval = select(sockfd+1, &rfds, NULL, NULL, &timeout); 
        if(retval == -1)
        {
            if(errno != EBADF)
                fprintf(stderr, "ERROR on select: %s\n", strerror(errno));
            break;
        }
        else if(retval)
        {
            /* should loop rfds but we have only one */
            int read_size;
            bool disconnected = false;
            while(true)
            {
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
                on_server_disconnect(sockfd);
                return;
            }
        }
    }
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
            fprintf(stderr, "ERROR opening socket\n");
            break;
        }

        server = gethostbyname(host);
        if(!server)
        {

            fprintf(stderr, "ERROR: no such host\n");
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
                fprintf(stderr, "ERROR: Failed to connect to %s:%d\n", host, port);
                close(sockfd);
                sockfd = -1;
                break;
            }
        }
    }while(false);

    return sockfd;
}
int main()
{
    g_servers = NULL;
    g_peers = NULL;
    run_console();
    return 0;
}
