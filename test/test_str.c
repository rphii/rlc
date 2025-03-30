#include "../src/str.h"

int main(void) {
    RStr s = RSTR("Hello " F("World!", BOLD IT FG_BK_B) "\n");
    printf("[%.*s]%zu\n", RSTR_F(s), rstr_length(s));
    for(size_t i = 0; i < rstr_length_nof(s) + 10; ++i) {
        size_t n = rstr_index_nof(s, i);
        printf("[%zu/%zu] : %zu\n", i, rstr_length_nof(s), n);
        //printf("[%zu/%zu] : %zu [%c]\n", i, rstr_length_nof(s), n, rstr_get_at(&s, n));
    }
    return 0;
}

