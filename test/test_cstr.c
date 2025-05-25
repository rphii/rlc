#include "../src/str2.h"
#include <stdio.h>
#include <stdlib.h>
#include "../src/err.h"
#include <stdint.h>

int main(void) {
    Str2 a = {0};
    Str2 b = str2("HOME path: /home/rphii");
    Str2 c = str2("HOME path!");
    //str2_l(getenv("HOME"));
    str2_fmt(&a, "HOME path: ");
    printff("%zu[%.*s]", str2_len(a), STR2_F(a));
    str2_fmt(&a, "%s", getenv("HOME"));
    printff("%zu[%.*s]", str2_len(a), STR2_F(a));
    printff("first %c@%zu:%zu", '/', str2_find_ch(a, '/'), str2_len(a));
    printff("last %c@%zu:%zu", '/', str2_rfind_ch(a, '/'), str2_len(a));
    printff("hash %zx", str2_hash(&a));
    printff("hash %zx", str2_hash(&a));
    printff("hash_ci %zx", str2_hash_ci(&a));
    printff("hash_ci %zx", str2_hash_ci(&a));
    printff("hash %zx", str2_hash(&a));
    printff("cmp %u", str2_hcmp(&a, &b));
    printff("cmp %u", str2_hcmp(&a, &c));
    printff("cmp %u", str2_cmp(a, b));
    printff("cmp %u", str2_cmp(a, c));
    printff("cmp %u", str2_hcmp_ci(&a, &b));
    printff("cmp %u", str2_hcmp_ci(&a, &c));
    printff("cmp %u", str2_cmp_ci(a, b));
    printff("cmp %u", str2_cmp_ci(a, c));
    printff("from first / to last / [%.*s]", STR2_F(str2_i0iE(a, str2_find_ch(a, '/'), str2_rfind_ch(a, '/'))));
    printff("from first / to end [%.*s]", STR2_F(str2_i0(a, str2_find_ch(a, '/'))));
    printff("from beginning to last / [%.*s]", STR2_F(str2_iE(a, str2_rfind_ch(a, '/'))));
    str2_free(&a);
    return 0;
}

