#include "../message.h"
#include <stdio.h>
#include <assert.h>

static void test_mtype() {
    unsigned int len = sizeof(unsigned int);
    assert(len == 4);
    assert(len == sizeof(MessageType));
    assert(len == sizeof(MTYPE_LOGIN));
    assert(len == sizeof(MTYPE_TEXT));
    assert(len == sizeof(MTYPE_END));
    printf("test_type pass\n");
}
static void test_msg() {
    assert(8 == sizeof(MessageHead));
    assert(8+8 == sizeof(Message));
    MessageHead head;
    Message m1, m2;

    head.magic = 0x8964;
    head.type = 2;
    head.length = 4;
    m1.head = head;
    m1.body = "hello";
    m2 = m1;
    printf("m1: {0x%x,%d,%d} %p\n",
            m1.head.magic, m1.head.type, m1.head.length,
            m1.body);
    printf("m2: {0x%x,%d,%d} %p\n",
            m2.head.magic, m2.head.type, m2.head.length,
            m2.body);
    assert(m2.head.magic == 0x8964);
    assert(m2.head.type == 2);
    assert(m2.head.length == 4);
    // the pointer is copied
    assert((long)m2.body == (long)m1.body);
    printf("test_msg pass\n");
}
int main()
{
    test_mtype();
    test_msg();
    return 0;
}
