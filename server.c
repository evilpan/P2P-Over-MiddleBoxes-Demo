#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "server_logic.h"
extern const char* CMD[];

static client_list *active_clients = NULL;
static int serverfd;

void on_message(const endpoint *from, char *data)
{
    LOG_TRACE("(%s:%d): %s", from->ip, from->port, data);
    char *cmd = strtok(data, " ");
    if (strcmp(cmd, CMD[LOGIN]) == 0)
    {
        add_client(active_clients, from->ip, from->port);
    }
    else if (strcmp(cmd, CMD[LOGOUT]) == 0)
    {
        delete_client(active_clients, from->ip, from->port);
    }
    else if (strcmp(cmd, CMD[LIST]) == 0)
    {
        char buf[RECV_BUFSIZE] = {0};
        list2str(active_clients, buf);
        udp_send(serverfd, from, buf);
    }
    else if (strcmp(cmd, CMD[PUNCH]) == 0)
    {
        char *peer_addr = strtok(NULL, " ");
        char *peer_port = strtok(NULL, " ");
        if (peer_addr == NULL || peer_port == NULL) {
            LOG_TRACE("bad punch destination");
        } else {
            endpoint dest;
            strncpy(dest.ip, peer_addr, sizeof(dest.ip));
            dest.port = atoi(peer_port);
            char request[40] = {0};
            snprintf(request, 40, "%s %s %d", CMD[SYN], from->ip, from->port);
            udp_send(serverfd, &dest, request);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        LOG_ERROR("Usage: %s host port", argv[0]);
        return 1;
    }
    serverfd = create_udp_socket(argv[1], atoi(argv[2]));
    assert( serverfd != -1);
    LOG_TRACE("server listening on %s:%s", argv[1], argv[2]);

    active_clients = create_list();

    start_udp_server(serverfd, on_message);

    destroy_list(&active_clients);
    return 0;
}
