#include "../endpoint.h"
#include <assert.h>
#include <stdio.h>
static void test_pod() {
    endpoint_t ep = ep_fromstring("127.0.0.1:1234");
    assert(ep.sin_family == AF_INET);
    assert(ep.sin_addr.s_addr == inet_addr("127.0.0.1"));
    assert(ep.sin_port == htons(1234));

    endpoint_t ep1 = ep;
    assert(ep_equal(ep1, ep));
    ep1.sin_port = 1234;
    assert(!ep_equal(ep1, ep));
    printf("test_pod pass\n");
}

static void test_convert() {
    endpoint_t ep = ep_fromstring("127.0.0.1:1234");
    endpoint_t ep1 = ep_frompair("127.0.0.1", 1234);
    assert(ep_equal(ep1, ep));
    char *str = ep_tostring(ep1);
    assert(strcmp(str, "127.0.0.1:1234") == 0);
    printf("test_convert pass\n");
}

static void test_error() {
    endpoint_t ep;
    const char *testcase[] = {
        "",
        "xxxxx",
        "xxx:0",
        "xxx:yyy",
        NULL
    };
    for (int i = 0; testcase[i] ; i++) {
        ep = ep_fromstring(testcase[i]);
        assert(strcmp("255.255.255.255:0", ep_tostring(ep)) == 0);
    }
    printf("test_error pass\n");
}
int main()
{
    test_pod();
    test_convert();
    test_error();
    return 0;
}
