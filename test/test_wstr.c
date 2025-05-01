#include <rphii/wstr.h>
#include <rphii/utf8.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>

struct termios restore;
struct termios term;

int get_pos(int *x, int *y) {

    fflush(stdout);
    char buf[30]={0};
    int ret, i, pow;
    char ch;

    *y = 0; *x = 0;

    tcsetattr(0, TCSANOW, &term);

    write(1, "\033[6n", 4);

    for( i = 0, ch = 0; ch != 'R'; i++ )
    {
        ret = read(0, &ch, 1);
        if ( !ret ) {
            tcsetattr(0, TCSANOW, &restore);
            fprintf(stderr, "getpos: error reading response!\n");
            return 1;
        }
        buf[i] = ch;
    }

    if (i < 2) {
        tcsetattr(0, TCSANOW, &restore);
        return(1);
    }

    for( i -= 2, pow = 1; buf[i] != ';'; i--, pow *= 10)
        *x = *x + ( buf[i] - '0' ) * pow;

    for( i-- , pow = 1; buf[i] != '['; i--, pow *= 10)
        *y = *y + ( buf[i] - '0' ) * pow;

    tcsetattr(0, TCSANOW, &restore);
    return 0;
}

int main(void) {


    tcgetattr(0, &term);
    tcgetattr(0, &restore);
    term.c_lflag &= ~(ICANON|ECHO);

    WStr w = {0};
    TRYC(wstr_from_rstr(&w, RSTR("ｕｎｉｘ")));
    wprintf(L"%.*ls=%zu\n", WSTR_F(w), wstr_wlength(w));
    TRYC(wstr_from_rstr(&w, RSTR("Stéphane")));
    wprintf(L"%.*ls=%zu\n", WSTR_F(w), wstr_wlength(w));
    TRYC(wstr_from_rstr(&w, RSTR("もで 諤奯ゞ")));
    wprintf(L"%.*ls=%zu\n", WSTR_F(w), wstr_wlength(w));
    TRYC(wstr_from_rstr(&w, RSTR("👍😎🤙")));
    wprintf(L"%.*ls=%zu\n", WSTR_F(w), wstr_wlength(w));

    size_t total = 0, fail = 0, ok = 0;
    for(size_t i = 32; i < 0x1fb00; ++i) {
        U8Point p = { .val = i};
        U8Str s = {0};
        cstr_from_u8_point(s, &p);
        //printf("%.*s:%u\n", p.bytes, s, p.bytes);
        if(wstr_from_rstr(&w, RSTR_LL(s, p.bytes))) continue;
        if(wstr_wlength(w) == (size_t)-1) continue;
        int x1, y1, x2, y2;
        printf("0x%04zx:", i);
        get_pos(&x1, &y1);
        wprintf(L"%.*ls", WSTR_F(w));
        get_pos(&x2, &y2);
        printf("=%zu", wstr_wlength(w));
        bool check = false;
        if(x2-x1==wstr_wlength(w)) {
            check = true;
            ++ok;
        }
        else ++fail;
        ++total;
        printf(" %s\n", check?F("OK", FG_GN_B):F("FAIL", FG_RD_B));
        //wprintf(L"%.*ls=%zu ", WSTR_F(w), wstr_wlength(w));
    }
    printf("total %zu ok %zu fail %zu\n", total, ok, fail);

error:
    wstr_free(&w);
    return 0;
}

