#include "../src/str2.h"
#include <stdio.h>
#include <stdlib.h>
#include "../src/err.h"
#include <stdint.h>

int main(void) {
    Str2 a = {0};
    Str2 b = str2("HOME path: ");
    Str2 c = str2(" \r HOME path!  \t ");
    //str2_l(getenv("HOME"));
    str2_fmt(&a, "HOME path: ");
    str2_extend(&b, str2_l(getenv("HOME")));
    printff("a: %zu[%.*s]", str2_len(a), STR2_F(a));
    printff("b: %zu[%.*s]", str2_len(b), STR2_F(b));
    printff("c: %zu[%.*s]", str2_len(c), STR2_F(c));
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
    printff("from first / to last / [%.*s] (%p)", STR2_F(str2_i0iE(a, str2_find_ch(a, '/'), str2_rfind_ch(a, '/'))), a.str);
    printff("from first / to end [%.*s]", STR2_F(str2_i0(a, str2_find_ch(a, '/'))));
    printff("from beginning to last / [%.*s]", STR2_F(str2_iE(a, str2_rfind_ch(a, '/'))));

    Str2 d = str2_copy(str2_i0iE(a, str2_find_ch(a, '/'), str2_rfind_ch(a, '/')));
    printff("copy from first / to last / %zu[%.*s] (%p)", str2_len(d), STR2_F(d), d.str);

    Str2 e = str2_trim(c);
    printff("trimmed [%.*s] is heap %u", STR2_F(e), str2_is_heap(e));

    Str2 pathtest = {0};
    Str2 x = {0};
    pathtest = str2("/etc/path.dir/filename");
    x = str2_get_ext(pathtest);
    printff("%.*s -> ext   [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    x = str2_get_noext(pathtest);
    printff("%.*s -> noext [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    x = str2_get_dir(pathtest);
    printff("%.*s -> dir   [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    x = str2_get_nodir(pathtest);
    printff("%.*s -> nodir [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    x = str2_get_basename(pathtest);
    printff("%.*s -> basen [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    pathtest = str2("/etc/path.dir/filename.txt");
    x = str2_get_ext(pathtest);
    printff("%.*s -> ext   [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    x = str2_get_noext(pathtest);
    printff("%.*s -> noext [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    x = str2_get_dir(pathtest);
    printff("%.*s -> dir   [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    x = str2_get_nodir(pathtest);
    printff("%.*s -> nodir [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    x = str2_get_basename(pathtest);
    printff("%.*s -> basen [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    pathtest = str2("/etc/path.dir/filename.txt/");
    x = str2_get_ext(pathtest);
    printff("%.*s -> ext   [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    x = str2_get_noext(pathtest);
    printff("%.*s -> noext [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    x = str2_get_dir(pathtest);
    printff("%.*s -> dir   [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    x = str2_get_nodir(pathtest);
    printff("%.*s -> nodir [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    x = str2_get_basename(pathtest);
    printff("%.*s -> basen [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));

    str2_free(&a);
    str2_free(&b);
    str2_free(&c);
    str2_free(&d);
    str2_free(&e);
    return 0;
}

