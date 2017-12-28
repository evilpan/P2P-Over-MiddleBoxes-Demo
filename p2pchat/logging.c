#include "logging.h"

static LogLevel CURRENT_LEVEL = INFO;
void log_setlevel(LogLevel level) {
    CURRENT_LEVEL = level;
}
LogLevel log_getlevel() {
    return CURRENT_LEVEL;
}
const char *levelstr(LogLevel level) {
    switch (level) {
        case DEBUG:
            return "DBG";
        case INFO:
            return "INF";
        case WARN:
            return "WAR";
        case ERROR:
            return "ERR";
        default:
            return "???";
    }
}
void log_msg(LogLevel level, const char *msg, ...) {
    if (level < CURRENT_LEVEL) return;
    va_list ap;
    va_start(ap, msg);
    char fmt[LOGBUF] = {0};
    char timestamp[26];
    struct timeb tp;
    ftime(&tp);
    struct tm *st = localtime(&tp.time);
    sprintf(timestamp, "%02d:%02d:%02d.%03d",
            st->tm_hour, st->tm_min, st->tm_sec, tp.millitm);
    sprintf(fmt, "%s %s: ", timestamp, levelstr(level));
    strcat(fmt, msg);
    strcat(fmt, "\n");
    vprintf(fmt, ap);
    va_end(ap);
}
