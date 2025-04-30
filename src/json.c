#include "json.h"

#include "str.h"
#include <err.h>
#include <stdlib.h>

/* TODO to make sure potential bugs are gone.
 * - after parse_any | parse_ch | parse_ws | ... check if end -> return r! 
 * - ... and check if we actually not all those checks, i mean, it should be safe 
 *   if I only interface with the result (and not directly the vec)
 * */

typedef struct JsonParse {
    RStr *in;
    JsonCallback func;
    void *data;
} JsonParse;

JsonResult json_parse_prep(RStr *in);
JsonResult json_parse_ch(RStr *in, char c);
JsonResult json_parse_any(RStr *in, char *s);
JsonResult json_parse_ws(RStr *in);

JsonResult json_parse_string(JsonParse *p);
JsonResult json_parse_value(JsonParse *p);
JsonResult json_parse_number(JsonParse *p);
JsonResult json_parse_object(JsonParse *p);
JsonResult json_parse_array(JsonParse *p);
JsonResult json_parse_const(JsonParse *p);

inline JsonResult json_parse_prep(RStr *in) {
    ASSERT_ARG(in);
    bool end = !rstr_length(*in);
    bool match = false;
    char front = end ? 0 : rstr_get_front(in);
    return (JsonResult){ .end = end, .match = match, .front = front };
}

inline JsonResult json_parse_ch(RStr *in, char c) {
    ASSERT_ARG(in);
    JsonResult r = json_parse_prep(in);
    if(r.end) return r;
    if(r.front == c) r.match = true;
    if(!r.match) return r;
    ++in->first;
    return r;
}

inline JsonResult json_parse_any(RStr *in, char *s) {
    ASSERT_ARG(in);
    ASSERT_ARG(s);
    JsonResult r = json_parse_prep(in);
    if(r.end) return r;
    if(strchr(s, r.front)) r.match = true;
    if(!r.match) return r;
    ++in->first;
    return r;
}

inline JsonResult json_parse_ws(RStr *in) {
    ASSERT_ARG(in);
    JsonResult r = {0};
    while((r = json_parse_any(in, " \n\r\t")).match) {}
    return r;
}

inline JsonResult json_parse_string(JsonParse *p) {
    ASSERT_ARG(p);
    RStr *in = p->in;
    JsonResult r = json_parse_ch(in, '"');
    if(r.end || !r.match) return r;
    for(;;) {
        r = json_parse_ch(in, '"');
        if(r.end) return r;
        if(r.match) break;
        if(r.front == '\\') {
            ++in->first;
            r = json_parse_any(in, "\"\\/bfnrtu");
            if(r.end || !r.match) return r;
            if(r.front == 'u') {
                for(int i = 0; i < 4; ++i) {
                    r = json_parse_any(in, "0123456789abcdefABCDEF");
                    if(r.end || !r.match) return r;
                }
            }
        } else if(r.front < 0x20 || r.front == 127) {
            /* control caracter! */
            r.match = false;
            break;
        } else {
            ++in->first;
        }
    }
    return r;
}

inline JsonResult json_parse_number(JsonParse *p) {
    ASSERT_ARG(p);
    RStr *in = p->in;
    JsonResult r = json_parse_ch(in, '-');
    if(r.end) return r;
    r = json_parse_any(in, "0123456789");
    if(!r.match) return r;
    /* 0 alone or digit 1-9 */
    if(r.front != '0') {
        while(r.match) {
            r = json_parse_any(in, "0123456789");
        }
    }
    /* fraction */
    r = json_parse_ch(in, '.');
    if(r.match) {
        r = json_parse_any(in, "0123456789");
        if(!r.match) return r;
        while(r.match) {
            r = json_parse_any(in, "0123456789");
        }
    }
    /* exponent */
    r = json_parse_any(in, "eE");
    if(r.match) {
        (void) json_parse_any(in, "+-");
        r = json_parse_any(in, "0123456789");
        //printff("r.front %c/match %u",r.front,r.match);getchar();
        if(!r.match) return r;
        while(r.match) {
            r = json_parse_any(in, "0123456789");
        }
    }
    return (JsonResult){ .match = true };
}

