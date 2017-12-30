#include "message.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

const char *strmtype(MessageType type) {
    switch(type) {
        case MTYPE_LOGIN:   return "LOGIN";
        case MTYPE_LOGOUT:  return "LOGOUT";
        case MTYPE_LIST:    return "LIST";
        case MTYPE_PUNCH:   return "PUNCH";
        case MTYPE_PING:    return "PING";
        case MTYPE_PONG:    return "PONG";
        case MTYPE_REPLY:   return "REPLY";
        case MTYPE_TEXT:    return "TEXT";
        default:            return "UNKNOW";
    }
}

/* return bytes serialized */
int msg_pack(Message msg, char *buf, unsigned int bufsize) {
    if (bufsize < MSG_HEADLEN + msg.head.length) {
        printf("buf too small");
        return 0;
    }
    int16_t m_magic = htons(msg.head.magic);
    int16_t m_type = htons(msg.head.type);
    int32_t m_length = htonl(msg.head.length);
    int index = 0;
    memcpy(buf + index, &m_magic, MSG_MAGICLEN);
    index += MSG_MAGICLEN;
    memcpy(buf + index, &m_type, MSG_TYPELEN);
    index += MSG_TYPELEN;
    memcpy(buf + index, &m_length, MSG_BODYLEN);
    index += MSG_BODYLEN;
    memcpy(buf + index, msg.body, msg.head.length);
    index += msg.head.length;
    return index;
}

/*
   Message body is a pointer to buf + len(head)
*/
Message msg_unpack(const char *buf, unsigned int buflen) {
    Message m;
    memset(&m, 0, sizeof(m));
    if (buflen < MSG_HEADLEN) {
        // at least we won't get an overflow
        return m;
    }
    int index = 0;
    m.head.magic = ntohs(*(uint16_t *)(buf + index));
    index += sizeof(uint16_t);
    if (m.head.magic != MSG_MAGIC) {
        return m;
    }
    m.head.type = ntohs(*(uint16_t *)(buf + index));
    index += sizeof(uint16_t);
    m.head.length = ntohl(*(uint32_t *)(buf + index));
    index += sizeof(uint32_t);
    if (index + m.head.length > buflen) {
        printf("message declared body size(%d) is larger than what's received (%d), truncating\n",
                m.head.length, buflen - MSG_HEADLEN);
        m.head.length = buflen - index;
    }
    m.body = buf + index;
    return m;
}

// send a Message
int udp_send_msg(int sock, endpoint_t peer, Message msg) {
    char buf[SEND_BUFSIZE] = {0};
    int wt_size = msg_pack(msg, buf, SEND_BUFSIZE);
    return sendto(sock, buf, wt_size,
            MSG_DONTWAIT, (struct sockaddr *)&peer, sizeof(peer));
}
// send a buf with length
int udp_send_buf(int sock, endpoint_t peer,
        MessageType type, const char *buf, unsigned int len) {
    Message m;
    m.head.magic = MSG_MAGIC;
    m.head.type = type;
    m.head.length = len;
    m.body = buf;
    return udp_send_msg(sock, peer, m);
}
// send a NULL terminated text
int udp_send_text(int sock, endpoint_t peer,
        MessageType type, const char *text) {
    unsigned int len = text == NULL ? 0 : strlen(text);
    return udp_send_buf(sock, peer, type, text, len);
}
