#include "arg.h"
#undef SSIZE_MAX
#include <sys/ioctl.h>
#include <unistd.h>

/* ENUMS, STRUCTS, VECTORS {{{ */

typedef enum {
    ARG_NONE,
    ARG_BOOL,
    ARG_FLAG,
    ARG_SSZ,
    ARG_INT,
    ARG_FLOAT,
    ARG_STRING,
    ARG_ENV,
    ARG_VECTOR,
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
        case ARG_SSZ: return "<int>";
        case ARG_FLOAT: return "<double>";
        case ARG_STRING: return "<string>";
        case ARG_ENV: return "<env>";
        case ARG_OPTION: return "<option>";
        case ARG_HELP: return "<help>";
        case ARG_FLAGS: return "<flags>";
        case ARG_VECTOR: return "<string-vec>";
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
    VrStr *rest_vec;
    RStr rest_desc;
} ArgBase;

typedef union ArgXVal {
    RStr *s;            // string
    ssize_t *z;         // number
    int *i;             // number
    double *f;          // double
    bool *b;            // bool
    void *x;            // exotic
    int e;              // enum
    VrStr *v;           // vector
} ArgXVal;

typedef struct ArgXCallback {
    ArgFunction func;
    void *data;
    bool quit_early;
    size_t priority;
} ArgXCallback;

typedef union ArgXNumber {
    ssize_t z;
    float f;
    int i;
} ArgXNumber;

