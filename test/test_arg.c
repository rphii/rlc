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

typedef enum {
    CONFIG_MODE_NONE,
    CONFIG_MODE_HELLO,
    CONFIG_MODE_INT,
    CONFIG_MODE_FLOAT,
    CONFIG_MODE_STRING,
    CONFIG_MODE_BOOL,
} ConfigModeList;

typedef struct Config {
    ssize_t whole;
    bool boolean;
    double number;
    RStr config;
    RStr string;
    ConfigList id;
    ConfigList id2;
    ConfigList id3;
    struct {
        ConfigModeList id;
        ssize_t z;
        RStr s;
        double f;
        bool b;
    } mode;
    struct {
        bool safe;
        bool unsafe;
        bool other;
    } flags;
} Config;

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

    preset.flags.other = true;
    preset.flags.safe = true;
    preset.id = CONFIG_LMAO;
    preset.config = RSTR("path/to/config/that-is-very-long-and-unnecessary");

    arg_init(arg, RSTR("test_arg"), RSTR("this is a test program to verify the functionality of an argument parser. also, this is a very very long and boring description, just so I can check whether or not it wraps and end correctly! isn't that fascinating..."), RSTR("github: https://github.com/rphii"));
    arg_init_width(arg, 40, 45);

    x=argx_init(arg_opt(arg), n_arg++, 'h', RSTR("help"), RSTR("print this help"));
      argx_help(x, arg);
    x=argx_init(arg_opt(arg), n_arg++, 0, RSTR("xyz"), RSTR("nothing"));
    x=argx_init(arg_opt(arg), n_arg++, 'b', RSTR("bool"), RSTR("boolean value and a long description that is"));
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
          argx_func(x, hello_world, &nfuck, false);
        x=argx_init(g, n_arg++, 0, RSTR("test"), RSTR("what the fuck"));
          argx_opt_enum(x, 6);
        x=argx_init(g, n_arg++, 0, RSTR("useless"), RSTR("what the fuck"));
          argx_opt_enum(x, 7);
        x=argx_init(g, n_arg++, 0, RSTR("verbose"), RSTR("what the fuck"));
          argx_opt_enum(x, 8);
    //x=argx_init(arg_opt(arg), n_arg++, 0, RSTR("very-long-option-that-is-very-important-and-cool-but-serves-no-purpose-whatsoever-anyways-how-are-you-doing-today"), RSTR("select another option"));
    x=argx_init(arg_opt(arg), n_arg++, 'F', RSTR("flags"), RSTR("set different flags"));
      g=argx_flag(x);
        x=argx_init(g, n_arg++, 0, RSTR("safe"), RSTR("enable safe operation"));
          argx_flag_set(x, &config.flags.safe, &preset.flags.safe);
        x=argx_init(g, n_arg++, 0, RSTR("unsafe"), RSTR("enable unsafe operation"));
          argx_flag_set(x, &config.flags.unsafe, &preset.flags.unsafe);
        x=argx_init(g, n_arg++, 0, RSTR("other"), RSTR("enable other operation"));
          argx_flag_set(x, &config.flags.other, &preset.flags.other);

    x=argx_pos(arg, n_arg++, RSTR("mode"), RSTR("the main mode"));
      g=argx_opt(x, &config.mode.id, &preset.mode.id);
        x=argx_init(g, n_arg++, 0, RSTR("none"), RSTR("do nothing"));
          argx_opt_enum(x, CONFIG_MODE_NONE);
        x=argx_init(g, n_arg++, 0, RSTR("hello"), RSTR("print hello"));
          argx_func(x, hello_world, &nfuck, true);
          argx_opt_enum(x, CONFIG_MODE_HELLO);
        x=argx_init(g, n_arg++, 0, RSTR("int"), RSTR("set int"));
          argx_int(x, &config.mode.z, &preset.mode.z);
          argx_opt_enum(x, CONFIG_MODE_INT);
        x=argx_init(g, n_arg++, 0, RSTR("float"), RSTR("set float"));
          argx_dbl(x, &config.mode.f, &preset.mode.f);
          argx_opt_enum(x, CONFIG_MODE_FLOAT);
        x=argx_init(g, n_arg++, 0, RSTR("string"), RSTR("set string"));
          argx_str(x, &config.mode.s, &preset.mode.s);
          argx_opt_enum(x, CONFIG_MODE_STRING);
        x=argx_init(g, n_arg++, 0, RSTR("bool"), RSTR("set bool"));
          argx_bool(x, &config.mode.b, &preset.mode.b);
          argx_opt_enum(x, CONFIG_MODE_BOOL);

    argx_env(arg, RSTR("ARG_CONFIG_PATH"), RSTR("config path"), &config.config, &preset.config, true);

    TRYC(arg_parse(arg, argc, argv, &quit_early));
    if(quit_early) goto clean;

    //printff("ARG_CONFIG_PATH is = [%.*s]", RSTR_F(config.config));

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

