#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <pthread.h>
#include <assert.h>
#include "logging.h"
#include "message.h"
#include "endpoint.h"
#include "endpoint_list.h"
#define PING_INTERVAL 10

static int quiting = 0;
static int g_clientfd;
static endpoint_t g_server;
static eplist_t *g_peers;
void *keepalive_loop();
void *receive_loop();
void *console_loop();

static void quit() {
    quiting = 1;
}

void *keepalive_loop() {
    Message ping;
    ping.head.magic = MSG_MAGIC;
    ping.head.type = MTYPE_PING;
    ping.head.length = 0;
    ping.body = NULL;
    unsigned int i = 0;
    while(!quiting) {
        // quit ASAP
        if (i++ < PING_INTERVAL) {
            sleep(1);
            continue;
        }
        i = 0;
        udp_send_msg(g_clientfd, g_server, ping);
        for (eplist_t *ep = g_peers->next; ep != NULL; ep = ep->next) {
            udp_send_msg(g_clientfd, ep->endpoint, ping);
        }
    }
    log_info("quiting keepalive_loop");
    return NULL;
}

void on_message(endpoint_t from, Message msg) {
    log_debug("RECV %d bytes FROM %s: %s %s", msg.head.length,
            ep_tostring(from), strmtype(msg.head.type), msg.body);
    // from server
    if (ep_equal(g_server, from)) {
        switch (msg.head.type) {
            case MTYPE_PUNCH:
                { 
                    endpoint_t peer = ep_fromstring(msg.body); 
                    log_info("%s on call, replying...", ep_tostring(peer));
                    udp_send_text(g_clientfd, peer, MTYPE_REPLY, NULL);
                }
                break;
            case MTYPE_REPLY:
                log_info("SERVER: %s", msg.body);
                break;
            default:
                break;
        }
        return;
    }
    // from peer
    switch (msg.head.type) {
        case MTYPE_TEXT:
            log_info("Peer(%s): %s", ep_tostring(from), msg.body);
            break;
        case MTYPE_REPLY:
            log_info("Peer(%s) replied, you can talk now", ep_tostring(from));
            eplist_add(g_peers, from);
        case MTYPE_PUNCH:
            /*
             * Usually we can't recevie punch request from other peer directly,
             * but it could happen when it come after we reply the punch request from server,
             * or there's a tunnel already.
             * */
            udp_send_text(g_clientfd, from, MTYPE_TEXT, "I SEE YOU");
            break;
        case MTYPE_PING:
            udp_send_text(g_clientfd, from, MTYPE_PONG, NULL);
        default:
            break;
    }
}
void *receive_loop() {
    endpoint_t peer;
    socklen_t addrlen;
    char buf[RECV_BUFSIZE];
    int nfds;
    fd_set readfds;
    struct timeval timeout;

    nfds = g_clientfd + 1;
    while(!quiting) {
        FD_ZERO(&readfds);
        FD_SET(g_clientfd, &readfds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int ret = select(nfds, &readfds, NULL, NULL, &timeout);
        if (ret == 0) {
            /* timeout */
            continue;
        }
        else if (ret == -1) {
            perror("select");
            continue;
        }
        assert(FD_ISSET(g_clientfd, &readfds));
        addrlen = sizeof(peer);
        memset(&peer, 0, addrlen);
        memset(buf, 0, RECV_BUFSIZE);
        int rd_size = recvfrom(g_clientfd, buf, RECV_BUFSIZE, 0,
                (struct sockaddr*)&peer, &addrlen);
        if (rd_size == -1) {
            perror("recvfrom");
            continue;
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
        on_message(peer, msg);
    }
    log_info("quiting receive_loop");
    return NULL;
}

static void print_help()
{
    const static char *help_message = ""
        "Usage:"
        "\n\n login"
        "\n     login to server so that other peer(s) can see you"
        "\n\n logout"
        "\n     logout from server"
        "\n\n list"
        "\n     list logined peers"
        "\n\n punch host:port"
        "\n     punch a hole through UDP to [host:port]"
        "\n     host:port must have been logged in to server"
        "\n     Example:"
        "\n     >>> punch 9.8.8.8:53"
        "\n\n send host:port data"
        "\n     send [data] to peer [host:port] through UDP protocol"
        "\n     the other peer could receive your message if UDP hole punching succeed"
        "\n     Example:"
        "\n     >>> send 8.8.8.8:53 hello"
        "\n\n help"
        "\n     print this help message"
        "\n\n quit"
        "\n     logout and quit this program";
    printf("%s\n", help_message);
}
void *console_loop() {
    char *line = NULL;
    size_t len;
    ssize_t read;
    while(fprintf(stdout, ">>> ") && (read = getline(&line, &len, stdin)) != -1) {
        /* ignore empty line */
        if (read == 1) continue;
        char *cmd = strtok(line, " ");
        if (strncmp(cmd, "list", 4) == 0) {
            udp_send_text(g_clientfd, g_server, MTYPE_LIST, NULL);
        } else if (strncmp(cmd, "login", 5) == 0) {
            udp_send_text(g_clientfd, g_server, MTYPE_LOGIN, NULL);
        } else if (strncmp(cmd, "logout", 5) == 0) {
            udp_send_text(g_clientfd, g_server, MTYPE_LOGOUT, NULL);
        } else if (strncmp(cmd, "punch", 5) == 0) {
            char *host_port = strtok(NULL, "\n");
            endpoint_t peer = ep_fromstring(host_port);
            log_info("punching %s", ep_tostring(peer));
            udp_send_text(g_clientfd, peer, MTYPE_PUNCH, NULL);
            udp_send_text(g_clientfd, g_server, MTYPE_PUNCH, host_port);
        } else if (strncmp(cmd, "send", 4) == 0) {
            char *host_port = strtok(NULL, " ");
            char *data = strtok(NULL, "\n");
            udp_send_text(g_clientfd, ep_fromstring(host_port), MTYPE_TEXT, data);
        } else if (strncmp(cmd, "help", 4) == 0) {
            print_help();
        } else if (strncmp(cmd, "quit", 4) == 0) {
            udp_send_text(g_clientfd, g_server, MTYPE_LOGOUT, NULL);
            quit();
            break;
        } else {
            printf("Unknown command %s\n", cmd);
            print_help();
        }
    }
    free(line);
    log_info("quiting console_loop");
    return NULL;
}

int main(int argc, char **argv)
{
    log_setlevel(INFO);
    if (argc != 2) {
        fprintf(stderr, "Usage: %s server:port\n", argv[0]);
        return 1;
    }
    int ret;
    pthread_t keepalive_pid, receive_pid, console_pid;

    g_server = ep_fromstring(argv[1]);
    g_peers = eplist_create();
    g_clientfd = socket(AF_INET, SOCK_DGRAM, 0);
    log_info("setting server to %s", ep_tostring(g_server));
    if (g_clientfd == -1) { perror("socket"); goto clean; }
    ret = pthread_create(&keepalive_pid, NULL, &keepalive_loop, NULL);
    if (ret != 0) { perror("keepalive"); goto clean; }
    ret = pthread_create(&receive_pid, NULL, &receive_loop, NULL);
    if (ret != 0) { perror("receive"); goto clean; }
    ret = pthread_create(&console_pid, NULL, &console_loop, NULL);
    if (ret != 0) { perror("console"); goto clean; }

    pthread_join(console_pid, NULL);
    pthread_join(receive_pid, NULL);
    pthread_join(keepalive_pid, NULL);
clean:
    close(g_clientfd);
    eplist_destroy(g_peers);
    return 0;
}
