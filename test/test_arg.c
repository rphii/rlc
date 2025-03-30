#include "../src/arg.h"
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

typedef enum {
    CONFIG_NONE,
    CONFIG_PRINT,
    CONFIG_BROWSER,
    CONFIG_LMAO,
} ConfigList;

typedef struct Config {
    ssize_t whole;
    bool boolean;
    double number;
    RStr config;
    RStr string;
    ConfigList id;
    ConfigList id2;
    ConfigList id3;
} Config;

#define ERR_main_arg(...) ERR_UNREACHABLE
int main_arg(struct Arg *arg, Config *c, Config *p, const int argc, const char **argv) {
    //ArgOpt *opt, *cat, *c2, *c3;

    /* 
     
    n ARG_NONE, ARG_NONE,
    b ARG_BOOL, ARG_BOOL,
    i ARG_INTE, ARG_INT,
    f ARG_FLOA, ARG_FLOAT,
    s ARG_STRI, ARG_STRING,
    x ARG_EXOT, ARG_EXOTIC,
    o ARG_OPTI, ARG_OPTION,
     
     *
     * */

#if 0
    TRYC(arg_attach_help(arg, argc, argv, RSTR("this is a test arg parser"), RSTR("https://github.com/rphii/c-arg")));
    TRYC(argopt_new(arg, &arg->options, &opt, 0, RSTR("--boring"), RSTR("a boring boolean value")));
    argopt_set_bool(opt, &c->boring, &p->boring);
    TRYC(argopt_new(arg, &arg->options, &opt, 0, RSTR("--verbose"), RSTR("verbosity level")));
    argopt_set_int(opt, &c->verbose, &p->verbose);
    TRYC(argopt_new(arg, &arg->options, &opt, 0, RSTR("--math"), RSTR("the math value")));
    argopt_set_float(opt, &c->math, &p->math);
    TRYC(argopt_new(arg, &arg->options, &opt, 0, RSTR("--config"), RSTR("config path")));
    argopt_set_str(opt, &c->config, &p->config);

    TRYC(argopt_new(arg, &arg->options, &cat, 0, RSTR("--action"), RSTR("specify what to do")));
    TRYC(argopt_set_option(cat, (int *)&c->id, (int *)&p->id));
    TRYC(argopt_new(arg, cat->options, &opt, 0, RSTR("none"), RSTR("specify what to do")));
    argopt_set_enum(opt, CONFIG_NONE);
    argopt_set_int(opt, &c->verbose, &p->verbose);
    TRYC(argopt_new(arg, cat->options, &opt, 0, RSTR("print"), RSTR("specify what to do")));
    argopt_set_enum(opt, CONFIG_PRINT);
    TRYC(argopt_new(arg, cat->options, &opt, 0, RSTR("browser"), RSTR("specify what to do")));
    argopt_set_enum(opt, CONFIG_BROWSER);

    TRYC(argopt_set_option(opt, (int *)&c->id2, (int *)&p->id2));
    TRYC(argopt_new(arg, opt->options, &c2, 0, RSTR("firefox"), RSTR("")));
    TRYC(argopt_new(arg, opt->options, &c2, 0, RSTR("zen"), RSTR("")));
    TRYC(argopt_new(arg, opt->options, &c2, 0, RSTR("brave"), RSTR("")));

    TRYC(argopt_set_option(c2, (int *)&c->id3, (int *)&p->id3));
    TRYC(argopt_new(arg, c2->options, &c3, 0, RSTR("private"), RSTR("")));
    TRYC(argopt_new(arg, c2->options, &c3, 0, RSTR("public"), RSTR("")));

    TRYC(argopt_new(arg, cat->options, &opt, 0, RSTR("lmao"), RSTR("specify what to do")));
    argopt_set_enum(opt, CONFIG_LMAO);
#endif

    return 0;
error:
    return -1;
}

#define TEST(msg)   do { \
        printf(F("=== %s ===", BOLD FG_BL_B) "\n", #msg); \
        arg_free(&arg); \
        TRYC(main_arg(&arg, argc, argv)); \
    } while(0)

#define TEST_ALL        0
#if TEST_ALL
#define TEST_EMPTY_LONG 1
#define TEST_TWIN_LONG  1
#define TEST_TWIN_SHORT 1
#endif