inline JsonResult json_parse_array(JsonParse *p) {
    ASSERT_ARG(p);
    RStr *in = p->in;
    JsonResult r = json_parse_ch(in, '[');
    if(r.end || !r.match) return r;
    json_parse_ws(in);
    for(bool loop = false;; loop = true) { r = json_parse_ch(in, ']');
        if(r.end || r.match) break;
        if(loop) {
            r = json_parse_ch(in, ',');
            if(!r.match) break;
        }
        r = json_parse_value(p);
        if(loop && !r.match) return r;
        if(!r.match) break;
    }
    return r;
}

inline JsonResult json_parse_const(JsonParse *p) {
    ASSERT_ARG(p);
    RStr *in = p->in;
    JsonResult r = json_parse_any(in, "tfn");
    if(r.end || !r.match) return r;
    switch(r.front) {
        case 't': {
            r = json_parse_ch(in, 'r');
            if(r.end || !r.match) return r;
            r = json_parse_ch(in, 'u');
            if(r.end || !r.match) return r;
            r = json_parse_ch(in, 'e');
        } break;
        case 'f': {
            r = json_parse_ch(in, 'a');
            if(r.end || !r.match) return r;
            r = json_parse_ch(in, 'l');
            if(r.end || !r.match) return r;
            r = json_parse_ch(in, 's');
            if(r.end || !r.match) return r;
            r = json_parse_ch(in, 'e');
        } break;
        case 'n': {
            r = json_parse_ch(in, 'u');
            if(r.end || !r.match) return r;
            r = json_parse_ch(in, 'l');
            if(r.end || !r.match) return r;
            r = json_parse_ch(in, 'l');
        } break;
        default: ABORT(ERR_UNREACHABLE);
    }
    return r;
}

inline JsonResult json_parse_value(JsonParse *p) {
    ASSERT_ARG(p);
    RStr *in = p->in;
    json_parse_ws(in);
    JsonResult r = json_parse_prep(in);
    /* if id == none go here, else continue with 2nd pass */
    switch(r.front) {
        case '[': r = json_parse_array(p); break;
        case '{': r = json_parse_object(p); break;
        case '"': r = json_parse_string(p); break;
        case 't': /* fall */
        case 'f': /* fall */
        case 'n': r = json_parse_const(p); break;
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9': case '-': /* fall */
                  r = json_parse_number(p); break;
        default: r = (JsonResult){0};
    }
    json_parse_ws(in);
    return r;
}

inline JsonResult json_parse_object(JsonParse *p) {
    ASSERT_ARG(p);
    RStr *in = p->in;
    JsonResult r = json_parse_ch(in, '{');
    if(r.end || !r.match) return r;
    bool loop = false;
    while(r.match) {
        if(loop) {
            r = json_parse_ch(in, ',');
            if(!r.match) break;
        }
        json_parse_ws(in);
        r = json_parse_string(p);
        if(!loop && !r.match) break;
        if(r.end || !r.match) return r;
        json_parse_ws(in);
        r = json_parse_ch(in, ':');
        if(r.end || !r.match) return r;
        r = json_parse_value(p);
        if(r.end || !r.match) return r;
        loop = true;
    }
    r = json_parse_ch(in, '}');
    return r;
}

JsonResult json_verify(RStr *in) {
    ASSERT_ARG(in);
    JsonResult r = json_parse(in, 0, 0);
    return r;
}

JsonResult json_parse(RStr *in, JsonCallback func, void *data) {
    ASSERT_ARG(in);
    RStr src = *in;
    JsonParse p = {0};
    p.in = in;
    p.func = func;
    p.data = data;
    json_parse_ws(in);
    JsonResult r = json_parse_object(&p);
    if(!r.match) {
        r = json_parse_array(&p);
    }
    json_parse_ws(in);
    if(rstr_length(*in)) r.match = false;

#if !defined NDEBUG && 0
    if(!r.match) {
        src.last = in->first;
        printff("FAILED! but PARSED:%.*s", RSTR_F(src));
    }
#endif

    return r;
}

