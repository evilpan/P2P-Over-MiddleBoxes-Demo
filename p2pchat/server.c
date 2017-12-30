#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "message.h"
#include "logging.h"
#include "endpoint.h"
#include "endpoint_list.h"

#define MAX_CLIENTS 10
typedef void callback_t(int server_sock, endpoint_t from, Message msg);
eplist_t *g_client_pool;

void udp_receive_loop(int listen_sock, callback_t callback)
{
    endpoint_t peer;
    socklen_t addrlen;
    char buf[RECV_BUFSIZE];
    for(;;) {
        addrlen = sizeof(peer);
        memset(&peer, 0, addrlen);
        memset(buf, 0, RECV_BUFSIZE);
        int rd_size;
        /* UDP isn't a "stream" protocol. once you do the initial recvfrom,
           the remainder of the packet is discarded */
        rd_size = recvfrom(listen_sock, buf, RECV_BUFSIZE, 0,
                (struct sockaddr*)&peer, &addrlen);
        if (rd_size == -1) {
            perror("recvfrom");
            break;
        } else if (rd_size == 0) {
            log_info("EOF from %s", ep_tostring(peer));
            continue;
        }
        Message msg = msg_unpack(buf, rd_size);
        if (msg.head.magic != MSG_MAGIC || msg.body == NULL) {
            log_warn("Invalid message(%d bytes): {0x%x,%d,%d} %p", rd_size,
                    msg.head.magic, msg.head.type, msg.head.length, msg.body);
            continue;
        }
        callback(listen_sock, peer, msg);
        continue;

    }
    log_info("udp_receive_loop stopped.");
}

void on_message(int sock, endpoint_t from, Message msg) {
    log_debug("RECV %d bytes FROM %s: %s %s", msg.head.length,
            ep_tostring(from), strmtype(msg.head.type), msg.body);
    switch(msg.head.type) {
        case MTYPE_LOGIN:
            {
                if (0 == eplist_add(g_client_pool, from)) {
                    log_info("%s logged in", ep_tostring(from));
                    udp_send_text(sock, from, MTYPE_REPLY, "Login success!");
                } else {
                    log_warn("%s failed to login", ep_tostring(from));
                    udp_send_text(sock, from, MTYPE_REPLY, "Login failed");
                }
            }
            break;
        case MTYPE_LOGOUT:
            {
                if (0 == eplist_remove(g_client_pool, from)) {
                    log_info("%s logged out", ep_tostring(from));
                    udp_send_text(sock, from, MTYPE_REPLY, "Logout success");
                } else {
                    log_info("%s failed to logout", ep_tostring(from));
                    udp_send_text(sock, from, MTYPE_REPLY, "Logout failed");
                }
            }
            break;
        case MTYPE_LIST:
            {
                log_info("%s quering list", ep_tostring(from));
                char text[SEND_BUFSIZE - MSG_HEADLEN] = {0};
                for (eplist_t *c = g_client_pool->next; c != NULL; c = c->next) {
                    if (ep_equal(c->endpoint, from)) strcat(text, "(you)");
                    strcat(text, ep_tostring(c->endpoint));
                    if (c->next) strcat(text, ";");
                }
                udp_send_text(sock, from, MTYPE_REPLY, text);
            }
            break;
        case MTYPE_PUNCH:
            {
                endpoint_t other = ep_fromstring(msg.body);
                log_info("punching to %s", ep_tostring(other));
                udp_send_text(sock, other, MTYPE_PUNCH, ep_tostring(from));
                udp_send_text(sock, from, MTYPE_TEXT, "punch request sent");
            }
            break;
        case MTYPE_PING:
            udp_send_text(sock, from, MTYPE_PONG, NULL);
            break;
        case MTYPE_PONG:
            break;
        default:
            udp_send_text(sock, from, MTYPE_REPLY, "Unkown command");
            break;
    }
}

int main(int argc, char **argv) {
    log_setlevel(DEBUG);
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    const char *host = "0.0.0.0";
    int port = atoi(argv[1]);
    int ret;
    endpoint_t server = ep_frompair(host, port);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    ret = bind(sock, (const struct sockaddr*)&server, sizeof(server));
    if (ret == -1) { 
        perror("bind");
        exit(EXIT_FAILURE);
    }
    g_client_pool = eplist_create();

    log_info("server start on %s", ep_tostring(server));
    udp_receive_loop(sock, on_message);

    eplist_destroy(g_client_pool);
    return EXIT_SUCCESS;
}