int hello_world(int *n) {
    printf("Hello, %i worlds!\n", *n);
    return 0;
}

int main(const int argc, const char **argv) {

    int err = 0;
    size_t n_arg = 0;
    Config config = {0};
    Config preset = {0};
    struct Arg *arg = arg_new();
    struct ArgX *x;
    struct ArgXGroup *g;
    ssize_t nfuck = 0;
    bool quit_early = false;

    arg_init(arg, argc, argv, RSTR("test_arg"), RSTR("this is a test program to verify the functionality of an argument parser. also, this is a very very long and boring description, just so I can check whether or not it wraps and end correctly! isn't that fascinating..."), RSTR("github: https://github.com/rphii"), '-', true, 0);

    x=argx_init(arg_opt(arg), n_arg++, 'h', RSTR("help"), RSTR("print this help"));
      argx_help(x, arg);
    x=argx_init(arg_opt(arg), n_arg++, 'b', RSTR("bool"), RSTR("boolean value"));
      argx_bool(x, &config.boolean, &preset.boolean);
    x=argx_init(arg_opt(arg), n_arg++, 'f', RSTR("double"), RSTR("double value"));
      argx_dbl(x, &config.number, &preset.number);
    x=argx_init(arg_opt(arg), n_arg++, 's', RSTR("string"), RSTR("string value"));
      argx_str(x, &config.string, &preset.string);
    x=argx_init(arg_opt(arg), n_arg++, 'i', RSTR("integer"), RSTR("integer value"));
      argx_int(x, &config.whole, &preset.whole);
    x=argx_init(arg_opt(arg), n_arg++, 'o', RSTR("option"), RSTR("select one option"));
      g=argx_opt(x, &config.id, &preset.id);
        x=argx_init(g, n_arg++, 0, RSTR("none"), RSTR("do nothing"));
          argx_opt_enum(x, CONFIG_NONE);
        x=argx_init(g, n_arg++, 0, RSTR("print"), RSTR("print stuff"));
          argx_opt_enum(x, CONFIG_PRINT);
        x=argx_init(g, n_arg++, 0, RSTR("browser"), RSTR("browse stuff"));
          argx_opt_enum(x, CONFIG_BROWSER);
        x=argx_init(g, n_arg++, 0, RSTR("lmao"), RSTR("what the fuck"));
          argx_opt_enum(x, CONFIG_LMAO);
          argx_int(x, &nfuck, 0);
          argx_func(x, hello_world, &nfuck, true);

    TRYC(arg_parse(arg, &quit_early));
    if(quit_early) return 0;

#if 0 /*{{{*/
    //arg_init(arg);

    ArgOpt *opt;
    Config c = {0};
    Config p = {0};

    p.config = RSTR("path/to/config");
    c.config = RSTR("path/to/config");
    p.verbose = 123;
    c.verbose = 100;
    p.math = 9.87;
    c.math = 1.23;
    //p.boring = false;
    //c.boring = false;

    TRYC(main_arg(&arg, &c, &p, argc, argv));
    //TRYC(arg_parse(&arg, 2, (const char *[]){argv[0], "-h"}));
    TRYC(arg_parse(&arg, argc, argv));

#if TEST_ALL
    printf("\n");
#if TEST_EMPTY_LONG
    TEST(TEST_EMPTY_LONG);
    if((opt = argopt_new(&arg, &arg.options, 0, RSTR(""), RSTR("")))) THROW("should not be able to create empty long option");
#endif

#if TEST_TWIN_LONG
    TEST(TEST_TWIN_LONG);
    if((opt = argopt_new(&arg, &arg.options, 0, RSTR("--help"), RSTR("")))) THROW("should not be able to create empty long option");
#endif

#if TEST_TWIN_SHORT
    TEST(TEST_TWIN_SHORT);
    if((opt = argopt_new(&arg, &arg.options, 'h', RSTR("--asdf"), RSTR("")))) THROW("should not be able to create empty long option");
#endif



    /* DONE; tests above */
    printf(F("=== ALL TESTS PASSED ===", FG_GN_B BOLD) "\n");
#endif
#endif /*}}}*/

clean:
    arg_free(&arg);
    return err;

error:
    ERR_CLEAN;
}

