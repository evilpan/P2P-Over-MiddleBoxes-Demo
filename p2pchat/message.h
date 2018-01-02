#ifndef P2PCHAT_MESSAGE_H
#define P2PCHAT_MESSAGE_H
#include <stdint.h>
#include "endpoint.h"

#define MSG_MAGIC 0x8964 
#define MSG_MAGICLEN 2
#define MSG_TYPELEN 2
#define MSG_BODYLEN 4
#define MSG_HEADLEN MSG_MAGICLEN + MSG_TYPELEN + MSG_BODYLEN
/* a message is a UDP datagram with following structure:
   -----16bits--+---16bits--+-----32bits----------+---len*8bits---+
   --  0x8964   + msg type  + msg length(exclude) + message body  +
   -------------+-----------+---------------------+---------------+
*/
#define SEND_BUFSIZE 1024
#define RECV_BUFSIZE 1024
typedef enum _MessageType MessageType;
typedef struct _Message Message;
typedef struct _MessageHead MessageHead;
enum _MessageType {
    MTYPE_LOGIN = 0,
    MTYPE_LOGOUT,
    MTYPE_LIST,
    MTYPE_PUNCH,
    MTYPE_PING,
    MTYPE_PONG,
    MTYPE_REPLY,
    MTYPE_TEXT,
    MTYPE_END
};

struct _MessageHead {
    uint16_t magic;
    uint16_t type;
    uint32_t length;
}__attribute__((packed));

struct _Message {
    MessageHead head;
    const char *body;
};

const char *strmtype(MessageType type);
int msg_pack(Message msg, char *buf, unsigned int bufsize);
Message msg_unpack(const char *buf, unsigned int bufsize);

// replay a Message
int udp_send_msg(int sock, endpoint_t peer, Message msg);
// reply a buf with length
int udp_send_buf(int sock, endpoint_t peer, MessageType type,
        const char *buf, unsigned int len);
// reply a NULL terminated text
int udp_send_text(int sock, endpoint_t peer, MessageType type, const char *text);

#endif
