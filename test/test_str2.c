#include "../src/str.h"
#include <stdio.h>
#include <stdlib.h>
#include "../src/err.h"
#include <stdint.h>
#include <math.h>

int main(void) {

    //printff("bit heap %zx", STR_BIT_HEAP);
    //printff("bit dyn  %zx", STR_BIT_DYNAMIC);
    //printff("bit mask %zx", STR_BIT_MASK);
    printff("sizeof Str %zu", sizeof(Str));
    //return 0;

    Str a = str_dyn(str(""));
    Str b = str_dyn(str("HOME path: "));
    Str c = str_dyn(str(" \v HOME path!  \t "));
    //str_l(getenv("HOME"));
    str_fmt(&a, "HOME path: ");
    str_extend(&b, str_l(getenv("HOME")));
    printff("a: %zu[%.*s]", str_len(a), STR_F(a));
    printff("b: %zu[%.*s]", str_len(b), STR_F(b));
    printff("c: %zu[%.*s]", str_len(c), STR_F(c));
    str_fmt(&a, "%s", getenv("HOME"));
    printff("a: %zu[%.*s]", str_len(a), STR_F(a));
    printff("%zu[%.*s]", str_len(a), STR_F(a));
    printff("first %c@%zu:%zu", '/', str_find_ch(a, '/'), str_len(a));
    printff("last %c@%zu:%zu", '/', str_rfind_ch(a, '/'), str_len(a));
    printff("hash %zx", str_hash(&a));
    printff("hash %zx", str_hash(&a));
    printff("hash_ci %zx", str_hash_ci(&a));
    printff("hash_ci %zx", str_hash_ci(&a));
    printff("hash %zx", str_hash(&a));
    printff("cmp %u", str_hcmp(&a, &b));
    printff("cmp %u", str_hcmp(&a, &c));
    printff("cmp %u", str_cmp(a, b));
    printff("cmp %u", str_cmp(a, c));
    printff("cmp %u", str_hcmp_ci(&a, &b));
    printff("cmp %u", str_hcmp_ci(&a, &c));
    printff("cmp %u", str_cmp_ci(a, b));
    printff("cmp %u", str_cmp_ci(a, c));
    printff("from first / to last / [%.*s] (%p)", STR_F(str_i0iE(a, str_find_ch(a, '/'), str_rfind_ch(a, '/'))), a.str);
    printff("from first / to end [%.*s]", STR_F(str_i0(a, str_find_ch(a, '/'))));
    printff("from beginning to last / [%.*s]", STR_F(str_iE(a, str_rfind_ch(a, '/'))));

    Str d = STR_DYN();
    str_copy(&d, str_i0iE(a, str_find_ch(a, '/'), str_rfind_ch(a, '/')));
    printff("copy from first / to last / %zu[%.*s] (%p)", str_len(d), STR_F(d), d.str);

    Str e = str_trim(c);
    printff("trimmed [%.*s] is heap %u", STR_F(e), str_is_heap(e));

    Str pathtest = {0};
    Str x = {0};
    pathtest = str("/etc/path.dir/filename");
    x = str_get_ext(pathtest);
    printff("%.*s -> ext   [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_get_noext(pathtest);
    printff("%.*s -> noext [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_get_dir(pathtest);
    printff("%.*s -> dir   [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_get_nodir(pathtest);
    printff("%.*s -> nodir [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_get_basename(pathtest);
    printff("%.*s -> basen [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_ensure_dir(pathtest);
    printff("%.*s -> ensur [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    pathtest = str("/etc/path.dir/filename.txt");
    x = str_get_ext(pathtest);
    printff("%.*s -> ext   [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_get_noext(pathtest);
    printff("%.*s -> noext [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_get_dir(pathtest);
    printff("%.*s -> dir   [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_get_nodir(pathtest);
    printff("%.*s -> nodir [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_get_basename(pathtest);
    printff("%.*s -> basen [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_ensure_dir(pathtest);
    printff("%.*s -> ensur [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    pathtest = str("/etc/path.dir/filename.txt/");
    x = str_get_ext(pathtest);
    printff("%.*s -> ext   [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_get_noext(pathtest);
    printff("%.*s -> noext [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_get_dir(pathtest);
    printff("%.*s -> dir   [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_get_nodir(pathtest);
    printff("%.*s -> nodir [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_get_basename(pathtest);
    printff("%.*s -> basen [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    x = str_ensure_dir(pathtest);
    printff("%.*s -> ensur [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    pathtest = str("/some/path////");
    x = str_ensure_dir(pathtest);
    printff("%.*s -> ensur [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    pathtest = str("/////");
    x = str_ensure_dir(pathtest);
    printff("%.*s -> ensur [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));
    pathtest = str("/");
    x = str_ensure_dir(pathtest);
    printff("%.*s -> ensur [%.*s]%zu", STR_F(pathtest), STR_F(x), str_len(x));

    Str f = STR_DYN();
    str_push(&f, '<');
    str_push(&f, 'x');
    str_push(&f, 'y');
    str_push(&f, '>');
    str_push(&f, 'z');
    printff("push [%.*s]", STR_F(f));
    printff("pair [%.*s]", STR_F(str_iE(f, str_pair_ch(f, '>'))));
    str_extend(&f, str(",abc,,def,ghi,,"));
    printff("splice input [%.*s]%zu", STR_F(f), str_len(f));
    for(Str splice = {0}; str_splice(f, &splice, ','); ) {
        printff("splice:[%.*s]", STR_F(splice));
    }
    str_extend(&f, str("xyz,ikj"));
    printff("splice input [%.*s]%zu", STR_F(f), str_len(f));
    for(Str splice = {0}; str_splice(f, &splice, ','); ) {
        if(!splice.str) continue;
        printff("splice:[%.*s]", STR_F(splice));
    }

    Str colors = STR_DYN();
    str_fmt_fgbg(&colors, str("bold"), (Color){0}, (Color){0}, true, false, false);
    printff("[%.*s]%zu", STR_F(colors), str_len(colors));
    str_clear(&colors);
    str_fmt_fgbg(&colors, str("italic"), (Color){0}, (Color){0}, false, true, false);
    printff("[%.*s]%zu", STR_F(colors), str_len(colors));
    str_clear(&colors);
    str_fmt_fgbg(&colors, str("underline"), (Color){0}, (Color){0}, false, false, true);
    printff("[%.*s]%zu", STR_F(colors), str_len(colors));
    str_clear(&colors);
    str_fmt_fgbg(&colors, str("vanilla"), (Color){0}, (Color){0}, false, false, false);
    str_push(&colors, ' ');
    str_fmt_fgbg(&colors, str("bold"), (Color){0}, (Color){0}, true, false, false);
    str_push(&colors, ' ');
    str_fmt_fgbg(&colors, str("italic"), (Color){0}, (Color){0}, false, true, false);
    str_push(&colors, ' ');
    str_fmt_fgbg(&colors, str("underline"), (Color){0}, (Color){0}, false, false, true);
    str_push(&colors, ' ');
    str_fmt_fgbg(&colors, str("bold+italic"), (Color){0}, (Color){0}, true, true, false);
    str_push(&colors, ' ');
    str_fmt_fgbg(&colors, str("bold+underline"), (Color){0}, (Color){0}, true, false, true);
    str_push(&colors, ' ');
    str_fmt_fgbg(&colors, str("italic+underline"), (Color){0}, (Color){0}, false, true, true);
    str_push(&colors, ' ');
    str_fmt_fgbg(&colors, str("bold+italic+underline"), (Color){0}, (Color){0}, true, true, true);
    str_push(&colors, ' ');
    str_fmt_fgbg(&colors, str("vanilla"), (Color){0}, (Color){0}, false, false, false);
    printff("[%.*s]%zu: fmt %zu..%zu", STR_F(colors), str_len(colors), str_find_f(colors, 0), str_rfind_f(colors, 0));
    printff("counts - e:%zu, i:%zu, ei:%zu, len nof %zu", str_count_ch(colors, 'e'), str_count_ch(colors, 'i'), str_count_any(colors, str("ei")), str_len_nof(colors));
    str_clear(&colors);
    str_fmt_fgbg(&colors, str("red"), COLOR_RED, COLOR_NONE, true, false, false);
    str_fmt_fgbg(&colors, str("navy-yellow"), COLOR_NAVY, COLOR_YELLOW, true, false, false);
    printff("[%.*s]%zu", STR_F(colors), str_len(colors));
    str_clear(&colors);
    str_fmt_fgbg(&colors, str("alpha-non-respecting"), COLOR_RGBA(0xFF,0x1F,0x1F,0x7F), COLOR_RGBA(0x00,0x1F,0xFF,0x7F), true, false, false);
    str_fmt_fgbga(&colors, str("alpha-respecting"), COLOR_RGBA(0xFF,0x1F,0x1F,0x7F), COLOR_RGBA(0x00,0x1F,0xFF,0x7F), true, false, false);
    printff("[%.*s]%zu", STR_F(colors), str_len(colors));
    str_clear(&colors);

    Color color = {0};
    Str colorstr = STR_DYN();
    colorstr = str("rgb(ff22ff)");
    if(!str_as_color(colorstr, &color)) {
        str_clear(&colors);
        str_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str_println(colors);
    }
    colorstr = str("rgb  (ff22ee)");
    if(!str_as_color(colorstr, &color)) {
        str_clear(&colors);
        str_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str_println(colors);
    }
    colorstr = str("rgb  (  ff22dd)");
    if(!str_as_color(colorstr, &color)) {
        str_clear(&colors);
        str_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str_println(colors);
    }
    colorstr = str("rgb  (  ff22cc   )");
    if(!str_as_color(colorstr, &color)) {
        str_clear(&colors);
        str_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str_println(colors);
    }
    colorstr = str("rgba  (  3322ccff   )");
    if(!str_as_color(colorstr, &color)) {
        str_clear(&colors);
        str_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str_println(colors);
    }
    colorstr = str("rgba  (  3322ccee   )");
    if(!str_as_color(colorstr, &color)) {
        str_clear(&colors);
        str_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str_println(colors);
    }
    colorstr = str("rgba  (  3322ccdd   )");
    if(!str_as_color(colorstr, &color)) {
        str_clear(&colors);
        str_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str_println(colors);
    }
    colorstr = str("rgba  (  123,255,26,0.933   )");
    if(!str_as_color(colorstr, &color)) {
        str_clear(&colors);
        str_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str_println(colors);
    }
    colorstr = str("rgba  (  123,255,26,0.5   )");
    if(!str_as_color(colorstr, &color)) {
        str_clear(&colors);
        str_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str_println(colors);
    }
    colorstr = str("rgba  (  123,255,26,0.05   )");
    if(!str_as_color(colorstr, &color)) {
        str_clear(&colors);
        str_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str_println(colors);
    }
    colorstr = str("rgba  (  123,,26,1   )");
    if(!str_as_color(colorstr, &color)) {
        str_clear(&colors);
        str_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str_println(colors);
    }

#if 0
    float flt;
    printf("red:");
    str_clear(&colors);
    str_input(&colors);
    try(str_as_float(colors, &flt));
    color.r = (uint8_t)roundf(flt*255.0f);
    printf("green:");
    str_clear(&colors);
    str_input(&colors);
    try(str_as_float(colors, &flt));
    color.g = (uint8_t)roundf(flt*255.0f);
    printf("blue:");
    str_clear(&colors);
    str_input(&colors);
    try(str_as_float(colors, &flt));
    color.b = (uint8_t)roundf(flt*255.0f);
    str_clear(&colors);
    str_fmt_fgbg(&colors, str("final color!"), COLOR_RGB_NEGATIVE(color), color, true, false, false);
    printff("[%.*s]%zu", STR_F(colors), str_len(colors));
#endif

#if 0
    printf("#rrggbb:");
    str_clear(&colors);
    str_input(&colors);
    try(str_as_color(str_trim(colors), &color));
    str_clear(&colors);
    str_fmt_fgbg(&colors, str("final color!"), COLOR_RGB_NEGATIVE(color), color, true, false, false);
    printff("[%.*s]%zu", STR_F(colors), str_len(colors));
#endif

#if 0
    str_clear(&colors);
    for(size_t i = 0; i < 0x1000000; ++i) {
        if(!((i+1)%0x1000)) printf("\r%zx (%.1f%%)",i, 100.0f*(double)i/(double)0x1000000);
        if(!((i)%0x100)) str_push(&colors, '\n');
        color.rgba = i;
        str_fmt_fgbg(&colors, str(" "), COLOR_NONE, color, false, false, false);
    }
    printff("\n[\n%.*s\n]%zu", STR_F(colors), str_len(colors));
#endif

    StrPrint p = {0};
#if 1
    str_printal(str("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed viverra felis vitae massa fringilla, in blandit est euismod. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Aenean consequat tincidunt volutpat. Suspendisse auctor eros lacus, quis consectetur urna rutrum nec. Maecenas eu lectus venenatis, aliquam libero vel, ornare nibh. Quisque sodales iaculis urna, ac venenatis est gravida eu. Morbi dignissim ac augue quis malesuada. Vestibulum aliquet dui porta ante accumsan, vel vestibulum massa commodo. In hac habitasse platea dictumst. Donec eget leo hendrerit, luctus nulla non, pulvinar nunc. Aliquam erat volutpat."), &p, 0, 50);
    str_println(str(""));

    str_printal(str("               lol                     "), &p, 0, 5);
    str_println(str(""));

    str_printal(str(F("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed viverra felis vitae massa fringilla, in blandit est euismod. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Aenean consequat tincidunt volutpat. Suspendisse auctor eros lacus, quis consectetur urna rutrum nec. Maecenas eu lectus venenatis, aliquam libero vel, ornare nibh. Quisque sodales iaculis urna, ac venenatis est gravida eu. Morbi dignissim ac augue quis malesuada. Vestibulum aliquet dui porta ante accumsan, vel vestibulum massa commodo. In hac habitasse platea dictumst.", FG_CY) " Donec eget leo hendrerit, luctus nulla non, pulvinar nunc. Aliquam erat volutpat."), &p, 0, 50);
    //str_println(str(""));
#endif

#if 1
    Str crazystring = STR_DYN(), crazystring_in = str(" Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed viverra felis vitae massa fringilla, in blandit est euismod. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Aenean consequat tincidunt volutpat. Suspendisse auctor eros lacus, quis consectetur urna rutrum nec. Maecenas eu lectus venenatis, aliquam libero vel, ornare nibh. Quisque sodales iaculis urna, ac venenatis est gravida eu. Morbi dignissim ac augue quis malesuada. Vestibulum aliquet dui porta ante accumsan, vel vestibulum massa commodo. In hac habitasse platea dictumst. Donec eget leo hendrerit, luctus nulla non, pulvinar nunc. Aliquam erat volutpat.");
    //memset(&p, 0, sizeof(p));
    for(size_t i = 0; i < str_len(crazystring_in); i += 3) {
        Color col = {0};
        col.rgba = rand();
        Str snip = str_i0iE(crazystring_in, i, i + 3 < str_len(crazystring_in) ? i + 3 : str_len(crazystring_in));
        str_fmt_fgbg(&crazystring, snip, col, COLOR_NONE, false, false, false);
    }
#endif

#if 0
    printf("index nof @ 0 -> %zu\n", str_index_nof(crazystring, 0));
    str_println(str_i0(crazystring, str_index_nof(crazystring, 0)));

    printf("index nof @ 1 -> %zu\n", str_index_nof(crazystring, 1));
    str_println(str_i0(crazystring, str_index_nof(crazystring, 1)));

    printf("index nof @ 2 -> %zu\n", str_index_nof(crazystring, 2));
    str_println(str_i0(crazystring, str_index_nof(crazystring, 2)));

    printf("index nof @ 3 -> %zu\n", str_index_nof(crazystring, 3));
    str_println(str_i0(crazystring, str_index_nof(crazystring, 3)));

    printf("index nof @ 4 -> %zu\n", str_index_nof(crazystring, 4));
    str_println(str_i0(crazystring, str_index_nof(crazystring, 4)));

    printf("index nof @ 5 -> %zu\n", str_index_nof(crazystring, 5));
    str_println(str_i0(crazystring, str_index_nof(crazystring, 5)));

    printf("index nof @ 6 -> %zu\n", str_index_nof(crazystring, 6));
    str_println(str_i0(crazystring, str_index_nof(crazystring, 6)));
#endif

#if 1
    str_printal(crazystring, &p, 10, 50);
    str_printal(str("asdf xyz abc"), &p, 30, 50);
    str_printal(str("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"), &p, 42, 50);

    Str spacy = STR_DYN(), spacy_in = str("                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                text                              ");
    for(size_t i = 0; i < str_len(spacy_in); i += 1) {
        Color col = { .rgba = rand() };
        Str snip = str_i0iE(spacy_in, i, i + 1 < str_len(spacy_in) ? i + 1 : str_len(spacy_in));
        str_fmt_fgbg(&spacy, snip, COLOR_RGB_NEGATIVE(col), col, false, false, false);
    }
    str_printal(spacy, &p, 45, 50);
    str_println(str(""));

    //str_freeall(a, b, c, d, e);
    str_free(&crazystring);
#endif

    Str y = STR_DYN();
    str_fmtx(&y, (StrFmtX){ .fg.rgba = 0xf13091 }, "a string!");
    str_fmtx(&y, (StrFmtX){ .fg.rgba = 0x982192 }, "another!");
    str_fmtx(&y, (StrFmtX){ .bg.rgba = 0x897921 }, "a string!");
    str_println(y);

    printff("EMPTY STRING>>");
    str_println(str_l(0));
    printff("EMPTY STRING<<");

    str_free(&colors);
    str_free(&a);
    str_free(&a);
    str_free(&b);
    str_free(&c);
    str_free(&d);
    str_free(&e);
    str_free(&f);
    printf("done\n");
    return 0;
}

