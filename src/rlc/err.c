#include "err.h"
#include <stdarg.h>
#include <execinfo.h>

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
        printf("Obtained %d stack frames.\n", size);
        for(i = 0; i < size; i++) {
            printf("%s\n", strings[i]);
        }
    }
    free(strings);
}


