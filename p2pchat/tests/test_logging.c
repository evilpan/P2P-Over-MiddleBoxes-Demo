#include "../logging.h"

int main() {
    int i = 0;
    log_setlevel(DEBUG);
    log_debug("hello %d", i++);
    log_info("hello %d", i++);
    log_warn("hello %d", i++);
    log_err("hello %d", i++);
    log_setlevel(INFO);
    log_debug("hello %d", i++);
    log_info("hello %d", i++);
    log_warn("hello %d", i++);
    log_err("hello %d", i++);
    log_setlevel(WARN);
    log_debug("hello %d", i++);
    log_info("hello %d", i++);
    log_warn("hello %d", i++);
    log_err("hello %d", i++);
    return 0;
}
