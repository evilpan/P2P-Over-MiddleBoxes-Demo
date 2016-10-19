#ifndef _P2P_UTILS_H
#define _P2P_UTILS_H

#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

#define RECV_BUFSIZE 1024

#include <fcntl.h>
#include <stdio.h>
bool set_nonblocking(int sockfd)
{
    int flags, s;

    flags = fcntl (sockfd, F_GETFL, 0);
    if (flags == -1) {
        fprintf(stderr, "ERROR: fcntl get\n");
        return false;
    }
    flags |= O_NONBLOCK;
    s = fcntl (sockfd, F_SETFL, flags);
    if (s == -1) {
        fprintf(stderr, "ERROR: fcntl set\n");
        return false;
    }
    return true;
}
#endif
