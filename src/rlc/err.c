#include "err.h"
#include <stdarg.h>
#include <execinfo.h>
#include <time.h>

#define RLC_TRACE_MAX   128

void Assert_x(int result, char *x, const char *file, int line, const char *func, char *fmt, ...) {
    char buf[4096];
    if(!result) {
        va_list va;
        va_start(va, fmt);
        vsnprintf(buf, 4096, fmt, va);
        va_end(va);
        ERR_PRINTF(F("[ABORT]", BOLD FG_BK BG_RD_B) " assertion of '%s' failed... " F("%s:%d:%s (end of trace) %s", FG_WT_B) " " "\n" , x, file, line, func, buf);
        rlc_trace();
        exit(-1);
    }
}

void rlc_trace(void) {
    void *array[RLC_TRACE_MAX];
    char **strings;
    int size, i;
    size = backtrace(array, RLC_TRACE_MAX);
    strings = backtrace_symbols(array, size);
    if(strings) {
        printf(F("*", FG_RD_B BG_BK BOLD) " Obtained %d stack frames:\n", size);
        for(i = 0; i < size; i++) {
            printf(F("*", FG_RD_B BG_BK BOLD) " %-3u * %s\n", i, strings[i]);
        }
        free(strings);
    }
    struct timespec tp;
    if(!clock_gettime(CLOCK_REALTIME, &tp)) {
        struct tm *tm = localtime(&tp.tv_sec);
        char buffer[32];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
        printf(F("%s", FG_RD_B BG_BK BOLD), buffer);
    } else {
        printf(F("!", FG_RD_B BG_BK BOLD));
    }
    switch(tp.tv_sec % 4) {
        case 0: printf(" Bailing out, sorry. (end of trace)\n"); break;
        case 1: printf(" Giving up, sorry. (end of trace)\n"); break;
        case 2: printf(" Sincerely, yours truly. (end of trace)\n"); break;
        default: printf(" Try rebooting, please. (end of trace)\n"); break;
    }
}


