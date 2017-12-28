#include <stdio.h>
#include <assert.h>
#include "../endpoint.h"
#include "../endpoint_list.h"
/*
 * Test with valgrind
 * */
void test_basic() {
    eplist_t *pool = eplist_create();
    assert(0 == eplist_count(pool));

    eplist_add(pool, ep_fromstring("192.168.0.1:3389"));
    eplist_add(pool, ep_fromstring("127.0.0.1:1234"));
    eplist_add(pool, ep_fromstring("127.0.0.1:3389"));

    printf("=== 3 ===\n");
    assert(3 == eplist_count(pool));
    eplist_dump(pool);

    // remove center
    eplist_remove(pool, ep_fromstring("127.0.0.1:1234"));
    printf("=== 2 ===\n");
    assert(2 == eplist_count(pool));
    eplist_dump(pool);

    // remove head
    eplist_remove(pool, ep_fromstring("192.168.0.1:3389"));
    printf("=== 1 ===\n");
    assert(1 == eplist_count(pool));
    eplist_dump(pool);

    // remove tail
    eplist_remove(pool, ep_fromstring("127.0.0.1:3389"));
    printf("=== 0 ===\n");
    assert(0 == eplist_count(pool));
    eplist_dump(pool);

    eplist_destroy(pool);
    printf("test_basic pass\n");
}

void test_corner() {
    eplist_t *pool = NULL;
    assert (-1 == eplist_count(pool));
    pool = eplist_create();
    assert(0 == eplist_add(pool, ep_fromstring("0:80")));
    assert(0 != eplist_add(pool, ep_fromstring("0:80")));
    assert(1 == eplist_count(pool));
    printf("test_corner pass\n");
    eplist_destroy(pool);
}
int main()
{
    test_basic();
    test_corner();
    return 0;
}
