#include "err.h"
#include <stdarg.h>

void Assert_x(int result, char *x, const char *file, int line, const char *func, char *fmt, ...) {
    char buf[4096];
    if(!result) {
        va_list va;
        va_start(va, fmt);
        vsnprintf(buf, 4096, fmt, va);
        va_end(va);
        ERR_PRINTF(F("[ABORT]", BOLD FG_BK BG_RD_B) " assertion of '%s' failed... " F("%s:%d:%s (end of trace) %s", FG_WT_B) " " "\n" , x, file, line, func, buf);
        exit(-1);
    }
}

