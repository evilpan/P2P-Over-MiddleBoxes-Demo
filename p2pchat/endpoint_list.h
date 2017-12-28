#ifndef P2P_ENDPOINT_LIST_H
#define P2P_ENDPOINT_LIST_H

#include "endpoint.h"
typedef struct _eplist_t eplist_t; 
struct _eplist_t {
    endpoint_t endpoint;
    time_t lastseen;
    eplist_t *next;
};

/*
 * create a new eplist_t *. Don't forget to destroy it with eplist_destroy()
 * */
eplist_t   *eplist_create();
void        eplist_destroy(eplist_t *head);
int         eplist_add(eplist_t *head, endpoint_t ep);
int         eplist_remove(eplist_t *head, endpoint_t ep);
int         eplist_count(eplist_t *head);
void        eplist_dump(eplist_t *head);
#endif
