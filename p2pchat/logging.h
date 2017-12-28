#ifndef P2P_CHAT_LOGGING_H
#define P2P_CHAT_LOGGING_H
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>

#define LOGBUF 1024

#define log_debug(msg, args...) log_msg(DEBUG, msg, ##args)
#define log_info(msg, args...) log_msg(INFO, msg, ##args)
#define log_warn(msg, args...) log_msg(WARN, msg, ##args)
#define log_err(msg, args...) log_msg(ERROR, msg, ##args)
typedef enum _LogLevel {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR
} LogLevel;


const char *levelstr(LogLevel level);
void log_msg(LogLevel level, const char *msg, ...);
void log_setlevel(LogLevel level);
LogLevel log_getlevel();



#endif
