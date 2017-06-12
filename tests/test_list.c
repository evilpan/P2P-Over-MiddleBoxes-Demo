#include <assert.h>
#include <stdio.h>

#include "../server_logic.h"

void test_ep()
{
    endpoint ep;
    printf("sizeof ep = %lu\n", sizeof(ep) );
    printf("sizeof struct = %lu\n", sizeof(endpoint));
    printf("sizeof ep.ip = %lu\n", sizeof(ep.ip));
    assert( sizeof(ep) == sizeof(endpoint) );
    assert( sizeof(ep.ip) == INET_ADDRSTRLEN );
    assert( sizeof(ep) == sizeof(ep.ip) + sizeof(ep.port) );

}
void test_list()
{
    char buf[1024] = {0};
    int total = 0;

    client_list *head = create_list();
    assert( 0 == (total = list2str(head, buf)) );

    add_client(head, "127.0.0.1", 1111);
    assert( 1 == (total = list2str(head, buf)) );
    printf("total %d nodes:\n%s\n", total, buf);

    add_client(head, "127.0.0.1", 2222);
    add_client(head, "127.0.0.1", 3333);
    add_client(head, "127.0.0.1", 4444);
    assert( 4 == (total = list2str(head, buf)) );
    printf("total %d nodes:\n%s\n", total, buf);

    assert( 1 == add_client(head, "127.0.0.1", 4444) );
    assert( 4 == (total = list2str(head, buf)) );

    delete_client(head, "127.0.0.1", 1111);
    delete_client(head, "127.0.0.1", 4444);
    assert( 2 == (total = list2str(head, buf)) );
    printf("total %d nodes:\n%s\n", total, buf);

    destroy_list(&head);
    assert( head == NULL );
    assert( 0 == (total = list2str(head, buf)) );

}

int main()
{

    test_ep();
    test_list();
    return 0;
}
