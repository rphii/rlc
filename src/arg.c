#include "arg.h"
#undef SSIZE_MAX
#include <sys/ioctl.h>
#include <unistd.h>

typedef enum {
    ARG_NONE,
    ARG_BOOL,
    ARG_FLAG,
    ARG_INT,
    ARG_FLOAT,
    ARG_STRING,
    ARG_ENV,
    ARG_POS,
    //ARG_EXOTIC,
    ARG_OPTION,
    ARG_FLAGS,
    ARG_HELP,
    /* above */
    ARG__COUNT
} ArgList;

const char *arglist_str(ArgList id) {
    switch(id) {
        case ARG_NONE: return "<none>";
        case ARG_BOOL: return "<bool>";
        case ARG_FLAG: return "<flag>";
        case ARG_INT: return "<int>";
        case ARG_FLOAT: return "<double>";
        case ARG_STRING: return "<string>";
        case ARG_ENV: return "<env>";
        case ARG_POS: return "<pos>";
        case ARG_OPTION: return "<option>";
        case ARG_HELP: return "<help>";
        case ARG_FLAGS: return "<flags>";
        //case ARG_EXOTIC: return "<exotic>";
        case ARG__COUNT: return "(internal:count)";
    }
    return "(internal:invalid)";
}

typedef struct ArgBase {
    RStr program;           // program name
    RStr desc;              // description of program
    RStr epilog;            // text below argument help
    unsigned char prefix;   // default: -
    unsigned char flag_sep; // default: ,
    bool show_help;         // display help if no arguments provided
    VrStr *rest;
} ArgBase;

typedef union ArgXVal {
    RStr *s;            // string
    ssize_t *z;         // number
    double *f;          // double
    bool *b;            // bool
    void *x;            // exotic
    int e;              // enum
} ArgXVal;

typedef struct ArgXCallback {
    ArgFunction func;
    void *data;
    bool quit_early;
} ArgXCallback;

typedef union ArgXNumber {
    ssize_t z;
    float f;
} ArgXNumber;

typedef struct ArgX { /*{{{*/

    ArgList id;
    ArgXVal val;
    ArgXVal ref;
    int e; // enum
    struct ArgXGroup *o; // option
    struct ArgXGroup *group; // required for options / parent group
    struct {
        ArgXNumber min;
        ArgXNumber max;
        ArgXCallback callback;
    } attr;
    struct {
        const unsigned char c;
        const RStr opt;
        const RStr desc;
        const size_t index;
    } info;

} ArgX; /*}}}*/

void argx_free(struct ArgX *argx);
void argx_group_free(struct ArgXGroup *group);

#include "lut.h"
LUT_INCLUDE(TArgX, targx, RStr, BY_VAL, struct ArgX, BY_REF);
LUT_IMPLEMENT(TArgX, targx, RStr, BY_VAL, struct ArgX, BY_REF, rstr_hash, rstr_cmp, 0, argx_free);

int argx_cmp_index(ArgX *a, ArgX *b) {
    ASSERT_ARG(a);
    ASSERT_ARG(b);
    return a->info.index - b->info.index;
}

#include "vec.h"
VEC_INCLUDE(VArgX, vargx, ArgX *, BY_VAL, BASE);
VEC_INCLUDE(VArgX, vargx, ArgX *, BY_VAL, ERR);
VEC_INCLUDE(VArgX, vargx, ArgX *, BY_VAL, SORT);
VEC_IMPLEMENT(VArgX, vargx, ArgX *, BY_VAL, BASE, 0);
VEC_IMPLEMENT(VArgX, vargx, ArgX *, BY_VAL, ERR);
VEC_IMPLEMENT(VArgX, vargx, ArgX *, BY_VAL, SORT, argx_cmp_index);

typedef struct ArgXGroup {
    VArgX vec;
    TArgX lut;
    RStr desc;
    struct Arg *root; // required literally only for assigning short options
    struct ArgX *parent; // required for options
} ArgXGroup;

typedef struct ArgParse {
    int argc;
    char **argv;
    bool force_done_parsing;
    size_t i;
    size_t n_pos_parsed;
    VArgX queue;
    ArgBase *base;  // need the info of prefix...
    struct {
        bool get;
        ArgX *x;
    } help;
} ArgParse;

typedef enum {
    ARG_PRINT_NONE,
    /* below */
    ARG_PRINT_SHORT,
    ARG_PRINT_LONG,
    ARG_PRINT_TYPE,
    ARG_PRINT_DESC,
    ARG_PRINT_VALUE,
    /* above */
    ARG_PRINT__LENGTH
} ArgPrintList;

typedef struct ArgPrint {
    struct {
        int max;    // max width
        int desc;   // spacing until description
        int c;      // spacing until short option
        int opt;    // spacing until long option
    } bounds;
    int pad;        // current padding
    int progress;   // how many characters already printed on line
    int relevant;   // 
    Str line;       // current line
    Str buf;        // temporary buffer
    bool wrapped;   // wheter or not the last line was wrapped
    ArgPrintList prev;  // previous print thing
} ArgPrint;

typedef struct Arg {
    ArgBase base;
    ArgXGroup pos;
    ArgXGroup opt;
    ArgXGroup env;
    ArgX *opt_short[256];
    ArgParse parse;
    ArgPrint print;
} Arg;


