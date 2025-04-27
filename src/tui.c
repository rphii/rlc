#include "tui.h"
#include "platform-detect.h"

#include <stdlib.h>
#include <unistd.h>
#include <termios.h>


ErrDecl tui_colorprint_init(void)
{
    int err = 0;
#if defined(PLATFORM_WINDOWS) && !defined(COLORPRINT_DISABLE)
    err = system("chcp 65001 >nul");
    if(err) {
        THROW("failed enabling utf-8 codepage. Try compiling with -D" S(PLATFORM_DISABLE) "");
    }
clean:
    return err;
error:
    ERR_CLEAN;
#else
    return err;
#endif
}

int tui_getch(void)
{
    //printf(F("[press any key] ", IT FG_CY_B));
    fflush(stdout);

#if defined(PLATFORM_WINDOWS)

    return _getch();

#else

    int buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if(tcgetattr(0, &old) < 0) {
        perror("tcsetattr()");
    }
    old.c_lflag &= (tcflag_t)~ICANON;
    old.c_lflag &= (tcflag_t)~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0) {
        perror("tcsetattr ICANON");
    }
    if(read(0, &buf, 1) < 0) {
        perror("read()");
    }
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0) {
        perror("tcsetattr ~ICANON");
    }
    //printf("%c", buf);
    return buf;

#endif
}

void tui_cursor_hide(void) {
    printf("\e[?25l");
    fflush(stdout);
}

void tui_cursor_show(void) {
    printf("\e[?25h");
    fflush(stdout);

    struct termios term;
    tcgetattr(fileno(stdin), &term);
    term.c_lflag |= ECHO;
    tcsetattr(fileno(stdin), 0, &term);

}

void tui_buffer_enter(void) {
    system("tput rmcup"); // hide fg
}

void tui_buffer_exit(void) {
    system("tput smcup"); // enter fg
}


void tui_clear(void)
{
    // printf("\033[H\033[J""\033[H\033[J");
    // TODO - is it ok to just call it once ??
    printf("\033[H\033[J");
}