typedef struct ArgX { /*{{{*/

    ArgList id;
    ArgXVal val;
    ArgXVal ref;
    int e; // enum
    RStr type;
    struct ArgXGroup *o; // option
    struct ArgXGroup *group; // required for options / parent group
    struct {
        ArgXNumber min;
        ArgXNumber max;
        ArgXCallback callback;
        bool hide_value;
    } attr;
    struct {
        const unsigned char c;
        const RStr opt;
        const RStr desc;
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
    return a->attr.callback.priority - b->attr.callback.priority;
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
    struct {
        VrStr *vec;
        RStr desc;
        ArgXGroup *pos;
    } rest;;
    RStr config;
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
    ARG_PRINT__LENGTH,
    ARG_PRINT__ENDLINE,
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

/*}}}*/

/* FUNCTION PROTOTYPES {{{ */

#define ERR_arg_config_load(...) "failed loading config"
ErrDecl arg_config_load(struct Arg *arg);

/*}}}*/

/* ~~~ creation of arguments ~~~ */

/* 0) SETTING UP {{{ */

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

void arg_init_rest(struct Arg *arg, RStr description, VrStr *rest) {
    ASSERT_ARG(arg);
    arg->base.rest_vec = rest;
    arg->base.rest_desc = description;
}

void arg_init_pipe(struct Arg *arg, VrStr *out, pthread_mutex_t *mutex) {
    ASSERT_ARG(arg);
    ASSERT_ARG(out);
    ASSERT_ARG(mutex);
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

struct ArgX *argx_init(struct ArgXGroup *group, const unsigned char c, const RStr optX, const RStr descX) {
    ASSERT_ARG(group);
    RStr opt = rstr_trim(optX);
    RStr desc = rstr_trim(descX);
    if(!rstr_length(opt)) ABORT("cannot add an empty long-opt argument");
    ArgX x = {
        .info = {c, opt, desc},
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

/*}}}*/

/* 1) ASSIGN MAIN ID {{{ */

void argx_str(ArgX *x, RStr *val, RStr *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_STRING;
    x->val.s = val;
    x->ref.s = ref;
}
void argx_ssz(ArgX *x, ssize_t *val, ssize_t *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_SSZ;
    x->val.z = val;
    x->ref.z = ref;
}
void argx_int(ArgX *x, int *val, int *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_INT;
    x->val.i = val;
    x->ref.i = ref;
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
void argx_type(struct ArgX *x, RStr type) {
    ASSERT_ARG(x);
    x->type = type;
}
void argx_none(ArgX *x) {
    ASSERT_ARG(x);
    x->id = ARG_NONE;
}
void argx_vstr(struct ArgX *x, VrStr *val, VrStr *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_VECTOR;
    x->val.v = val;
    x->ref.v = ref;
}
struct ArgXGroup *argx_opt(ArgX *x, int *val, int *ref) {
    ASSERT_ARG(x);
    ArgXGroup *options = wargx_new();
    options->desc = x->info.opt;
    options->parent = x;
    x->id = ARG_OPTION;
    x->val.i = (int *)val;
    x->ref.i = (int *)ref;
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
    x->attr.callback.priority = 1;
    x->attr.callback.quit_early = true;
}

struct ArgX *argx_pos(struct Arg *arg, RStr opt, RStr desc) {
    ASSERT_ARG(arg);
    ArgX *x = argx_init(&arg->pos, 0, opt, desc);
    return x;
}

void argx_env(struct Arg *arg, RStr opt, RStr desc, RStr *val, RStr *ref, bool hide_value) {
    ASSERT_ARG(arg);
    ASSERT_ARG(val);
    ArgX *x = argx_init(&arg->env, 0, opt, desc);
    x->id = ARG_ENV;
    x->val.s = val;
    x->ref.s = ref;
    x->attr.hide_value = hide_value;
}

/* }}} */

/* 2) ASSIGN SPECIFIC OPTIONS {{{ */

void argx_int_mm(ArgX *x, int min, int max) {
    ASSERT_ARG(x);
    if(x->id != ARG_INT) ABORT("wrong argx type in '%.*s' to set min/max: %s", RSTR_F(x->info.opt), arglist_str(x->id));
    x->attr.min.i = min;
    x->attr.max.i = max;
}

void argx_ssz_mm(ArgX *x, ssize_t min, ssize_t max) {
    ASSERT_ARG(x);
    if(x->id != ARG_SSZ) ABORT("wrong argx type in '%.*s' to set min/max: %s", RSTR_F(x->info.opt), arglist_str(x->id));
    x->attr.min.z = min;
    x->attr.max.z = max;
}

void argx_dbl_mm(ArgX *x, double min, double max) {
    ASSERT_ARG(x);
    if(x->id != ARG_FLOAT) ABORT("wrong argx type in '%.*s' to set min/max: %s", RSTR_F(x->info.opt), arglist_str(x->id));
    x->attr.min.f = min;
    x->attr.max.f = max;
}

void argx_func(struct ArgX *x, size_t priority, void *func, void *data, bool quit_early) {
    ASSERT_ARG(x);
    ASSERT_ARG(func);
    x->attr.callback.priority = priority;
    x->attr.callback.func = func;
    x->attr.callback.data = data;
    x->attr.callback.quit_early = quit_early;
}
void argx_opt_enum(struct ArgX *x, int val) {
    ASSERT_ARG(x);
    if(!(x->group && x->group->parent) || x->group->parent->id != ARG_OPTION) {
        ABORT("can only set enums to child nodes of options " F("[%.*s]", BOLD), RSTR_F(x->info.opt));
    }
    if(!x->group->parent->val.i) {
        ABORT("parent " F("[%.*s]", BOLD) " has to be assigned to an enum", RSTR_F(x->group->parent->info.opt));
    }
    x->e = val;
    //printff("SET [%.*s] ENUM TO %i", RSTR_F(x->info.opt), val);
}
void argx_hide_value(struct ArgX *x, bool hide_value) {
    ASSERT_ARG(x);
    x->attr.hide_value = hide_value;
}

/* }}}*/

/* ~~~ implementing the argument parser to actually work ~~~ */

/* PRINTING FUNCTIONS {{{ */

void arg_do_print(Arg *arg, int endline) {
    ASSERT_ARG(arg);
    int pad = arg->print.pad;
    int pad0 = arg->print.progress > pad ? 0 : pad - arg->print.progress;
    RStr content = str_rstr(arg->print.line);
    RStr fmt = RSTR("");
    bool repeat = false;
    /* first padding */
    printf("%*s", pad0, "");
    arg->print.progress += pad0;
    /* print line-wise */
    while(rstr_length(content)) {
        /* if we repeat, continue with .. */
        int len_line_printable = rstr_length_nof(content);
        /* start new line and pad correctly if we repeat */
        if(repeat) {
            printf(F("\n%*s", ""), pad, "");
            arg->print.progress = pad;
            content = rstr_triml(content);
        }
        if(len_line_printable + arg->print.progress > arg->print.bounds.max) {
            len_line_printable = arg->print.bounds.max - arg->print.progress;
        }
        int len_line_index = rstr_index_nof(content, len_line_printable);

        RStr line_print = RSTR_IE(content, len_line_index);
        size_t fE = 0;
        size_t f0 = rstr_rfind_f(line_print, &fE);
        printf("%.*s%.*s", RSTR_F(fmt), RSTR_F(line_print));
        if(f0 < rstr_length(line_print)) fmt = str_rstr(STR_IE(STR_I0(line_print, f0), fE));
        arg->print.progress += len_line_printable;

        //printf("%.*s", RSTR_F(content));
        //rstr_clear(&content);
        content.first += len_line_index;
        repeat = true;
    }
    for(int i = 0; i < endline; ++i) {
        printf("\n");
        arg->print.progress = 0;
    }
    str_clear(&arg->print.line);
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
                if(str_copy(&arg->print.line, &STR(" "))) ABORT(ERR_MALLOC);
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
                    if(str_copy(&arg->print.line, &STR(""))) ABORT(ERR_MALLOC);
                    arg_do_print(arg, true);
                    arg->print.pad = arg->print.bounds.opt + 4;
                } else {
                    TRYC(str_fmt(&arg->print.line, " "));
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
                if(str_copy(&arg->print.line, &STR(" "))) ABORT(ERR_MALLOC);
                arg_do_print(arg, false);
                arg->print.pad = arg->print.bounds.desc;
                str_clear(&arg->print.line);
            }
            TRYG(str_extend_back(&arg->print.line, arg->print.buf));
        } break;
        case ARG_PRINT__ENDLINE: {
            arg_do_print(arg, 1);
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
        case ARG_SSZ:
        case ARG_INT:
        case ARG_FLAG:
        case ARG_BOOL:
        case ARG_VECTOR:
        case ARG_FLOAT: {
            if(rstr_length(argx->type)) {
                arg_handle_print(arg, ARG_PRINT_TYPE, F("<%.*s>", ARG_TYPE_F), RSTR_F(argx->type));
            } else {
                arg_handle_print(arg, ARG_PRINT_TYPE, F("%s", ARG_TYPE_F), arglist_str(argx->id));
            }
        } break;
        case ARG_OPTION: {
            ArgXGroup *g = argx->o;
            if(vargx_length(g->vec)) {
                arg_handle_print(arg, ARG_PRINT_TYPE, F("<", ARG_OPTION_F));
                for(size_t i = 0; i < vargx_length(g->vec); ++i) {
                    if(i) arg_handle_print(arg, ARG_PRINT_TYPE, F("|", ARG_OPTION_F));
                    ArgX *x = vargx_get_at(&g->vec, i);
                    if(g && g->parent && g->parent->val.i && *g->parent->val.i == x->e) {
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
        case ARG_ENV:
        case ARG_NONE:
        case ARG__COUNT: break;
    }
    arg_handle_print(arg, ARG_PRINT_TYPE, " ");
} /*}}}*/

void argx_print_post(Arg *arg, ArgX *argx, ArgXVal *val) { /*{{{*/
    ASSERT_ARG(argx);
    ASSERT_ARG(val);
    ArgXVal out = *val;
    if(argx->attr.hide_value) {
        arg_handle_print(arg, ARG_PRINT_VALUE, "");
        arg_handle_print(arg, ARG_PRINT__ENDLINE, "");
        return;
    }
    switch(argx->id) {
        case ARG_ENV:
        case ARG_STRING: {
            if(val->s && rstr_length(*val->s)) {
                arg_handle_print(arg, ARG_PRINT_VALUE, F("=", FG_BK_B) F("%.*s", ARG_VAL_F), RSTR_F(*val->s));
            } else {
                arg_handle_print(arg, ARG_PRINT_VALUE, "");
            }
        } break;
        case ARG_SSZ: {
            ssize_t zero = 0;
            if(!val->z) out.z = &zero;
            arg_handle_print(arg, ARG_PRINT_VALUE, F("=", FG_BK_B) F("%zi", ARG_VAL_F), *out.z);
        } break;
        case ARG_INT: {
            int zero = 0;
            if(!val->i) out.i = &zero;
            arg_handle_print(arg, ARG_PRINT_VALUE, F("=", FG_BK_B) F("%i", ARG_VAL_F), *out.i);
        } break;
        case ARG_VECTOR: {
            if(val->v && vrstr_length(*val->v)) {
                arg_handle_print(arg, ARG_PRINT_VALUE, F("=[", ARG_F_SEP));
                for(size_t i = 0; val->v && i < vrstr_length(*val->v); ++i) {
                    RStr *s = vrstr_get_at(val->v, i);
                    if(i) arg_handle_print(arg, ARG_PRINT_VALUE, F(",", ARG_F_SEP));
                    arg_handle_print(arg, ARG_PRINT_VALUE, F("%.*s", ARG_VAL_F), RSTR_F(*s));
                }
                arg_handle_print(arg, ARG_PRINT_VALUE, F("]", ARG_F_SEP));
            } else {
                arg_handle_print(arg, ARG_PRINT_VALUE, "");
            }
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
        case ARG_FLAGS: {
        } break;
        case ARG_NONE: {
            arg_handle_print(arg, ARG_PRINT_VALUE, "");
        } break;
        case ARG__COUNT: break;
    }
    arg_handle_print(arg, ARG_PRINT__ENDLINE, "");
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
        arg_do_print(arg, 2);
        for(size_t i = 0; i < vargx_length(x->o->vec); ++i) {
            //arg_handle_print(arg, ARG_PRINT_NONE, "\n");
            ArgX *xx = vargx_get_at(&x->o->vec, i);
            argx_print(arg, xx, false);
        }
    }
    /* print value */
    if(detailed) {
        arg_do_print(arg, 1);
        if(!is_detailed_option) arg_do_print(arg, 1);
        arg_handle_print(arg, ARG_PRINT_SHORT, "current value");
    }
    if(detailed && x->id == ARG_OPTION && x->o) {
        int *n = x->val.i;
        for(size_t i = 0; n && i < vargx_length(x->o->vec); ++i) {
            ArgX *xx = vargx_get_at(&x->o->vec, i);
            if(xx->e != *n) continue;
            argx_print(arg, xx, false);
            break;
        }
        if(!n) {
            arg_do_print(arg, 1);
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
            arg_handle_print(arg, ARG_PRINT_NONE, F("%.*s:", BOLD UL), RSTR_F(x->group->desc));
            arg_do_print(arg, 1);
        }
    }
    argx_print(arg, x, (x == parse->help.x));
    //argx_print(base, x, false);
} /*}}}*/

void argx_group_print(Arg *arg, ArgXGroup *group) { /*{{{*/
    ASSERT_ARG(arg);
    ASSERT_ARG(group);
    if(!vargx_length(group->vec) && !(group == &arg->pos)) {
        return;
    }
    if(rstr_length(group->desc)) {
        arg_handle_print(arg, ARG_PRINT_NONE, F("%.*s:", BOLD UL), RSTR_F(group->desc));
        arg_do_print(arg, 1);
    }
    if(group == &arg->pos) {
        arg_handle_print(arg, ARG_PRINT_SHORT, F("%s", BOLD), arg->parse.argv[0]);
        for(size_t i = 0; i < vargx_length(group->vec); ++i) {
            ArgX *x = vargx_get_at(&group->vec, i);
            arg_handle_print(arg, ARG_PRINT_SHORT, " %.*s", RSTR_F(x->info.opt));
            //argx_print(arg, x, false);
        }
        if(vargx_length(arg->opt.vec)) {
            arg_handle_print(arg, ARG_PRINT_SHORT, " " F("[options]", BOLD FG_CY));
        }
        if(rstr_length(arg->base.rest_desc)) {
            arg_handle_print(arg, ARG_PRINT_SHORT, " " F("%.*s", BOLD FG_MG_B), RSTR_F(arg->base.rest_desc));
        }
        arg_do_print(arg, 1);
    }
    for(size_t i = 0; i < vargx_length(group->vec); ++i) {
        ArgX *x = vargx_get_at(&group->vec, i);
        argx_print(arg, x, false);
    }
} /*}}}*/

void arg_help_set(struct Arg *arg, struct ArgX *x) {
    ASSERT_ARG(arg);
    ASSERT_ARG(x);
    if(!arg->parse.help.get) {
        arg->parse.help.get = true;
        arg->parse.help.x = x;
    }
}

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

void arg_config(struct Arg *arg, RStr conf) {
    ASSERT_ARG(arg);
    arg->parse.config = conf;
}

/* }}} */

#if 0
bool arg_group_info_opt(struct ArgXGroup *g, void *x, RStr *found) {
    if(!g) return false;
    if(!x) return false;
    ASSERT_ARG(found);
    for(TArgXKV **kv = targx_iter_all(&g->lut, 0);
            kv;
            kv = targx_iter_all(&g->lut, kv)) {
        if((*kv)->val->
    }
}

RStr arg_info_opt(struct Arg *arg, void *x) {
    if(!x) return RSTR("");
    for(arg->parse
}
#endif

/* PARSING FUNCTIONS {{{ */

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
        if(parse->force_done_parsing) {
        } else if(!parse->help.get) {
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

bool argx_parse_is_origin_from_pos(ArgParse *parse, ArgX *argx) {
    ASSERT_ARG(parse);
    ASSERT_ARG(parse->rest.pos);
    ASSERT_ARG(argx);
    if(!argx->group) return false;
    if(argx->group == parse->rest.pos) return true;
    if(!argx->group->parent) return false;
    return argx_parse_is_origin_from_pos(parse, argx->group->parent);
}

#define ERR_argx_parse(parse, argx, ...) "failed parsing argument " F("[%.*s]", BOLD FG_WT_B) " " F("%s", ARG_TYPE_F), RSTR_F(argx->info.opt), arglist_str(argx->id)
ErrDecl argx_parse(ArgParse *parse, ArgX *argx, bool *quit_early) {
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
        if(argx->group->parent->val.i) {
            *argx->group->parent->val.i = argx->e;
        }
    }
    /* actually begin parsing */
    bool need_help = false;
    //printff("SETTING VALUE FOR %.*s", RSTR_F(argx->info.opt));
    switch(argx->id) {
        case ARG_BOOL: { //printff("GET VALUE FOR BOOL");
            if(parse->i < parse->argc) {
                TRYC(arg_parse_getv(parse, &argV, &need_help)); //printff("GOT VALUE [%.*s]", RSTR_F(argV));
                if(need_help) break;
                if(rstr_as_bool(argV, argx->val.b, true)) {
                    *argx->val.b = true;
                    arg_parse_getv_undo(parse);
                }
            } else {
                *argx->val.b = true;
            }
        } break;
        case ARG_FLAG: {
            *argx->val.b = true;
        } break;
        case ARG_SSZ: {
            TRYC(arg_parse_getv(parse, &argV, &need_help));
            if(need_help) break;
            TRYC(rstr_as_int(argV, argx->val.z));
        } break;
        case ARG_INT: {
            TRYC(arg_parse_getv(parse, &argV, &need_help));
            if(need_help) break;
            ssize_t z = 0;
            TRYC(rstr_as_int(argV, &z));
            *argx->val.i = (int)z;
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
        case ARG_VECTOR: {
            TRYC(arg_parse_getv(parse, &argV, &need_help));
            if(need_help) break;
            TRYG(vrstr_push_back(argx->val.v, &argV));
            if(argx_parse_is_origin_from_pos(parse, argx)) {
                parse->rest.vec = argx->val.v;
                parse->rest.desc = argx->info.desc;
            }
        } break;
        case ARG_OPTION: {
            TRYC(arg_parse_getv(parse, &argV, &need_help));
            if(need_help) break;
            ArgX *x = 0;
            TRYC(arg_parse_getopt(argx->o, &x, argV));
            TRYC(argx_parse(parse, x, quit_early));
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
                TRYC(argx_parse(parse, x, quit_early));
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
    /* TODO DRY */
    if(argx && argx->attr.callback.func && !argx->attr.callback.priority) {
        if(argx->attr.callback.func(argx->attr.callback.data)) {
            THROW_PRINT("failed executing function for " F("[%.*s]", BOLD) "\n", RSTR_F(argx->info.opt));
            goto error_skip_help;
        }
        *quit_early = argx->attr.callback.quit_early;
        //if(*quit_early) break;
    }
    return 0;
error:
    if(!parse->help.get) {
        parse->help.get = true;
        parse->help.x = argx;
    }
error_skip_help:
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
        case ARG_SSZ: {
            if(argx->ref.z) *argx->val.z = *argx->ref.z;
        } break;
        case ARG_INT: {
            if(argx->ref.i) *argx->val.i = *argx->ref.i;
        } break;
        case ARG_FLOAT: {
            if(argx->ref.f) *argx->val.f = *argx->ref.f;
        } break;
        case ARG_ENV:
        case ARG_STRING: {
            //printff("SETTING STR %.*s=%.*s", RSTR_F(argx->info.opt), RSTR_F(*argx->ref.s));
            if(argx->ref.s) *argx->val.s = *argx->ref.s;
        } break;
        case ARG_VECTOR: {
            if(argx->ref.v) *argx->val.v = *argx->ref.v;
        } break;
        case ARG_OPTION: {
            if(argx->ref.i) *argx->val.i = *argx->ref.i;
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
    parse->rest.vec = arg->base.rest_vec;
    parse->rest.desc = arg->base.rest_desc;
    parse->rest.pos = &arg->pos;
    Str temp_clean_env = {0};
    ArgX *argx = 0;
    parse->i = 1;
    int err = 0;
    /* prepare parsing */
    unsigned char pfx = arg->base.prefix;
    bool need_help = false;
    /* start parsing */
    int config_status = arg_config_load(arg);
    /* check optional arguments */
    while(parse->i < parse->argc) {
        RStr argV = RSTR("");
        TRYC(arg_parse_getv(parse, &argV, &need_help));
        if(need_help) break;
        if(*quit_early) goto quit_early;
        if(!rstr_length(argV)) continue;
        //printff(" [%.*s] %zu / %zu", RSTR_F(argV), parse->i, parse->argc);
        if(!parse->force_done_parsing && rstr_length(argV) >= 1 && rstr_get_at(&argV, 0) == pfx) {
            /* regular checking for options */
            if(rstr_length(argV) >= 2 && rstr_get_at(&argV, 1) == pfx) {
                ASSERT(rstr_length(argV) > 2, ERR_UNREACHABLE);
                RStr arg_query = RSTR_I0(argV, 2);
                /* long option */
                TRYC(arg_parse_getopt(&arg->opt, &argx, arg_query));
                TRYC(argx_parse(parse, argx, quit_early));
            } else {
                RStr arg_queries = RSTR_I0(argV, 1);
                /* short option */
                for(size_t i = 0; i < rstr_length(arg_queries); ++i) {
                    const unsigned char query = rstr_get_at(&arg_queries, i);
                    TRYC(arg_parse_getopt_short(arg, &argx, query));
                    TRYC(argx_parse(parse, argx, quit_early));
                    //printff("SHORT OPTION! %.*s", RSTR_F(arg_queries));
                }
                //ArgX *argx = arg->opt_short[
            }
        } else if(parse->n_pos_parsed < vargx_length(arg->pos.vec)) {
            /* check for positional */
            arg_parse_getv_undo(parse);
            ArgX *x = vargx_get_at(&arg->pos.vec, parse->n_pos_parsed);
            TRYC(argx_parse(parse, x, quit_early));
            ++parse->n_pos_parsed;
        } else if(parse->rest.vec) {
            /* no argument, push rest */
            TRYG(vrstr_push_back(parse->rest.vec, &argV));
        }
        /* in case of trying to get help, also search pos and then env */
        if(parse->help.get && !parse->help.x && parse->i < parse->argc) {
            (void)arg_parse_getv(parse, &argV, &need_help);
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
    if(config_status) {
        if(parse->help.get) {
            printf("\n");
        } else {
            goto error_skip_help;
        }
    }
#if 0
    arg_help(arg);
#else
    /* now go over the queue and do post processing */
    vargx_sort(&parse->queue);
    for(size_t i = 0; i < vargx_length(parse->queue); ++i) {
        ArgX *x = vargx_get_at(&parse->queue, i);
        if(!x->attr.callback.priority) continue;
        //printff("CHECK QUEUE [%.*s]", RSTR_F(x->info.opt));
        if(x && x->attr.callback.func) {
            if(x->attr.callback.func(x->attr.callback.data)) {
                THROW_PRINT("failed executing function for " F("[%.*s]", BOLD) "\n", RSTR_F(x->info.opt));
                goto error_skip_help;
            }
            *quit_early = x->attr.callback.quit_early;
            if(*quit_early) goto quit_early;
        }
    }
#endif
quit_early:
    if((parse->argc < 2 && arg->base.show_help)) {
        arg_help(arg);
        *quit_early = true;
    } else if(!arg->parse.help.get && parse->n_pos_parsed < vargx_length(arg->pos.vec)) {
        THROW("missing %zu positional arguments", vargx_length(arg->pos.vec) - parse->n_pos_parsed);
    }
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
error_skip_help:
    ERR_CLEAN;
}

/*}}}*/

/* CONFIG FUNCTIONS {{{ */

int arg_config_error(struct Arg *arg, RStr line, size_t line_nb, RStr opt, ArgX *argx) {
    ASSERT_ARG(arg);
    int done = 0;
    if(line.s) {
        THROW_PRINT("config error on " F("line %zu", BOLD FG_MG_B) ":\n", line_nb);
        if(!opt.s) {
            ERR_PRINTF("        %.*s:\n", RSTR_F(line));
            ERR_PRINTF("        ^");
        } else {
            RStr pre = RSTR_LL(rstr_iter_begin(line), opt.first - line.first);
            RStr at = opt;
            RStr post = RSTR_LL(rstr_iter_at(&line, opt.last - line.first), line.last - opt.last);
            ERR_PRINTF("        %.*s" F("%.*s", BOLD FG_RD_B) "%.*s\n", RSTR_F(pre), RSTR_F(at), RSTR_F(post));
            ERR_PRINTF("        %*s", (int)(opt.first - line.first), "");
            for(size_t i = 0; i < opt.last - opt.first; ++i) {
                ERR_PRINTF(F("~", BOLD FG_RD_B));
            }
        }
        ERR_PRINTF("\n");
        ++done;
    }
    if(argx) {
        arg->parse.help.get = true;
        arg->parse.help.x = argx;
        arg_help(arg);
        arg->parse.help.get = false;
        arg->parse.help.x = 0;
        ++done;
    }
    return done;
}

ErrDecl arg_config_load(struct Arg *arg) {
    ASSERT_ARG(arg);
    int err = 0;
    size_t line_nb = 0;
    RStr line = {0}, opt = {0}, conf = arg->parse.config;
    if(!rstr_length(conf)) return 0;
    ArgX *argx = 0;
    for(memset(&line, 0, sizeof(line)); line.first < conf.last; line = rstr_trim(rstr_splice(conf, &line, '\n')), ++line_nb) {
        argx = 0;
        if(!line.s) continue;
        if(!rstr_length(line)) continue;
        if(rstr_get_front(&line) == '#') continue;
        //printff("CONFIG:%.*s",RSTR_F(line));
        for(memset(&opt, 0, sizeof(opt)); opt.first < line.last; opt = rstr_trim(rstr_splice(line, &opt, '='))) {
            if(!opt.s) continue;
            //printff(" OPT:%.*s",RSTR_F(opt));
            if(!argx) {
                TRYC(arg_parse_getopt(&arg->opt, &argx, opt));
                if(argx->id == ARG_HELP) {
                    THROW("cannot configure help");
                } else if(argx->id == ARG_ENV) {
                    THROW("cannot configure env");
                } else if(argx->id == ARG_NONE) {
                    THROW("cannot configure non-value option");
                }
            } else {
                //printff("setting value for [%.*s] : %.*s", RSTR_F(argx->info.opt), RSTR_F(opt));
                switch(argx->id) {
                    case ARG_OPTION: {
                        ArgXGroup *group = argx->o;
                        ArgX *x = 0;
                        TRYC(arg_parse_getopt(group, &x, opt));
                        argx = x;
                    } break;
                    case ARG_BOOL: {
                        bool *b = argx->ref.b ? argx->ref.b : argx->val.b;
                        TRYC(rstr_as_bool(opt, b, true));
                    } break;
                    case ARG_SSZ: {
                        ssize_t *z = argx->ref.z ? argx->ref.z : argx->val.z;
                        TRYC(rstr_as_int(opt, z));
                    } break;
                    case ARG_INT: {
                        int *i = argx->ref.i ? argx->ref.i : argx->val.i;
                        ssize_t z = 0;
                        TRYC(rstr_as_int(opt, &z));
                        *i = (int)z;
                    } break;
                    case ARG_FLOAT: {
                        double *f = argx->ref.f ? argx->ref.f : argx->val.f;
                        TRYC(rstr_as_double(opt, f));
                    } break;
                    case ARG_STRING: {
                        RStr *s = argx->ref.s ? argx->ref.s : argx->val.s;
                        *s = opt;
                    } break;
                    case ARG_FLAG: {
                        bool *b = argx->ref.b ? argx->ref.b : argx->val.b;
                        *b = true;
                    } break;
                    case ARG_NONE: {
                        THROW("option cannot have a value");
                    } break;
                    case ARG_FLAGS: {
                        ASSERT(argx->o, ERR_NULLPTR);
                        for(size_t i = 0; i < vargx_length(argx->o->vec); ++i) {
                            ArgX *x = vargx_get_at(&argx->o->vec, i);
                            bool *b = x->ref.b ? x->ref.b : x->val.b;
                            *b = false;
                        }
                        for(RStr flag = {0}; flag.first < opt.last; flag = rstr_splice(opt, &flag, arg->base.flag_sep)) {
                            if(!flag.s) continue;
                            ArgX *x = 0;
                            TRYC(arg_parse_getopt(argx->o, &x, flag));
                            bool *b = x->ref.b ? x->ref.b : x->val.b;
                            *b = true;
                        }
                    } break;
                    case ARG_VECTOR: {
                        VrStr *v = argx->ref.v ? argx->ref.v : argx->val.v;
                        TRYG(vrstr_push_back(v, &opt));
                    } break;
                    case ARG_HELP:
                    case ARG_ENV: 
                    case ARG__COUNT: ABORT(ERR_UNREACHABLE);
                }
                //printff(" GROUP %p", argx->group);
                //printff(" PARENT %p", argx->group ? argx->group->parent : 0);
                //printff(" ID %s", argx->group ? argx->group->parent ? arglist_str(argx->group->parent->id) : "" : 0);
            }
            /* check enum / option; TODO DRY */
            if(argx->group && argx->group->parent && argx->group->parent->id == ARG_OPTION) {
                if(argx->group->parent->ref.i) {
                    *argx->group->parent->ref.i = argx->e;
                } else if(argx->group->parent->val.i) {
                    *argx->group->parent->val.i = argx->e;
                }
            }
        }
parse_continue: ; /* semicolon to remove warning */
    }
    arg_parse_setref(arg);
    return err;
error:
    err = -1;
    if(arg_config_error(arg, line, line_nb, opt, argx)) goto parse_continue;
    return err;
}

/*}}}*/

/* FREEING FUNCTIONS {{{ */

void argx_free(ArgX *argx) {
    ASSERT_ARG(argx);
    if(argx->id == ARG_OPTION || argx->id == ARG_FLAGS) {
        vargx_free(&argx->o->vec);
        targx_free(&argx->o->lut);
        free(argx->o);
    }
    if(argx->id == ARG_VECTOR) {
        vrstr_free(argx->val.v);
    }
    memset(argx, 0, sizeof(*argx));
};

void argx_group_free(ArgXGroup *group) {
    ASSERT_ARG(group);
    targx_free(&group->lut);
    vargx_free(&group->vec);
    memset(group, 0, sizeof(*group));
}


void arg_free(struct Arg **parg) {
    ASSERT_ARG(parg);
    Arg *arg = *parg;
    argx_group_free(&arg->opt);
    argx_group_free(&arg->env);
    argx_group_free(&arg->pos);
    str_free(&arg->print.line);
    str_free(&arg->print.buf);
    if(arg->base.rest_vec) vrstr_free(arg->base.rest_vec);
    free(*parg);
    *parg = 0;
}

/*}}}*/

