#include "../src/str.h"

#define TEST_EQ(x, v) do { \
        printf("%s "); \
        if((x) == v) { \
            printf("OK\n"); \
        } else { \
            printf("FAIL\n"); \
        } \
    } while(0)


void test_str_rfind_substr(void) { /*{{{*/
    Str s = STR("hello, string this is a teststring!");
    RStr t = RSTR("string");
    printf("str_rfind_substr [%.*s] %zu\n", RSTR_F(t), str_rfind_substr(s, t));
    t = RSTR("string ");                             
    printf("str_rfind_substr [%.*s] %zu\n", RSTR_F(t), str_rfind_substr(s, t));
    t = RSTR("string!");                             
    printf("str_rfind_substr [%.*s] %zu\n", RSTR_F(t), str_rfind_substr(s, t));
    t = RSTR("string!!");                            
    printf("str_rfind_substr [%.*s] %zu\n", RSTR_F(t), str_rfind_substr(s, t));
} /*}}}*/   

void test_str_rfind_f(void) { /*{{{*/
    Str s = STR("Hello, " F("World", BOLD UL) "!");
    size_t i0, iE, l = str_length(s);
    i0 = str_rfind_f(s, &iE);
    printf("%s [%.*s] %zu/%zu/%zu\n", __func__, STR_F(s), i0, iE, l);

    Str t = STR_IE(s, i0);
    i0 = str_rfind_f(t, &iE);
    printf("%s [%.*s] %zu/%zu/%zu\n", __func__, STR_F(t), i0, iE, l);
} /*}}}*/

void test_str_length_nof(void){/*{{{*/
    RStr s = RSTR(F("Hello", BOLD) ", " F("World", IT) "!");
    size_t len = rstr_length_nof(s);
    printf("%s [%.*s] %zu\n", __func__, RSTR_F(s), len);
}/*}}}*/

int main(void) {

    test_str_rfind_substr();
    test_str_rfind_f();
    test_str_length_nof();
#if 0
    RStr s = RSTR("Hello " F("World!", BOLD IT FG_BK_B) "\n");
    printf("[%.*s]%zu\n", RSTR_F(s), rstr_length(s));
    for(size_t i = 0; i < rstr_length_nof(s) + 10; ++i) {
        size_t n = rstr_index_nof(s, i);
        printf("[%zu/%zu] : %zu\n", i, rstr_length_nof(s), n);
        //printf("[%zu/%zu] : %zu [%c]\n", i, rstr_length_nof(s), n, rstr_get_at(&s, n));
    }
#endif


    return 0;
}