ATTR_NODISCARD struct Arg *arg_new(void) {
    Arg *result = 0;
    NEW(Arg, result);
    return result;
}
ATTR_NODISCARD struct ArgXGroup *wargx_new(void) {
    ArgXGroup *result = 0;
    NEW(ArgXGroup, result);
    return result;
}

ArgXGroup *arg_pos(Arg *arg) {
    return &arg->pos;
}
ArgXGroup *arg_opt(Arg *arg) {
    return &arg->opt;
}
//ArgXGroup *arg_env(Arg *arg) {
//    return &arg->env;
//}

void arg_init(struct Arg *arg, RStr program, RStr description, RStr epilog) {
    ASSERT_ARG(arg);
    arg->base.program = program;
    arg->base.desc = description;
    arg->base.epilog = epilog;
    arg->base.prefix = ARG_INIT_DEFAULT_PREFIX;
    arg->base.show_help = ARG_INIT_DEFAULT_SHOW_HELP;
    arg->base.flag_sep = ARG_INIT_DEFAULT_FLAG_SEP;
    arg->pos.desc = RSTR("Usage");
    arg->opt.desc = RSTR("Options");
    arg->env.desc = RSTR("Environment Variables");
    arg->opt.root = arg;
    arg->parse.base = &arg->base;
    arg->print.bounds.c = 2;
    arg->print.bounds.opt = 6;
    arg_init_width(arg, 80, 45);
}

void arg_init_width(struct Arg *arg, int width, int percent) {
    ASSERT_ARG(arg);
    if(width == 0) {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        width = w.ws_col;
    }
    int desc = width * 45;
    if(desc % 200) desc += 200 - desc % 200;
    desc /= 100;
    arg->print.bounds.desc = desc;
    arg->print.bounds.max = width;
}

void arg_init_show_help(struct Arg *arg, bool show_help) {
    ASSERT_ARG(arg);
    arg->base.show_help = show_help;
}

void arg_init_prefix(struct Arg *arg, unsigned char prefix) {
    ASSERT_ARG(arg);
    arg->base.prefix = prefix;
}

void arg_init_rest(struct Arg *arg, VrStr *rest) {
    ASSERT_ARG(arg);
    arg->base.rest = rest;
}


#define ERR_argx_group_push(...) "failed adding argument x"
ErrDecl argx_group_push(ArgXGroup *group, ArgX *in, ArgX **out) {
    ASSERT_ARG(group);
    ASSERT_ARG(in);
    TArgXKV *xkv = targx_once(&group->lut, in->info.opt, in);
    if(!xkv) THROW("option '%.*s' already exists!", RSTR_F(in->info.opt));
    TRYG(vargx_push_back(&group->vec, xkv->val));
    //return xkv->val;
    if(out) *out = xkv->val;
    return 0;
error:
    //ABORT("critical error in " F("[%.*s]", BOLD) " -- aborting", RSTR_F(group->desc));
    return -1;
}

struct ArgX *argx_init(struct ArgXGroup *group, size_t index, const unsigned char c, const RStr optX, const RStr descX) {
    ASSERT_ARG(group);
    RStr opt = rstr_trim(optX);
    RStr desc = rstr_trim(descX);
    if(!rstr_length(opt)) ABORT("cannot add an empty long-opt argument");
    ArgX x = {
        .info = {c, opt, desc, index},
        .group = group,
    };
    ArgX *argx = 0;
    TRYC(argx_group_push(group, &x, &argx));
    if(c) {
        if(!group->root) ABORT("cannot specify short option '%c' for '%.*s'", c, RSTR_F(opt));
        ArgX *xx = group->root->opt_short[c];
        if(xx) ABORT("already specified short option '%c' for '%.*s'; cannot set for '%.*s' as well.", c, RSTR_F(xx->info.opt), RSTR_F(opt));
        group->root->opt_short[c] = argx;
    }
    return argx;
error:
    ABORT("critical error in " F("[%.*s]", BOLD) " -- aborting", RSTR_F(group->desc));
    return 0;
}

/* 1) ASSIGN MAIN ID {{{ */

void argx_str(ArgX *x, RStr *val, RStr *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_STRING;
    x->val.s = val;
    x->ref.s = ref;
}
void argx_int(ArgX *x, ssize_t *val, ssize_t *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_INT;
    x->val.z = val;
    x->ref.z = ref;
}
void argx_dbl(ArgX *x, double *val, double *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_FLOAT;
    x->val.f = val;
    x->ref.f = ref;
}
void argx_bool(ArgX *x, bool *val, bool *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_BOOL;
    x->val.b = val;
    x->ref.b = ref;
}
void argx_flag_set(ArgX *x, bool *val, bool *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_FLAG;
    x->val.b = val;
    x->ref.b = ref;
}
void argx_none(ArgX *x) {
    ASSERT_ARG(x);
    x->id = ARG_NONE;
}
struct ArgXGroup *argx_opt(ArgX *x, void *val, void *ref) {
    ASSERT_ARG(x);
    ArgXGroup *options = wargx_new();
    options->desc = x->info.opt;
    options->parent = x;
    x->id = ARG_OPTION;
    x->val.z = (ssize_t *)val;
    x->ref.z = (ssize_t *)ref;
    x->o = options;
    return options;
}

struct ArgXGroup *argx_flag(struct ArgX *x) {
    ArgXGroup *flags = wargx_new();
    flags->desc = x->info.opt;
    flags->parent = x;
    x->id = ARG_FLAGS;
    x->o = flags;
    return flags;
}

