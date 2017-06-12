#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include "common.h"
extern const char* CMD[];
int clientfd;

void print_help()
{
    const char *help_message = ""
        "Usage:"
        "\n\n login [host] [port]"
        "\n     Login to p2p server."
        "\n\n logout"
        "\n     Logout from p2p server."
        "\n\n list"
        "\n     List all logged peer(s) of server"
        "\n\n punch [host] [port]"
        "\n     Send punch request directly to host:port(seen by server)"
        "\n     and the same time, send punch requet to server to be relayed."
        "\n     For example:"
        "\n         punch 192.168.1.110 52333"
        "\n     This will get a tunnel over NAT(s) to 192.168.1.110:52333 if successed"
        "\n\n send [host] [port] [message]:"
        "\n     Send [message] to peer which is already punched."
        "\n\n help:"
        "\n     Print this message."
        "\n\n quit: Quit the application.\n";
    printf("%s\n", help_message);
}


void start_console(int clientfd)
{
    char *line = NULL;
    size_t len;
    ssize_t read;
    endpoint server;
    bool logedin = false;
    while(fprintf(stderr, ">>>") && (read = getline(&line, &len, stdin)) != -1)
    {
        if (read == 1)
            continue;
        enum { MAX_DELIMS = 4 }; /* enum hack */
        char *tokens[MAX_DELIMS] = {NULL};
        int token_nums = 0;
        for (int i = 0; i < MAX_DELIMS; i++)
        {
            if (i == 0)
                tokens[i] = strtok(line, " ");
            else
            {
                if (i == MAX_DELIMS - 1)
                    tokens[i] = strtok(NULL, "");
                else
                    tokens[i] = strtok(NULL, " ");
            }

            if (tokens[i] == NULL)
                break;
            token_nums++;
            //LOG_TRACE("params[%d]:%s", i, tokens[i]);
        }
        if ( (0 == strncmp(tokens[0], "login", 5)) && (3 == token_nums) )
        {
            strncpy(server.ip, tokens[1], strlen(tokens[1]) + 1 );
            server.port = atoi(tokens[2]);
            LOG_TRACE("login to %s:%d", server.ip, server.port);
            udp_send(clientfd, &server, CMD[LOGIN]);
            logedin = true;
        }
        else if ( (0 == strncmp(tokens[0], "logout", 6)) )
        {
            if (logedin) {
                udp_send(clientfd, &server, CMD[LOGOUT]);
                logedin = false;
            }
            else
                LOG_TRACE("you need to login first");

        }
        else if ((0 == strncmp(tokens[0], "list", 4)))
        {
            if (logedin)
                udp_send(clientfd, &server, CMD[LIST]);
            else
                LOG_TRACE("you need to login first");
        }
        else if ( (0 == strncmp(tokens[0], "punch", 5)) && (3 == token_nums) )
        {
            if (logedin) {

                endpoint peer;
                strncpy(peer.ip, tokens[1], sizeof(peer.ip));
                peer.port = atoi(tokens[2]);
                udp_send(clientfd, &peer, "anything");

                char command[40] = {};
                snprintf(command, 40, "%s %s %s", CMD[PUNCH], tokens[1], tokens[2]);
                udp_send(clientfd, &server, command);
            }
            else
                LOG_TRACE("you need to login first");
        }
        else if ( (0 == strncmp(tokens[0], "send", 4) && (4 == token_nums) ) )
        {
            endpoint peer;
            strncpy(peer.ip, tokens[1], strlen(tokens[1]) + 1 );
            peer.port = atoi(tokens[2]);
            udp_send(clientfd, &peer, tokens[3]);
            LOG_TRACE("send to (%s:%d):%s", peer.ip, peer.port, tokens[3]);
        }
        else if ((0 == strncmp(tokens[0], "quit", 4))) {
            if (logedin) 
                udp_send(clientfd, &server, CMD[LOGOUT]);
            break;
        }
        else
        {
            print_help();
        }

        continue;
    }
    free(line);
}

void on_message(const endpoint *from, char *data)
{
    LOG_TRACE("(%s:%d): %s", from->ip, from->port, data);

    char *cmd = strtok(data, " ");
    if (strcmp(cmd, CMD[SYN]) == 0) {
        char *peer_ip = strtok(NULL, " ");
        char *peer_port = strtok(NULL, " ");
        if(peer_ip && peer_port) {
            endpoint ep;
            strncpy(ep.ip, peer_ip, sizeof(ep.ip));
            ep.port = atoi(peer_port);
            udp_send(clientfd, &ep, CMD[ACK]);
        }
    }

}
struct arguments {
    int sockfd;
    msg_callback *on_message;
};
void *start_server_daemon(void *args)
{
    struct arguments *as = args;
    start_udp_server(as->sockfd, as->on_message);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        LOG_TRACE("Usage: %s host port", argv[0]);
        return 1;
    }
    clientfd = create_udp_socket(argv[1], atoi(argv[2]));
    assert( clientfd != -1);

    struct arguments args;
    args.sockfd = clientfd;
    args.on_message = on_message;
    pthread_t pid;
    if (0 == pthread_create(&pid, NULL, start_server_daemon ,&args) ) {
        LOG_TRACE("client listenning on %s:%s", argv[1], argv[2]);
    } else {
        return -1;
    }

    start_console(clientfd);

}
