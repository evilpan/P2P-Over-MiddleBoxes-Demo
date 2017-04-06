#ifndef P2PDEMO_SERVER_LOGIC_H
#define P2PDEMO_SERVER_LOGIC_H

#include "common.h"
typedef struct client_list client_list;
struct client_list {
    endpoint *client;
    client_list *next;
};


client_list *create_list();
void destroy_list(client_list **head);

int add_client(client_list *head, const char *ip, int port);
int delete_client(client_list *head, const char *ip, int port);
int list2str(client_list *head, char *str);
#endif