void argx_help(struct ArgX *x, struct Arg *arg) {
    ASSERT_ARG(x);
    x->id = ARG_HELP;
    x->attr.callback.func = (ArgFunction)arg_help;
    x->attr.callback.data = arg;
    x->attr.callback.quit_early = true;
}

struct ArgX *argx_pos(struct Arg *arg, size_t index, RStr opt, RStr desc) {
    ASSERT_ARG(arg);
    ArgX *x = argx_init(&arg->pos, index, 0, opt, desc);
    return x;
}

void argx_env(struct Arg *arg, RStr opt, RStr desc, RStr *val, RStr *ref) {
    ASSERT_ARG(arg);
    ASSERT_ARG(val);
    ArgX *x = argx_init(&arg->env, 0, 0, opt, desc);
    x->id = ARG_ENV;
    x->val.s = val;
    x->ref.s = ref;
}

/* }}} */

/* 2) ASSIGN SPECIFIC OPTIONS {{{ */

void argx_int_mm(ArgX *x, ssize_t min, ssize_t max) {
    ASSERT_ARG(x);
    if(x->id != ARG_INT) ABORT("wrong argx type in '%.*s' to set min/max: %s", RSTR_F(x->info.opt), arglist_str(x->id));
    x->attr.min.z = min;
    x->attr.max.z = max;
}

void argx_dbl_mm(ArgX *x, double min, double max) {
    ASSERT_ARG(x);
    if(x->id != ARG_FLOAT) ABORT("wrong argx type in '%.*s' to set min/max: %s", RSTR_F(x->info.opt), arglist_str(x->id));
    x->attr.min.f = min;
    x->attr.max.f = max;
}

void argx_func(struct ArgX *x, void *func, void *data, bool quit_early) {
    ASSERT_ARG(x);
    ASSERT_ARG(func);
    x->attr.callback.func = func;
    x->attr.callback.data = data;
    x->attr.callback.quit_early = quit_early;
}
void argx_opt_enum(struct ArgX *x, int val) {
    ASSERT_ARG(x);
    if(!(x->group && x->group->parent) || x->group->parent->id != ARG_OPTION) {
        ABORT("can only set enums to child nodes of options " F("[%.*s]", BOLD), RSTR_F(x->info.opt));
    }
    if(!x->group->parent->val.z) {
        ABORT("parent " F("[%.*s]", BOLD) " has to be assigned to an enum", RSTR_F(x->group->parent->info.opt));
    }
    x->e = val;
    //printff("SET [%.*s] ENUM TO %i", RSTR_F(x->info.opt), val);
}

/* }}}*/

/* PRINTING FUNCTIONS {{{ */

void arg_do_print(Arg *arg, bool endline) {
    ASSERT_ARG(arg);
    Str *line = &arg->print.line;
    size_t len_nof = str_length_nof(arg->print.line);
    size_t len = str_length(arg->print.line);
    RStr fmt = RSTR("");
    size_t f0, fE;
    bool repeat = false;
    //printff("DO[%.*s] PROG[%zu] LEN[%zu] LEN_NOF[%zu] PAD[%zu]", STR_F(arg->print.line), arg->print.progress, len, len_nof, arg->print.pad);
    int pad = arg->print.progress > arg->print.pad ? 0 : arg->print.pad - arg->print.progress;
    if(!len) {
        /* TODO: DRY */
        if(endline) {
            printf("\n");
            arg->print.progress = 0;
        }
    }
    while(len) {

        size_t relevant_progress = (!repeat ? arg->print.progress : 0);
        if(repeat) {
            pad = arg->print.pad;
            relevant_progress = 0;
        }

        printf("%*s", pad, "");
        if(repeat) {
            printf(F("..", FG_BK_B));
        }

        size_t m = len;
        if(len_nof + pad + (repeat ? 4 : 2) + relevant_progress > arg->print.bounds.max) {
            m = arg->print.bounds.max - relevant_progress - pad - (repeat ? 4 : 2);
        }
        size_t n = str_index_nof(*line, m);

#if 1
        RStr line_print = str_rstr(STR_IE(*line, n));
        f0 = rstr_rfind_f(line_print, &fE);
        //if(f0+fE < n) fmt = str_rstr(STR_IE(STR_I0(*line, f0), fE));
        printf("%.*s%.*s", RSTR_F(fmt), RSTR_F(line_print));
        if(f0 < n) fmt = str_rstr(STR_IE(STR_I0(*line, f0), fE));
#else
        RStr line_print = str_rstr(STR_IE(*line, n));
        printf("%.*s", RSTR_F(line_print));
#endif

        if(len_nof + pad + (repeat ? 4 : 2) + relevant_progress > arg->print.bounds.max) {
            printf(F("..", FG_BK_B) "\n");
            arg->print.progress += n;
            line->first += n;
        } else {
            if(endline) {
                printf("\n");
                arg->print.progress = 0;
            } else {
                arg->print.progress += pad + str_length_nof(*line);
            }
            str_clear(line);
        }
        len_nof = str_length_nof(*line);
        len = str_length(*line);
        repeat = true;
    }
}

