#ifndef P2PCHAT_ENDPOINT_H
#define P2PCHAT_ENDPOINT_H
#include <netinet/in.h>
typedef struct sockaddr_in endpoint_t;
//struct _EndPoint {
//    // network byte order(big endian)
//    int ip;
//    short port;
//}/*__attribute__((packed))*/;

int ep_equal(endpoint_t lp, endpoint_t rp);

/* string is host:port format */
/* IPV4 ONLY */
char *ep_tostring(endpoint_t ep);
endpoint_t ep_fromstring(const char *tuple);
endpoint_t ep_frompair(const char *ip, short port);

#endif
