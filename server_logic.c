#include "server_logic.h"
#include <stdlib.h>
client_list *create_list()
{
    client_list *head = (client_list *)malloc(sizeof(client_list));
    head->client = NULL;
    head->next = NULL;
    return head;
}

void destroy_list(client_list **head)
{
    if (*head == NULL)
        return;

    if ((*head)->client != NULL) {
        free((*head)->client);
        (*head)->client = NULL;
    }
    destroy_list( &(*head)->next );
    (*head)->next = NULL;

    free(*head);
    *head = NULL;
}

int add_client(client_list *head, const char *ip, int port)
{
    if (head == NULL) return -1;
    client_list *pos;
    for (pos = head; pos != NULL; pos = pos->next) {
        if (pos->client != NULL &&
            strncmp(pos->client->ip, ip, sizeof(pos->client->ip)) == 0 &&
            pos->client->port == port) {
            LOG_TRACE("client(%s:%d) exists", ip, port);
            return 1;
        }
        if (pos->next == NULL) {
            endpoint *new_client = calloc(1, sizeof(endpoint));
            client_list *node = create_list();
            strncpy(new_client->ip, ip, sizeof(new_client->ip)); /* calloc guarantee null terminated */
            new_client->port = port;
            node->client = new_client;
            node->next = NULL;
            pos->next = node;
            break;
        }
    }
    return 0;
}
int delete_client(client_list *head, const char *ip, int port)
{
    if (head == NULL) return -1;
    int found = 0;
    client_list *pos;
    for (pos = head; pos != NULL; pos = pos->next) {
        client_list *next = pos->next;
        if (next == NULL) break;
        if ( (strncmp(next->client->ip, ip, sizeof(next->client->ip)) == 0) &&
                next->client->port == port )
        {
            pos->next = next->next;
            free(next->client);
            free(next);
            found = 1;
            break;
        }
    }
    if (found)
        return 0;
    LOG_TRACE("client(%s:%d) not found", ip, port);
    return -1;

}

int list2str(client_list *head, char *str)
{
    if (head == NULL) return 0;

    char buf[23] = {0};
    int total = 0;
    str[0] = 0;
    for (client_list *c = head->next; c != NULL;
            c = c->next)
    {
        snprintf(buf, 23, "(%s %d)", c->client->ip, c->client->port);
        //printf("one: %s\n", buf);
        strcat(str, buf);
        total ++;
    }
    return total;
}