void arg_handle_print(Arg *arg, ArgPrintList id, const char *format, ...) {
    ASSERT_ARG(arg);
    ASSERT_ARG(format);
    /* start string fmt */
    str_clear(&arg->print.buf);
    va_list argp;
    va_start(argp, format);
    int result = str_fmt_va(&arg->print.buf, format, argp);
    va_end(argp);
    if(result) THROW(ERR_MALLOC);
    /* do padding */
    switch(id) {
        case ARG_PRINT_NONE: {
            arg->print.pad = 0;
            TRYG(str_extend_back(&arg->print.line, arg->print.buf));
            arg_do_print(arg, false);
            arg->print.progress = 0;
        } break;
        case ARG_PRINT_SHORT: {
            arg->print.pad = arg->print.bounds.c;
        } goto ARG_PRINT__KEEPLINE;
        case ARG_PRINT_LONG: {
            arg->print.pad = arg->print.bounds.opt;
        } goto ARG_PRINT__KEEPLINE;
        /* special cases */
        case ARG_PRINT_TYPE: {
            if(arg->print.prev == ARG_PRINT_LONG) {
                str_copy(&arg->print.line, &STR(" "));
                arg_do_print(arg, false);
                str_clear(&arg->print.line);
                arg->print.pad = arg->print.bounds.opt + 2;
            }
            TRYG(str_extend_back(&arg->print.line, arg->print.buf));
        } break;
        case ARG_PRINT_DESC: {
            if(arg->print.prev == ARG_PRINT_TYPE) {
                arg_do_print(arg, false);
                if(arg->print.progress + 1 > arg->print.bounds.desc) {
                    str_copy(&arg->print.line, &STR(""));
                    arg_do_print(arg, true);
                    arg->print.pad = arg->print.bounds.opt + 4;
                } else {
		    TRYC(str_fmt(&arg->print.line, " "));
		    /////for(size_t i = arg->print.progress + 2; i < arg->print.bounds.desc; ++i) {
		    /////    //TRYC(str_fmt(&arg->print.line, "·"));
            /////    TRYC(str_fmt(&arg->print.line, F("%c", FG_BK_B), !((i + 1) % 2) ? '-' : ' '));
            /////}
            arg_do_print(arg, false);
            arg->print.pad = arg->print.bounds.desc;
                }
            } else {
                arg->print.pad = arg->print.bounds.desc;
            }
            str_clear(&arg->print.line);
            TRYG(str_extend_back(&arg->print.line, arg->print.buf));
            arg_do_print(arg, false);
        } break;
        case ARG_PRINT_VALUE: {
            if(arg->print.prev == ARG_PRINT_DESC) {
                str_copy(&arg->print.line, &STR(" "));
                arg_do_print(arg, false);
                arg->print.pad = arg->print.bounds.opt + 6; //arg->print.progress;
            }
            str_clear(&arg->print.line);
            TRYG(str_extend_back(&arg->print.line, arg->print.buf));
            arg_do_print(arg, true);
        } break;
        ARG_PRINT__KEEPLINE: {
            TRYG(str_extend_back(&arg->print.line, arg->print.buf));
            arg_do_print(arg, false);
        } break;
        case ARG_PRINT__LENGTH: ABORT(ERR_UNREACHABLE);
    }
    arg->print.prev = id;
    return;
error:
    ABORT(ERR_INTERNAL("formatting error"));
    //return result;
}

void argx_print_pre(Arg *arg, ArgX *argx) { /*{{{*/
    switch(argx->id) {
        case ARG_STRING:
        case ARG_INT:
        case ARG_FLAG:
        case ARG_BOOL:
        case ARG_FLOAT: {
            arg_handle_print(arg, ARG_PRINT_TYPE, F("%s", ARG_TYPE_F), arglist_str(argx->id));
        } break;
        case ARG_OPTION: {
            ArgXGroup *g = argx->o;
            if(vargx_length(g->vec)) {
                arg_handle_print(arg, ARG_PRINT_TYPE, F("<", ARG_OPTION_F));
                for(size_t i = 0; i < vargx_length(g->vec); ++i) {
                    if(i) arg_handle_print(arg, ARG_PRINT_TYPE, F("|", ARG_OPTION_F));
                    ArgX *x = vargx_get_at(&g->vec, i);
                    //ASSERT_ARG(x->group);
                    //ASSERT_ARG(x->group->parent);
                    //ASSERT_ARG(x->group->parent->val.z);
                    if(x->group && x->group->parent && x->group->parent->val.z && *x->group->parent->val.z == i) {
                        arg_handle_print(arg, ARG_PRINT_TYPE, F("%.*s", ARG_OPTION_F UL), RSTR_F(x->info.opt));
                    } else {
                        arg_handle_print(arg, ARG_PRINT_TYPE, F("%.*s", ARG_OPTION_F), RSTR_F(x->info.opt));
                    }
                }
                arg_handle_print(arg, ARG_PRINT_TYPE, F(">", ARG_OPTION_F));
            }
        } break;
        case ARG_FLAGS: {
            ArgXGroup *g = argx->o;
            if(vargx_length(g->vec)) {
                arg_handle_print(arg, ARG_PRINT_TYPE, F("<", ARG_FLAG_F));
                for(size_t i = 0; i < vargx_length(g->vec); ++i) {
                    if(i) arg_handle_print(arg, ARG_PRINT_TYPE, F("|", ARG_FLAG_F));
                    ArgX *x = vargx_get_at(&g->vec, i);
                    ASSERT_ARG(x->group);
                    ASSERT_ARG(x->group->parent);
                    ASSERT(x->id == ARG_FLAG, "the option [%.*s] in [--%.*s] should be set as a %s", RSTR_F(x->info.opt), RSTR_F(x->group->parent->info.opt), arglist_str(ARG_FLAG));
                    if(*x->val.b) {
                        arg_handle_print(arg, ARG_PRINT_TYPE, F("%.*s", ARG_FLAG_F UL), RSTR_F(x->info.opt));
                    } else {
                        arg_handle_print(arg, ARG_PRINT_TYPE, F("%.*s", ARG_FLAG_F), RSTR_F(x->info.opt));
                    }
                }
                arg_handle_print(arg, ARG_PRINT_TYPE, F(">", ARG_FLAG_F));
            }
        } break;
        case ARG_HELP: {
            arg_handle_print(arg, ARG_PRINT_TYPE, F("<arg>", ARG_TYPE_F));
        } break;
        case ARG_POS:
        case ARG_ENV:
        case ARG_NONE:
        case ARG__COUNT: break;
    }
} /*}}}*/

