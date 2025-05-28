#include "../src/str2.h"
#include <stdio.h>
#include <stdlib.h>
#include "../src/err.h"
#include <stdint.h>
#include <math.h>

int main(void) {

    //printff("bit heap %zx", STR2_BIT_HEAP);
    //printff("bit dyn  %zx", STR2_BIT_DYNAMIC);
    //printff("bit mask %zx", STR2_BIT_MASK);
    printff("sizeof Str2 %zu", sizeof(Str2));
    //return 0;

    Str2 a = {0};
    Str2 b = str2_dyn(str2("HOME path: "));
    Str2 c = str2_dyn(str2(" \v HOME path!  \t "));
    //str2_l(getenv("HOME"));
    str2_fmt(&a, "HOME path: ");
    str2_extend(&b, str2_l(getenv("HOME")));
    printff("a: %zu[%.*s]", str2_len(a), STR2_F(a));
    printff("b: %zu[%.*s]", str2_len(b), STR2_F(b));
    printff("c: %zu[%.*s]", str2_len(c), STR2_F(c));
    str2_fmt(&a, "%s", getenv("HOME"));
    printff("a: %zu[%.*s]", str2_len(a), STR2_F(a));
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
    x = str2_ensure_dir(pathtest);
    printff("%.*s -> ensur [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
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
    x = str2_ensure_dir(pathtest);
    printff("%.*s -> ensur [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
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
    x = str2_ensure_dir(pathtest);
    printff("%.*s -> ensur [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    pathtest = str2("/some/path////");
    x = str2_ensure_dir(pathtest);
    printff("%.*s -> ensur [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    pathtest = str2("/////");
    x = str2_ensure_dir(pathtest);
    printff("%.*s -> ensur [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));
    pathtest = str2("/");
    x = str2_ensure_dir(pathtest);
    printff("%.*s -> ensur [%.*s]%zu", STR2_F(pathtest), STR2_F(x), str2_len(x));

    Str2 f = {0};
    str2_push(&f, '<');
    str2_push(&f, 'x');
    str2_push(&f, 'y');
    str2_push(&f, '>');
    str2_push(&f, 'z');
    printff("push [%.*s]", STR2_F(f));
    printff("pair [%.*s]", STR2_F(str2_iE(f, str2_pair_ch(f, '>'))));
    str2_extend(&f, str2(",abc,,def,ghi,,"));
    printff("splice input [%.*s]%zu", STR2_F(f), str2_len(f));
    for(Str2 splice = {0}; str2_splice(f, &splice, ','); ) {
        printff("splice:[%.*s]", STR2_F(splice));
    }
    str2_extend(&f, str2("xyz,ikj"));
    printff("splice input [%.*s]%zu", STR2_F(f), str2_len(f));
    for(Str2 splice = {0}; str2_splice(f, &splice, ','); ) {
        if(!splice.str) continue;
        printff("splice:[%.*s]", STR2_F(splice));
    }

    Str2 colors = {0};
    str2_fmt_fgbg(&colors, str2("bold"), (Color){0}, (Color){0}, true, false, false);
    printff("[%.*s]%zu", STR2_F(colors), str2_len(colors));
    str2_clear(&colors);
    str2_fmt_fgbg(&colors, str2("italic"), (Color){0}, (Color){0}, false, true, false);
    printff("[%.*s]%zu", STR2_F(colors), str2_len(colors));
    str2_clear(&colors);
    str2_fmt_fgbg(&colors, str2("underline"), (Color){0}, (Color){0}, false, false, true);
    printff("[%.*s]%zu", STR2_F(colors), str2_len(colors));
    str2_clear(&colors);
    str2_fmt_fgbg(&colors, str2("vanilla"), (Color){0}, (Color){0}, false, false, false);
    str2_push(&colors, ' ');
    str2_fmt_fgbg(&colors, str2("bold"), (Color){0}, (Color){0}, true, false, false);
    str2_push(&colors, ' ');
    str2_fmt_fgbg(&colors, str2("italic"), (Color){0}, (Color){0}, false, true, false);
    str2_push(&colors, ' ');
    str2_fmt_fgbg(&colors, str2("underline"), (Color){0}, (Color){0}, false, false, true);
    str2_push(&colors, ' ');
    str2_fmt_fgbg(&colors, str2("bold+italic"), (Color){0}, (Color){0}, true, true, false);
    str2_push(&colors, ' ');
    str2_fmt_fgbg(&colors, str2("bold+underline"), (Color){0}, (Color){0}, true, false, true);
    str2_push(&colors, ' ');
    str2_fmt_fgbg(&colors, str2("italic+underline"), (Color){0}, (Color){0}, false, true, true);
    str2_push(&colors, ' ');
    str2_fmt_fgbg(&colors, str2("bold+italic+underline"), (Color){0}, (Color){0}, true, true, true);
    str2_push(&colors, ' ');
    str2_fmt_fgbg(&colors, str2("vanilla"), (Color){0}, (Color){0}, false, false, false);
    printff("[%.*s]%zu: fmt %zu..%zu", STR2_F(colors), str2_len(colors), str2_find_f(colors, 0), str2_rfind_f(colors, 0));
    printff("counts - e:%zu, i:%zu, ei:%zu, len nof %zu", str2_count_ch(colors, 'e'), str2_count_ch(colors, 'i'), str2_count_any(colors, str2("ei")), str2_len_nof(colors));
    str2_clear(&colors);
    str2_fmt_fgbg(&colors, str2("red"), COLOR_RED, COLOR_NONE, true, false, false);
    str2_fmt_fgbg(&colors, str2("navy-yellow"), COLOR_NAVY, COLOR_YELLOW, true, false, false);
    printff("[%.*s]%zu", STR2_F(colors), str2_len(colors));
    str2_clear(&colors);
    str2_fmt_fgbg(&colors, str2("alpha-non-respecting"), COLOR_RGBA(0xFF,0x1F,0x1F,0x7F), COLOR_RGBA(0x00,0x1F,0xFF,0x7F), true, false, false);
    str2_fmt_fgbga(&colors, str2("alpha-respecting"), COLOR_RGBA(0xFF,0x1F,0x1F,0x7F), COLOR_RGBA(0x00,0x1F,0xFF,0x7F), true, false, false);
    printff("[%.*s]%zu", STR2_F(colors), str2_len(colors));
    str2_clear(&colors);

    Color color = {0};
    Str2 colorstr = {0};
    colorstr = str2("rgb(ff22ff)");
    if(!str2_as_color(colorstr, &color)) {
        str2_clear(&colors);
        str2_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str2_println(colors);
    }
    colorstr = str2("rgb  (ff22ee)");
    if(!str2_as_color(colorstr, &color)) {
        str2_clear(&colors);
        str2_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str2_println(colors);
    }
    colorstr = str2("rgb  (  ff22dd)");
    if(!str2_as_color(colorstr, &color)) {
        str2_clear(&colors);
        str2_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str2_println(colors);
    }
    colorstr = str2("rgb  (  ff22cc   )");
    if(!str2_as_color(colorstr, &color)) {
        str2_clear(&colors);
        str2_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str2_println(colors);
    }
    colorstr = str2("rgba  (  3322ccff   )");
    if(!str2_as_color(colorstr, &color)) {
        str2_clear(&colors);
        str2_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str2_println(colors);
    }
    colorstr = str2("rgba  (  3322ccee   )");
    if(!str2_as_color(colorstr, &color)) {
        str2_clear(&colors);
        str2_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str2_println(colors);
    }
    colorstr = str2("rgba  (  3322ccdd   )");
    if(!str2_as_color(colorstr, &color)) {
        str2_clear(&colors);
        str2_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str2_println(colors);
    }
    colorstr = str2("rgba  (  123,255,26,0.933   )");
    if(!str2_as_color(colorstr, &color)) {
        str2_clear(&colors);
        str2_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str2_println(colors);
    }
    colorstr = str2("rgba  (  123,255,26,0.5   )");
    if(!str2_as_color(colorstr, &color)) {
        str2_clear(&colors);
        str2_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str2_println(colors);
    }
    colorstr = str2("rgba  (  123,255,26,0.05   )");
    if(!str2_as_color(colorstr, &color)) {
        str2_clear(&colors);
        str2_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str2_println(colors);
    }
    colorstr = str2("rgba  (  123,,26,1   )");
    if(!str2_as_color(colorstr, &color)) {
        str2_clear(&colors);
        str2_fmt_fgbga(&colors, colorstr, COLOR_NONE, color, false, false, false);
        str2_println(colors);
    }

#if 0
    float flt;
    printf("red:");
    str2_clear(&colors);
    str2_input(&colors);
    try(str2_as_float(colors, &flt));
    color.r = (uint8_t)roundf(flt*255.0f);
    printf("green:");
    str2_clear(&colors);
    str2_input(&colors);
    try(str2_as_float(colors, &flt));
    color.g = (uint8_t)roundf(flt*255.0f);
    printf("blue:");
    str2_clear(&colors);
    str2_input(&colors);
    try(str2_as_float(colors, &flt));
    color.b = (uint8_t)roundf(flt*255.0f);
    str2_clear(&colors);
    str2_fmt_fgbg(&colors, str2("final color!"), COLOR_RGB_NEGATIVE(color), color, true, false, false);
    printff("[%.*s]%zu", STR2_F(colors), str2_len(colors));
#endif

#if 0
    printf("#rrggbb:");
    str2_clear(&colors);
    str2_input(&colors);
    try(str2_as_color(str2_trim(colors), &color));
    str2_clear(&colors);
    str2_fmt_fgbg(&colors, str2("final color!"), COLOR_RGB_NEGATIVE(color), color, true, false, false);
    printff("[%.*s]%zu", STR2_F(colors), str2_len(colors));
#endif

#if 0
    str2_clear(&colors);
    for(size_t i = 0; i < 0x1000000; ++i) {
        if(!((i+1)%0x1000)) printf("\r%zx (%.1f%%)",i, 100.0f*(double)i/(double)0x1000000);
        if(!((i)%0x100)) str2_push(&colors, '\n');
        color.rgba = i;
        str2_fmt_fgbg(&colors, str2(" "), COLOR_NONE, color, false, false, false);
    }
    printff("\n[\n%.*s\n]%zu", STR2_F(colors), str2_len(colors));
#endif

    Str2Print p = {0};
#if 1
    str2_printal(str2("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed viverra felis vitae massa fringilla, in blandit est euismod. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Aenean consequat tincidunt volutpat. Suspendisse auctor eros lacus, quis consectetur urna rutrum nec. Maecenas eu lectus venenatis, aliquam libero vel, ornare nibh. Quisque sodales iaculis urna, ac venenatis est gravida eu. Morbi dignissim ac augue quis malesuada. Vestibulum aliquet dui porta ante accumsan, vel vestibulum massa commodo. In hac habitasse platea dictumst. Donec eget leo hendrerit, luctus nulla non, pulvinar nunc. Aliquam erat volutpat."), &p, 0, 50);
    str2_println(str2(""));

    str2_printal(str2("               lol                     "), &p, 0, 5);
    str2_println(str2(""));

    str2_printal(str2(F("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed viverra felis vitae massa fringilla, in blandit est euismod. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Aenean consequat tincidunt volutpat. Suspendisse auctor eros lacus, quis consectetur urna rutrum nec. Maecenas eu lectus venenatis, aliquam libero vel, ornare nibh. Quisque sodales iaculis urna, ac venenatis est gravida eu. Morbi dignissim ac augue quis malesuada. Vestibulum aliquet dui porta ante accumsan, vel vestibulum massa commodo. In hac habitasse platea dictumst.", FG_CY) " Donec eget leo hendrerit, luctus nulla non, pulvinar nunc. Aliquam erat volutpat."), &p, 0, 50);
    str2_println(str2(""));
#endif

#if 1
    Str2 crazystring = {0}, crazystring_in = str2("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed viverra felis vitae massa fringilla, in blandit est euismod. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Aenean consequat tincidunt volutpat. Suspendisse auctor eros lacus, quis consectetur urna rutrum nec. Maecenas eu lectus venenatis, aliquam libero vel, ornare nibh. Quisque sodales iaculis urna, ac venenatis est gravida eu. Morbi dignissim ac augue quis malesuada. Vestibulum aliquet dui porta ante accumsan, vel vestibulum massa commodo. In hac habitasse platea dictumst. Donec eget leo hendrerit, luctus nulla non, pulvinar nunc. Aliquam erat volutpat.");
    memset(&p, 0, sizeof(p));
    for(size_t i = 0; i < str2_len(crazystring_in); i += 3) {
        Color col = {0};
        col.rgba = rand();
        Str2 snip = str2_i0iE(crazystring_in, i, i + 3 < str2_len(crazystring_in) ? i + 3 : str2_len(crazystring_in));
        str2_fmt_fgbg(&crazystring, snip, col, COLOR_NONE, false, false, false);
    }
#endif

#if 0
    printf("index nof @ 0 -> %zu\n", str2_index_nof(crazystring, 0));
    str2_println(str2_i0(crazystring, str2_index_nof(crazystring, 0)));

    printf("index nof @ 1 -> %zu\n", str2_index_nof(crazystring, 1));
    str2_println(str2_i0(crazystring, str2_index_nof(crazystring, 1)));

    printf("index nof @ 2 -> %zu\n", str2_index_nof(crazystring, 2));
    str2_println(str2_i0(crazystring, str2_index_nof(crazystring, 2)));

    printf("index nof @ 3 -> %zu\n", str2_index_nof(crazystring, 3));
    str2_println(str2_i0(crazystring, str2_index_nof(crazystring, 3)));

    printf("index nof @ 4 -> %zu\n", str2_index_nof(crazystring, 4));
    str2_println(str2_i0(crazystring, str2_index_nof(crazystring, 4)));

    printf("index nof @ 5 -> %zu\n", str2_index_nof(crazystring, 5));
    str2_println(str2_i0(crazystring, str2_index_nof(crazystring, 5)));

    printf("index nof @ 6 -> %zu\n", str2_index_nof(crazystring, 6));
    str2_println(str2_i0(crazystring, str2_index_nof(crazystring, 6)));
#endif

#if 1
    str2_printal(crazystring, &p, 0, 50);
    str2_println(str2(""));
    //str2_printraw(crazystring);


    Str2 spacy = {0}, spacy_in = str2("                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                text                              ");
    for(size_t i = 0; i < str2_len(spacy_in); i += 1) {
        Color col = {0};
        col.rgba = rand();
        Str2 snip = str2_i0iE(spacy_in, i, i + 1 < str2_len(spacy_in) ? i + 1 : str2_len(spacy_in));
        str2_fmt_fgbg(&spacy, snip, col, COLOR_NONE, false, false, false);
    }
    str2_printal(spacy, &p, 0, 50);
    str2_println(str2(""));

    //str2_freeall(a, b, c, d, e);
    str2_free(&crazystring);
#endif
    str2_free(&colors);
    str2_free(&a);
    str2_free(&a);
    str2_free(&b);
    str2_free(&c);
    str2_free(&d);
    str2_free(&e);
    str2_free(&f);
    printf("done\n");
    return 0;
}