#include "file.h"

void test(bool expect, const char *s) {
    RStr in = RSTR_L(s);
    JsonResult r = json_verify(&in);
    if(expect == r.match) {
        printff(F("ok  %s", FG_GN_B) ": %s", r.match?"pass":"fail", s);
    } else if(expect && !r.match) {
        printff(F("exp.pass", FG_YL) ": %s", s);
    } else if(!expect && r.match) {
        printff(F("exp.fail", FG_YL_B) ": %s", s);
    }
}

void test_file(bool expect, const char *f) {
    Str content = {0};
    RStr file = RSTR_L(f);
    TRYC(file_str_read(file, &content));
    RStr parse = str_rstr(content);
    JsonResult r = json_verify(&parse);
    if(expect == r.match) {
        printff(F("ok  %s", FG_GN_B) ": %s", r.match?"pass":"fail", f);
    } else if(expect && !r.match) {
        printff(F("exp.pass", FG_YL) ": %s", f);
    } else if(!expect && r.match) {
        printff(F("exp.fail", FG_YL_B) ": %s", f);
    }
error:;
    str_free(&content);
}

int main(void) {

    test(false, "");
    test(false, "[\"\":1]");
    test(false, "[\"x\"");
    test(false, "[\"x");
    test(false, "[x");
    test(false, "[x\"");
    test(true, "[]");
    test(true, "[[] ]");
    test(true, "{}");
    test(true, "{}");
    test(false, "[-2.]");
    test(false, "[0.e1]");
    test(false, "[0.e1]");
    test(false, "[2.e+3]");
    test(false, "[2.e-3]");
    test(false, "[2.e3]");
    test(false, "[0.3e+]");
    test(false, "[0E+]");
    test(false, "[0e+]");
    test(false, "[1.0e+]");
    test(false, "[1.0e-]");
    test(false, "[9.e+]");
    test(true, "[\"7F\"]");

    test_file(false, "../test/test-json/fail1.json");
    test_file(false, "../test/test-json/fail2.json");
    test_file(false, "../test/test-json/fail3.json");
    test_file(false, "../test/test-json/fail4.json");
    test_file(false, "../test/test-json/fail5.json");
    test_file(false, "../test/test-json/fail6.json");
    test_file(false, "../test/test-json/fail7.json");
    test_file(false, "../test/test-json/fail8.json");
    test_file(false, "../test/test-json/fail9.json");
    test_file(false, "../test/test-json/fail10.json");
    test_file(false, "../test/test-json/fail11.json");
    test_file(false, "../test/test-json/fail12.json");
    test_file(false, "../test/test-json/fail13.json");
    test_file(false, "../test/test-json/fail14.json");
    test_file(false, "../test/test-json/fail15.json");
    test_file(false, "../test/test-json/fail16.json");
    test_file(false, "../test/test-json/fail17.json");
    test_file(false, "../test/test-json/fail18.json");
    test_file(false, "../test/test-json/fail19.json");
    test_file(false, "../test/test-json/fail20.json");
    test_file(false, "../test/test-json/fail21.json");
    test_file(false, "../test/test-json/fail22.json");
    test_file(false, "../test/test-json/fail23.json");
    test_file(false, "../test/test-json/fail24.json");
    test_file(false, "../test/test-json/fail25.json");
    test_file(false, "../test/test-json/fail26.json");
    test_file(false, "../test/test-json/fail27.json");
    test_file(false, "../test/test-json/fail28.json");
    test_file(false, "../test/test-json/fail29.json");
    test_file(false, "../test/test-json/fail30.json");
    test_file(false, "../test/test-json/fail31.json");
    test_file(false, "../test/test-json/fail32.json");
    test_file(false, "../test/test-json/fail33.json");
    test_file(true, "../test/test-json/pass1.json");
    test_file(true, "../test/test-json/pass2.json");
    test_file(true, "../test/test-json/pass3.json");

    return 0;
}