void argx_print_post(Arg *arg, ArgX *argx, ArgXVal *val) { /*{{{*/
    ASSERT_ARG(argx);
    ASSERT_ARG(val);
    ArgXVal out = *val;
    switch(argx->id) {
        case ARG_POS:
        case ARG_ENV:
        case ARG_STRING: {
            if(val->s && rstr_length(*val->s)) {
                arg_handle_print(arg, ARG_PRINT_VALUE, F("=", FG_BK_B) F("%.*s", ARG_VAL_F), RSTR_F(*val->s));
            } else {
                arg_handle_print(arg, ARG_PRINT_VALUE, "");
            }
        } break;
        case ARG_INT: {
            ssize_t zero = 0;
            if(!val->z) out.z = &zero;
            arg_handle_print(arg, ARG_PRINT_VALUE, F("=", FG_BK_B) F("%zi", ARG_VAL_F), *out.z);
        } break;
        case ARG_FLAG:
        case ARG_BOOL: {
            bool zero = 0;
            if(!val->b) out.b = &zero;
            arg_handle_print(arg, ARG_PRINT_VALUE, F("=", FG_BK_B) F("%s", ARG_VAL_F), *out.b ? "true" : "false");
        } break;
        case ARG_FLOAT: {
            double zero = 0;
            if(!val->f) out.f = &zero;
            arg_handle_print(arg, ARG_PRINT_VALUE, F("=", FG_BK_B) F("%f", ARG_VAL_F), *out.f);
        } break;
        case ARG_HELP:
        case ARG_OPTION:
        case ARG_FLAGS:
        case ARG_NONE: {
            arg_handle_print(arg, ARG_PRINT_VALUE, "");
        } break;
        case ARG__COUNT: break;
    }
} /*}}}*/

void argx_print(Arg *arg, ArgX *x, bool detailed) { /*{{{*/
    unsigned char pfx = arg->base.prefix;
    /* print short form */
    if(x->info.c) {
        arg_handle_print(arg, ARG_PRINT_SHORT, F("%c%c", BOLD FG_WT_B), pfx, x->info.c);
    }
    /* print long form (should always be available) */
    //printff("[%.*s] %s group %p", RSTR_F(x->info.opt), arglist_str(x->id), x->group);
    if(x->group && x->group == &arg->pos) {
        arg_handle_print(arg, ARG_PRINT_LONG, F("%.*s", IT), RSTR_F(x->info.opt));
    } else if(x->id == ARG_ENV) {
        arg_handle_print(arg, ARG_PRINT_SHORT, F("%.*s", ARG_F_ENV), RSTR_F(x->info.opt));
    } else if(x->group && x->group->parent) {
        arg_handle_print(arg, ARG_PRINT_LONG, F("  %.*s", BOLD FG_WT_B), RSTR_F(x->info.opt));
    } else {
        arg_handle_print(arg, ARG_PRINT_LONG, F("%c%c%.*s", BOLD FG_WT_B), pfx, pfx, RSTR_F(x->info.opt));
    }
    //arg_handle_print(arg, ARG_PRINT_DESC, " ");
    argx_print_pre(arg, x);
    /* print description */
    if(rstr_length(x->info.desc)) {
        arg_handle_print(arg, ARG_PRINT_DESC, "%.*s", RSTR_F(x->info.desc));
    }
    bool is_detailed_option = false;
    if(detailed && x->id == ARG_OPTION && x->o) {
        is_detailed_option = true;
        arg_handle_print(arg, ARG_PRINT_NONE, "\n\n");
        for(size_t i = 0; i < vargx_length(x->o->vec); ++i) {
            //arg_handle_print(arg, ARG_PRINT_NONE, "\n");
            ArgX *xx = vargx_get_at(&x->o->vec, i);
            argx_print(arg, xx, false);
        }
    }
    /* print value */
    if(detailed) {
        arg_handle_print(arg, ARG_PRINT_NONE, "\n");
        if(!is_detailed_option) arg_handle_print(arg, ARG_PRINT_NONE, "\n");
        arg_handle_print(arg, ARG_PRINT_SHORT, "current value");
    }
    if(detailed && x->id == ARG_OPTION && x->o) {
        ssize_t *n = x->val.z;
        for(size_t i = 0; n && i < vargx_length(x->o->vec); ++i) {
            ArgX *xx = vargx_get_at(&x->o->vec, i);
            if(xx->e != *n) continue;
            argx_print(arg, xx, false);
            break;
        }
        if(!n) {
            arg_handle_print(arg, ARG_PRINT_NONE, "\n");
        }
    } else {
        argx_print_post(arg, x, &x->val);
    }
    if(detailed) {
        arg_handle_print(arg, ARG_PRINT_SHORT, "default value");
        argx_print_post(arg, x, &x->ref);
        //arg_handle_print(arg, ARG_PRINT_NONE, "\n");
    }
    /* done */
    //rg_handle_print(arg, ARG_PRINT_NONE, "\n");
} /*}}}*/

