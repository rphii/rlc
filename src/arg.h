#ifndef ARG_H

#include "str.h"
#include "str2.h"

#define ARG_INIT_DEFAULT_WIDTH      80
#define ARG_INIT_DEFAULT_PREFIX     '-'
#define ARG_INIT_DEFAULT_SHOW_HELP  true
#define ARG_INIT_DEFAULT_FLAG_SEP   ','

#define ARG_F_ENV       BOLD FG_WT_B
#define ARG_VAL_F       FG_GN
#define ARG_TYPE_F      FG_CY
#define ARG_OPTION_F    FG_BL
#define ARG_FLAG_F      FG_YL
#define ARG_F_SEP       FG_BK_B

/* types of arguments
 *
 * */

struct Arg;
struct ArgX;
struct TArgX;
struct ArgXGroup;

typedef int (*ArgFunction)(void *);

struct Arg *arg_new(void);
void arg_init(struct Arg *arg, Str2 program, Str2 description, Str2 epilog);
void arg_init_width(struct Arg *arg, int width, int percent);
void arg_init_show_help(struct Arg *arg, bool show_help);
void arg_init_prefix(struct Arg *arg, unsigned char prefix);
void arg_init_rest(struct Arg *arg, Str2 description, VStr2 *rest);

void arg_free(struct Arg **arg);

struct ArgXGroup *arg_opt(struct Arg *arg);

struct ArgX *argx_pos(struct Arg *arg, Str2 opt, Str2 desc);
void argx_env(struct Arg *arg, Str2 opt, Str2 desc, Str2 *val, Str2 *ref, bool hide_value);
struct ArgX *argx_init(struct ArgXGroup *group, const unsigned char c, const Str2 optX, const Str2 descX);

void argx_str(struct ArgX *x, Str2 *val, Str2 *ref);
void argx_ssz(struct ArgX *x, ssize_t *val, ssize_t *ref);
void argx_int(struct ArgX *x, int *val, int *ref);
void argx_dbl(struct ArgX *x, double *val, double *ref);
void argx_bool(struct ArgX *x, bool *val, bool *ref);
void argx_none(struct ArgX *x);
void argx_vstr(struct ArgX *x, VStr2 *val, VStr2 *ref);
struct ArgXGroup *argx_opt(struct ArgX *x, int *val, int *ref);
struct ArgXGroup *argx_flag(struct ArgX *x);
void argx_opt_enum(struct ArgX *x, int val);
void argx_flag_set(struct ArgX *x, bool *val, bool *ref);
void argx_type(struct ArgX *x, Str2 type);
void argx_func(struct ArgX *x, size_t priority, void *func, void *data, bool quit_early);
void argx_help(struct ArgX *x, struct Arg *arg);
void argx_hide_value(struct ArgX *x, bool hide_value);

struct ArgX *argx_new(struct TArgX *group, const unsigned char c, const Str2 opt, const Str2 desc);

void arg_help_set(struct Arg *arg, struct ArgX *x);
int arg_help(struct Arg *arg);
void arg_config(struct Arg *arg, Str2 conf);


#if 0
Str2 arg_info_opt(struct Arg *arg, void *x);
#endif

#define ERR_arg_parse(...) "failed parsing arguments"
ErrDecl arg_parse(struct Arg *arg, const unsigned int argc, const char **argv, bool *quit_early);

#define ARG_H
#endif

