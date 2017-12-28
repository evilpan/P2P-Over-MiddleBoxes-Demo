#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <arpa/inet.h>
#include "endpoint_list.h"

// head is dummpy
eplist_t *eplist_create() {
    eplist_t *head = (eplist_t *)malloc(sizeof(eplist_t));
    head->lastseen = -1;
    head->next = NULL;
    return head;
}

void eplist_destroy(eplist_t *head) {
    if (head == NULL) return;
    eplist_destroy(head->next);
    head->next = NULL;
    free(head);
}

int eplist_add(eplist_t *head, endpoint_t ep) {
    eplist_t *current;
    for (current = head; current != NULL; current = current->next) {
        if (current != head && ep_equal(ep, current->endpoint)) {
            // do not add existing endpoint
            return 1;
        } else if (current->next == NULL) {
            eplist_t *newep = (eplist_t *)malloc(sizeof(eplist_t));
            newep->lastseen = time(NULL);
            newep->endpoint = ep;
            newep->next = NULL;
            current->next = newep;
            return 0;
        }
    }
    /* shouldn't be here */
    assert(1);
    return 1;
}

int eplist_remove(eplist_t *head, endpoint_t ep) {
    eplist_t *current;
    for (current = head;
            current != NULL && current->next != NULL;
            current = current->next) {
        if (ep_equal(ep, current->next->endpoint)) {
            eplist_t *temp = current->next;
            current->next = temp->next;
            free(temp);
            return 0;
        }
    }
    return 1;
}

int eplist_count(eplist_t *head) {
    int i = -1;
    for (eplist_t *current = head; current != NULL; current = current->next) {
        i++;
    }
    return i;
}

void eplist_dump(eplist_t *head) {
    if (head == NULL) return;

    eplist_t *current;
    for (current = head->next; current != NULL; current = current->next) {
        printf("%s(%ld)%s", ep_tostring(current->endpoint),
                current->lastseen, current->next ? "->" : ";\n");
    }
}