void argx_print_specific(Arg *arg, ArgParse *parse, ArgX *x) { /*{{{*/
    if(x->group) {
        if(x->group->parent) {
            argx_print_specific(arg, parse, x->group->parent);
        } else {
            arg_handle_print(arg, ARG_PRINT_NONE, F("%.*s:\n", BOLD UL), RSTR_F(x->group->desc));
        }
    }
    argx_print(arg, x, (x == parse->help.x));
    //argx_print(base, x, false);
} /*}}}*/

void argx_group_print(Arg *arg, ArgXGroup *group) { /*{{{*/
    ASSERT_ARG(arg);
    ASSERT_ARG(group);
    if(!vargx_length(group->vec)) {
        return;
    }
    if(rstr_length(group->desc)) {
        arg_handle_print(arg, ARG_PRINT_NONE, F("%.*s:", BOLD UL), RSTR_F(group->desc));
        arg_handle_print(arg, ARG_PRINT_NONE, "\n");
    }
    if(group == &arg->pos) {
        arg_handle_print(arg, ARG_PRINT_SHORT, F("%s", BOLD), arg->parse.argv[0]);
        for(size_t i = 0; i < vargx_length(group->vec); ++i) {
            ArgX *x = vargx_get_at(&group->vec, i);
            printf(" %.*s", RSTR_F(x->info.opt));
            //argx_print(arg, x, false);
        }
        arg_handle_print(arg, ARG_PRINT_NONE, "\n");
    }
    for(size_t i = 0; i < vargx_length(group->vec); ++i) {
        ArgX *x = vargx_get_at(&group->vec, i);
        argx_print(arg, x, false);
    }
} /*}}}*/

int arg_help(struct Arg *arg) { /*{{{*/
    ASSERT_ARG(arg);
    if(arg->parse.help.x && arg->parse.help.get) {
        /* specific help */
        argx_print_specific(arg, &arg->parse, arg->parse.help.x);
    } else {
        /* default help */
        arg_handle_print(arg, ARG_PRINT_NONE, F("%.*s:", BOLD) " %.*s", RSTR_F(arg->base.program), RSTR_F(arg->base.desc));
        printf("\n");
        argx_group_print(arg, &arg->pos);
        argx_group_print(arg, &arg->opt);
        argx_group_print(arg, &arg->env);
        if(rstr_length(arg->base.epilog)) {
            arg_handle_print(arg, ARG_PRINT_NONE, "%.*s", RSTR_F(arg->base.epilog));
            printf("\n");
        }
    }
    return 0;
} /*}}}*/

/* }}} */

void argx_free(ArgX *argx) {
    ASSERT_ARG(argx);
    if(argx->id == ARG_OPTION || argx->id == ARG_FLAGS) {
        vargx_free(&argx->o->vec);
        targx_free(&argx->o->lut);
        free(argx->o);
    }
    memset(argx, 0, sizeof(*argx));
};

void argx_group_free(ArgXGroup *group) {
    ASSERT_ARG(group);
    targx_free(&group->lut);
    vargx_free(&group->vec);
    memset(group, 0, sizeof(*group));
}

#if 0
bool arg_parse_done(ArgParse *parse) {
    return !(parse->i < parse->argc);
}
#endif

#define ERR_arg_parse_getopt(group, ...) "failed getting option for " F("[%.*s]", BOLD), RSTR_F((group)->desc)
ErrDecl arg_parse_getopt(ArgXGroup *group, ArgX **x, RStr opt) {
    ASSERT_ARG(group);
    ASSERT_ARG(x);
    *x = targx_get(&group->lut, opt);
    if(!*x) THROW("value " F("%.*s", FG_BL_B) " is not a valid option", RSTR_F(opt));
    return 0;
error:
    return -1;
}

#define ERR_arg_parse_getopt_short(arg, ...) "failed getting short option"
ErrDecl arg_parse_getopt_short(Arg *arg, ArgX **x, const unsigned char c) {
    ASSERT_ARG(arg);
    ASSERT_ARG(x);
    *x = arg->opt_short[c];
    if(!*x) THROW("value " F("%c", FG_BL_B) " is not a valid short option", c);
    return 0;
error:
    return -1;
}

#define ERR_arg_parse_getv(...) "failed getting an argument"
ErrDecl arg_parse_getv(ArgParse *parse, RStr *argV, bool *need_help) {
    ASSERT_ARG(parse);
    ASSERT_ARG(parse->argv);
    ASSERT_ARG(argV);
    unsigned int pfx = parse->base->prefix;
    RStr result;
repeat:
    if(parse->i < parse->argc) {
        char *argv = parse->argv[parse->i++];
        result = RSTR_L(argv);
        if(!parse->force_done_parsing && rstr_length(result) == 2 && rstr_get_at(&result, 0) == pfx && rstr_get_at(&result, 1) == pfx) {
            parse->force_done_parsing = true;
            goto repeat;
        }
        *argV = result;
        //printff("GOT ARGUMENT %.*s", RSTR_F(*argV));
    } else {
        if(!parse->help.get) {
            THROW("no arguments left");
        } else {
            *need_help = true;
        }
    }
    return 0;
error:
    return -1;
}

