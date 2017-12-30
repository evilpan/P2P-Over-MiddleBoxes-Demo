#include "endpoint.h"
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define INET_PORTSTRLEN 5
#define TUPLE_LEN (INET_ADDRSTRLEN + INET_PORTSTRLEN + 1)
int ep_equal(endpoint_t lp, endpoint_t rp) {
    return ( (lp.sin_family == rp.sin_family) &&
        (lp.sin_addr.s_addr == rp.sin_addr.s_addr) &&
        (lp.sin_port == rp.sin_port) );
}

/* NOT THREAD SAFE */
char *ep_tostring(endpoint_t ep) {
    static char tuple[TUPLE_LEN];
    snprintf(tuple, TUPLE_LEN, "%s:%d",
            inet_ntoa(ep.sin_addr),
            ntohs(ep.sin_port));
    return tuple;
}

endpoint_t ep_fromstring(const char *tuple) {
    char _tuple[TUPLE_LEN];
    char *host = NULL;
    char *port = NULL;
    sprintf(_tuple, "%s", tuple);
    host = strtok(_tuple, ":");
    port = strtok(NULL, ":");
    if (host == NULL || port == NULL) {
        host = "255.255.255.255";
        port = "0";
    }
    return ep_frompair(host, atoi(port));
}

endpoint_t ep_frompair(const char *host, short port) {
    endpoint_t ep;
    memset(&ep, 0, sizeof ep);
    ep.sin_family = AF_INET;
    ep.sin_addr.s_addr = inet_addr(host);
    ep.sin_port = htons(port);
    return ep;
}