void arg_parse_getv_undo(ArgParse *parse) {
    ASSERT_ARG(parse);
    ASSERT_ARG(parse->argv);
    ASSERT(parse->i, "nothing left to undo");
    --parse->i;
}

#define ERR_argx_parse(parse, argx) "failed parsing argument " F("[%.*s]", BOLD FG_WT_B) " " F("%s", ARG_TYPE_F), RSTR_F(argx->info.opt), arglist_str(argx->id)
ErrDecl argx_parse(ArgParse *parse, ArgX *argx) {
    ASSERT_ARG(parse);
    ASSERT_ARG(argx);
    //printff("PARSE [%.*s]", RSTR_F(argx->info.opt));
    /* add to queue for post processing */
    TRYG(vargx_push_back(&parse->queue, argx));
    RStr argV = RSTR("");
    /* check if we want to get help for this */
    if(parse->help.get) {
        if(!parse->help.x || (argx->group ? parse->help.x == argx->group->parent : false)) {
            parse->help.x = argx;
            //printff("GETTING HELP [%.*s]", RSTR_F(parse->help.x->info.opt));
        }
    }
    /* check enum / option */
    if(argx->group && argx->group->parent && argx->group->parent->id == ARG_OPTION) {
        *argx->group->parent->val.z = argx->e;
    }
    /* actually begin parsing */
    bool need_help = false;
    switch(argx->id) {
        case ARG_BOOL: { //printff("GET VALUE FOR BOOL");
            if(parse->i < parse->argc) {
                TRYC(arg_parse_getv(parse, &argV, &need_help)); //printff("GOT VALUE [%.*s]", RSTR_F(argV));
                if(need_help) break;
            }
            if(rstr_as_bool(argV, argx->val.b, true)) {
                *argx->val.b = argx->ref.b ? !*argx->ref.b : true;
                arg_parse_getv_undo(parse);
            }
        } break;
        case ARG_FLAG: {
            *argx->val.b = true;
        } break;
        case ARG_INT: {
            TRYC(arg_parse_getv(parse, &argV, &need_help));
            if(need_help) break;
            TRYC(rstr_as_int(argV, argx->val.z));
        } break;
        case ARG_FLOAT: {
            TRYC(arg_parse_getv(parse, &argV, &need_help));
            if(need_help) break;
            TRYC(rstr_as_double(argV, argx->val.f));
        } break;
        case ARG_STRING: {
            TRYC(arg_parse_getv(parse, &argV, &need_help));
            if(need_help) break;
            *argx->val.s = argV;
        } break;
        case ARG_OPTION: {
            TRYC(arg_parse_getv(parse, &argV, &need_help));
            if(need_help) break;
            ArgX *x = 0;
            TRYC(arg_parse_getopt(argx->o, &x, argV));
            TRYC(argx_parse(parse, x));
        } break;
        case ARG_FLAGS: {
            TRYC(arg_parse_getv(parse, &argV, &need_help));
            if(need_help) break;
            ASSERT(argx->o, ERR_NULLPTR);
            for(size_t i = 0; i < vargx_length(argx->o->vec); ++i) {
                ArgX *x = vargx_get_at(&argx->o->vec, i);
                *x->val.b = false;
            }
            for(RStr flag = {0}; flag.first < argV.last; flag = rstr_splice(argV, &flag, parse->base->flag_sep)) {
                if(!flag.s) continue;
                ArgX *x = 0;
                TRYC(arg_parse_getopt(argx->o, &x, flag));
                TRYC(argx_parse(parse, x));
            }
        } break;
        case ARG_HELP: {
            parse->help.get = true;
        } break;
        case ARG_ENV: ABORT(ERR_UNREACHABLE);
        /* above */
        case ARG__COUNT:
        case ARG_NONE: break;
    }
    return 0;
error:
    if(!parse->help.get) {
        parse->help.get = true;
        parse->help.x = argx;
    }
    return -1;
}

void arg_parse_setref_group(struct ArgXGroup *group);

void arg_parse_setref_argx(struct ArgX *argx) {
    ASSERT_ARG(argx);
    switch(argx->id) {
        case ARG_FLAG:
        case ARG_BOOL: {
            if(argx->ref.b) *argx->val.b = *argx->ref.b;
        } break;
        case ARG_INT: {
            if(argx->ref.z) *argx->val.z = *argx->ref.z;
        } break;
        case ARG_FLOAT: {
            if(argx->ref.f) *argx->val.f = *argx->ref.f;
        } break;
        case ARG_ENV:
        case ARG_STRING: {
            //printff("SETTING STR %.*s=%.*s", RSTR_F(argx->info.opt), RSTR_F(*argx->ref.s));
            if(argx->ref.s) *argx->val.s = *argx->ref.s;
        } break;
        case ARG_OPTION: {
            if(argx->ref.z) *argx->val.z = *argx->ref.z;
            if(argx->o) arg_parse_setref_group(argx->o);
        } break;
        case ARG_FLAGS: {
            if(argx->o) arg_parse_setref_group(argx->o);
        } break;
        case ARG_HELP:
        case ARG_NONE:
        case ARG__COUNT: break;
    }
}

void arg_parse_setref_group(struct ArgXGroup *group) {
    ASSERT_ARG(group);
    for(size_t i = 0; i < vargx_length(group->vec); ++i) {
        ArgX *x = vargx_get_at(&group->vec, i);
        arg_parse_setref_argx(x);
    }
}

void arg_parse_setref(struct Arg *arg) {
    ASSERT_ARG(arg);
    /* first verify some things */
    /* finally assign */
    arg_parse_setref_group(&arg->opt);
    arg_parse_setref_group(&arg->env);
    //arg_parse_setref_group(&arg->pos);
}

ErrDecl arg_parse(struct Arg *arg, const unsigned int argc, const char **argv, bool *quit_early) {
    ASSERT_ARG(arg);
    ASSERT_ARG(arg->parse.base);
    ASSERT_ARG(quit_early);
    ASSERT_ARG(argv);
    arg_parse_setref(arg);
    ArgParse *parse = &arg->parse;
    parse->argv = (char **)argv;
    parse->argc = argc;
    Str temp_clean_env = {0};
    ArgX *argx = 0;
    parse->i = 1;
    int err = 0;
    /* prepare parsing */
    unsigned char pfx = arg->base.prefix;
    bool need_help = false;
    /* start parsing */
    /* check optional arguments */
    while(parse->i < parse->argc) {
        RStr argV = RSTR("");
        TRYC(arg_parse_getv(parse, &argV, &need_help));
        if(need_help) break;
        if(!rstr_length(argV)) continue;
        if(!parse->force_done_parsing && rstr_length(argV) >= 1 && rstr_get_at(&argV, 0) == pfx) {
            /* regular checking for options */
            if(rstr_length(argV) >= 2 && rstr_get_at(&argV, 1) == pfx) {
                if(rstr_length(argV) == 2) {
                    parse->force_done_parsing = true;
                    continue;
                }
                RStr arg_query = RSTR_I0(argV, 2);
                /* long option */
                TRYC(arg_parse_getopt(&arg->opt, &argx, arg_query));
                TRYC(argx_parse(parse, argx));
            } else {
                RStr arg_queries = RSTR_I0(argV, 1);
                /* short option */
                for(size_t i = 0; i < rstr_length(arg_queries); ++i) {
                    const unsigned char query = rstr_get_at(&arg_queries, i);
                    TRYC(arg_parse_getopt_short(arg, &argx, query));
                    TRYC(argx_parse(parse, argx));
                    //printff("SHORT OPTION! %.*s", RSTR_F(arg_queries));
                }
                //ArgX *argx = arg->opt_short[
            }
        } else if(parse->n_pos_parsed < vargx_length(arg->pos.vec)) {
            /* check for positional */
            arg_parse_getv_undo(parse);
            ArgX *x = vargx_get_at(&arg->pos.vec, parse->n_pos_parsed);
            TRYC(argx_parse(parse, x));
            ++parse->n_pos_parsed;
        } else if(arg->base.rest) {
            /* no argument, push rest */
            TRYG(vrstr_push_back(arg->base.rest, &argV));
        }
        /* in case of trying to get help, also search pos and then env */
        if(parse->help.get && !parse->help.x && parse->i < parse->argc) {
            arg_parse_getv(parse, &argV, &need_help);
            ArgX *x1 = targx_get(&arg->pos.lut, argV);
            ArgX *x2 = targx_get(&arg->env.lut, argV);
            ArgX *x = x1 ? x1 : x2;
            if(x) {
                arg->parse.help.x = x;
            } else {
                arg_parse_getv_undo(parse);
            }
        }
    }
    /* gather environment variables */
    for(size_t i = 0; i < vargx_length(arg->env.vec); ++i) {
        ArgX *x = vargx_get_at(&arg->env.vec, i);
        TRYG(rstr_copy(&temp_clean_env, &x->info.opt));
        const char *cenv = getenv(temp_clean_env.s);
        if(!cenv) continue;
        RStr env = RSTR_L(cenv);
        *x->val.s = env;
    }
    /* go over positional arguments */
    for(size_t i = 0; i < vargx_length(arg->pos.vec); ++i) {
    }
    /* now go over the queue and do post processing */
    vargx_sort(&parse->queue);
    for(size_t i = 0; i < vargx_length(parse->queue); ++i) {
        ArgX *x = vargx_get_at(&parse->queue, i);
        //printff("CHECK QUEUE [%.*s]", RSTR_F(x->info.opt));
        if(x && x->attr.callback.func) {
            //printff("CALLBACK!");
            TRY(x->attr.callback.func(x->attr.callback.data), "failed executing function for " F("[%.*s]", BOLD), RSTR_F(x->info.opt));
            *quit_early = x->attr.callback.quit_early;
            if(*quit_early) break;
        }
    }
    if(parse->n_pos_parsed < vargx_length(arg->pos.vec)) {
        THROW("missing %zu positional arguments", vargx_length(arg->pos.vec) - parse->n_pos_parsed);
    }
    if(parse->argc < 2 && arg->base.show_help) arg_help(arg);
clean:
    str_free(&temp_clean_env);
    vargx_free(&parse->queue);
    //DO THIS OUTSIDE:
    //if(*quit_early) {
    //    arg_free(&arg);
    //}
    return err;
error:
    arg_help(arg);
    ERR_CLEAN;
}

void arg_free(struct Arg **parg) {
    ASSERT_ARG(parg);
    Arg *arg = *parg;
    argx_group_free(&arg->opt);
    argx_group_free(&arg->env);
    argx_group_free(&arg->pos);
    str_free(&arg->print.line);
    str_free(&arg->print.buf);
    free(*parg);
}

