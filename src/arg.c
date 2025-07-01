#include "arg.h"
#undef SSIZE_MAX
#include <sys/ioctl.h>
#include <unistd.h>
#include "file.h"

/* ENUMS, STRUCTS, VECTORS {{{ */

typedef enum {
    ARG_NONE,
    ARG_BOOL,
    ARG_FLAG,
    ARG_SSZ,
    ARG_INT,
    ARG_FLOAT,
    ARG_STRING,
    ARG_COLOR,
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
        case ARG_NONE: return "none";
        case ARG_BOOL: return "bool";
        case ARG_FLAG: return "flag";
        case ARG_INT: return "int";
        case ARG_SSZ: return "int";
        case ARG_FLOAT: return "double";
        case ARG_STRING: return "string";
        case ARG_COLOR: return "color";
        case ARG_OPTION: return "option";
        case ARG_HELP: return "help";
        case ARG_FLAGS: return "flags";
        case ARG_VECTOR: return "string-vec";
        //case ARG_EXOTIC: return "<exotic>";
        case ARG__COUNT: return "(internal:count)";
    }
    return "(internal:invalid)";
}

typedef struct ArgBase {
    Str program;           // program name
    Str desc;              // description of program
    Str epilog;            // text below argument help
    unsigned char prefix;   // default: -
    unsigned char flag_sep; // default: ,
    bool show_help;         // display help if no arguments provided
    bool compgen_wordlist; // generate compgen wordlist
    bool source_check;     // display config errors or not
    VStr *rest_vec;
    Str rest_desc;
} ArgBase;

typedef union ArgXVal {
    StrC *s;            // string
    ssize_t *z;         // number
    int *i;             // number
    double *f;          // double
    bool *b;            // bool
    void *x;            // exotic
    int e;              // enum
    VStrC *v;           // vector
    Color *c;           // color
} ArgXVal;

typedef struct ArgXCallback {
    ArgFunction func;
    void *data;
    bool quit_early;
    ssize_t priority;
} ArgXCallback;

typedef union ArgXNumber {
    ssize_t z;
    float f;
    int i;
} ArgXNumber;

typedef struct ArgX { /*{{{*/

    ArgList id;
    size_t count; // number of times argx_parse successfully parsed
    ArgXVal val;
    ArgXVal ref;
    int e; // enum
    StrC type;
    struct ArgXTable *table; // table for below
    struct ArgXGroup *o; // options, flags
    struct ArgXGroup *group; // required for options / parent group
    struct {
        ArgXNumber min;
        ArgXNumber max;
        ArgXCallback callback;
        bool hide_value;
        bool is_env;
        bool require_tf;
    } attr;
    struct {
        const unsigned char c;
        const Str opt;
        const Str desc;
    } info;

} ArgX; /*}}}*/

void argx_free(struct ArgX *argx);
void argx_group_free_array(struct ArgXGroup **group);

#include "lut.h"
LUT_INCLUDE(TArgX, targx, Str, BY_VAL, struct ArgX, BY_REF);
LUT_IMPLEMENT(TArgX, targx, Str, BY_VAL, struct ArgX, BY_REF, str_dhash, str_cmp, 0, argx_free);
//typedef struct ArgX *TArgX;

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

typedef struct ArgXTable {
    StrC desc;
    TArgX lut;
} ArgXTable;

typedef struct ArgXGroup {
    StrC desc;
    ArgXTable *table;
    struct Arg *root; // required literally only for assigning short options
    struct ArgX *parent; // required for options
    struct ArgX **list;
    bool explicit_help;
} ArgXGroup;

VEC_INCLUDE(VArgXGroup, vargxgroup, ArgXGroup *, BY_REF, BASE);
VEC_INCLUDE(VArgXGroup, vargxgroup, ArgXGroup *, BY_REF, ERR);
VEC_INCLUDE(VArgXGroup, vargxgroup, ArgXGroup *, BY_REF, SORT);
VEC_IMPLEMENT(VArgXGroup, vargxgroup, ArgXGroup *, BY_REF, BASE, argx_group_free_array);
VEC_IMPLEMENT(VArgXGroup, vargxgroup, ArgXGroup *, BY_REF, ERR);
//VEC_IMPLEMENT(VArgXGroup, vargxgroup, ArgXGroup *, BY_REF, SORT, argx_group_free_array);

typedef struct ArgStream {
    char **argv;
    int argc;
    size_t i;
    size_t n_pos_parsed;
} ArgStream;

typedef struct ArgParse {
    bool done_compgen;
    bool force_done_parsing;
    ArgStream instream;
    VArgX queue;
    ArgBase *base;  // need the info of prefix...
    struct {
        bool get;
        bool get_explicit;
        ArgX *x;
        ArgXGroup *group;
        ArgX *helpx;
    } help;
    struct {
        VStr *vec;
        Str desc;
        ArgXGroup *pos;
    } rest;
    VStr config;
    VStr config_files_expand;
    VStr config_files_base;
} ArgParse;

typedef struct ArgPrint {
    struct {
        int max;    // max width
        int desc;   // spacing until description
        int c;      // spacing until short option
        int opt;    // spacing until long option
    } bounds;
    StrAlign p_al2;
    bool compgen_nfirst;
} ArgPrint;

typedef struct ArgFmt {
    StrFmtX group;
    StrFmtX group_delim;
    StrFmtX type;
    StrFmtX type_delim;
    StrFmtX one_of;
    StrFmtX one_of_set;
    StrFmtX one_of_delim;
    StrFmtX flag;
    StrFmtX flag_set;
    StrFmtX flag_delim;
    StrFmtX val;
    StrFmtX val_delim;
    StrFmtX c;
    StrFmtX opt;
    StrFmtX pos;
    StrFmtX env;
    StrFmtX desc;
    StrFmtX program;
} ArgFmt;

typedef struct Arg {
    ArgBase base;
    struct {
        ArgXTable pos;
        ArgXTable opt;
    } tables;
    ArgX *opt_short[256];
    ArgXGroup pos;
    VArgXGroup groups;
    ArgFmt fmt;
    ArgParse parse;
    ArgPrint print;
} Arg;

/*}}}*/

/* FUNCTION PROTOTYPES {{{ */

#define ERR_arg_config_from_str(...) "failed loading config"
ErrDecl arg_config_from_str(struct Arg *arg, StrC text);

/*}}}*/

/* ~~~ creation of arguments ~~~ */

/* 0) SETTING UP {{{ */

ATTR_NODISCARD struct Arg *arg_new(void) {
    Arg *result = 0;
    NEW(Arg, result);
    return result;
}
ATTR_NODISCARD struct ArgXGroup *wargx_new(void) {
    ArgXGroup *group = 0;
    NEW(ArgXGroup, group);
    ArgXTable *table = 0;
    NEW(ArgXTable, table);
    group->table = table;
    return group;
}

struct ArgXGroup *argx_group(struct Arg *arg, Str desc, bool explicit_help) {
    size_t len = vargxgroup_length(arg->groups);
    (void)vargxgroup_resize(&arg->groups, len + 1);
    //array_resize(arg->groups, len + 1);
    ArgXGroup **result = vargxgroup_get_at(&arg->groups, len);
    *result = malloc(sizeof(**result));
    ASSERT_ARG(*result);
    memset(*result, 0, sizeof(**result));
    (*result)->explicit_help = explicit_help;
    (*result)->table = &arg->tables.opt;
    (*result)->root = arg;
    (*result)->desc = desc;
    return *result;
}

ArgXGroup *arg_pos(Arg *arg) {
    return &arg->pos;
}

void arg_init(struct Arg *arg, Str program, Str description, Str epilog) {
    ASSERT_ARG(arg);
    arg->base.program = program;
    arg->base.desc = description;
    arg->base.epilog = epilog;
    arg->base.prefix = ARG_INIT_DEFAULT_PREFIX;
    arg->base.show_help = ARG_INIT_DEFAULT_SHOW_HELP;
    arg->base.flag_sep = ARG_INIT_DEFAULT_FLAG_SEP;
    arg->pos.desc = str("Usage");
    arg->pos.root = arg;
    arg->pos.table = &arg->tables.pos;
    arg->pos.desc = str("Usage");
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

void arg_init_rest(struct Arg *arg, Str description, VStr *rest) {
    ASSERT_ARG(arg);
    arg->base.rest_vec = rest;
    arg->base.rest_desc = description;
}

void arg_init_pipe(struct Arg *arg, VStr *out, pthread_mutex_t *mutex) {
    ASSERT_ARG(arg);
    ASSERT_ARG(out);
    ASSERT_ARG(mutex);
}

void arg_init_fmt(struct Arg *arg) {
    ASSERT_ARG(arg);
    arg->fmt.program.fg = COLOR_WHITE;
    arg->fmt.program.bold = true;
    arg->fmt.group.fg = COLOR_WHITE;
    arg->fmt.group.bold = true;
    arg->fmt.group.underline = true;
    arg->fmt.opt.fg = COLOR_WHITE;
    arg->fmt.opt.bold = true;
    arg->fmt.pos.italic = true;
    arg->fmt.val_delim.fg = COLOR_GRAY;
    arg->fmt.val.fg = COLOR_GREEN;
    arg->fmt.flag_delim.fg = COLOR_GRAY;
    arg->fmt.flag.fg = COLOR_YELLOW;
    arg->fmt.flag_set.fg = COLOR_YELLOW;
    arg->fmt.flag_set.bold = true;
    arg->fmt.flag_set.underline = true;
    arg->fmt.flag_delim.fg = COLOR_GRAY;
    arg->fmt.c.fg = COLOR_WHITE;
    arg->fmt.c.bold = true;
    arg->fmt.env.fg = COLOR_WHITE;
    arg->fmt.env.bold = true;
    arg->fmt.one_of.fg = COLOR_BLUE;
    arg->fmt.one_of_set.fg = COLOR_BLUE;
    arg->fmt.one_of_set.bold = true;
    arg->fmt.one_of_set.underline = true;
    arg->fmt.one_of_delim.fg = COLOR_GRAY;
    arg->fmt.type_delim.fg = COLOR_GRAY;
    arg->fmt.type.fg = COLOR_GREEN;
}

#define ERR_argx_group_push(...) "failed adding argument x"
ErrDecl argx_group_push(ArgXGroup *group, ArgX *in, ArgX **out) {
    ASSERT_ARG(group);
    ASSERT_ARG(in);
    //ASSERT_ARG(group->table);
    TArgXKV *xkv = targx_once(&group->table->lut, in->info.opt, in);
    if(!xkv) THROW("option '%.*s' already exists!", STR_F(in->info.opt));
    array_push(group->list, xkv->val);
    //return xkv->val;
    if(out) *out = xkv->val;
    return 0;
error:
    //ABORT("critical error in " F("[%.*s]", BOLD) " -- aborting", STR_STR_F(group->desc));
    return -1;
}

struct ArgX *argx_init(struct ArgXGroup *group, const unsigned char c, StrC optX, StrC descX) {
    ASSERT_ARG(group);
    Str opt = str_trim(optX);
    Str desc = str_trim(descX);
    if(!str_len_raw(opt)) ABORT("cannot add an empty long-opt argument");
    ArgX x = {
        .info = {c, opt, desc},
        .group = group,
        .table = group->table,
    };
    ArgX *argx = 0;
    TRYC(argx_group_push(group, &x, &argx));
    if(c) {
        if(!group->root) ABORT("cannot specify short option '%c' for '%.*s'", c, STR_F(opt));
        ArgX *xx = group->root->opt_short[c];
        if(xx) ABORT("already specified short option '%c' for '%.*s'; cannot set for '%.*s' as well.", c, STR_F(xx->info.opt), STR_F(opt));
        group->root->opt_short[c] = argx;
    }
    return argx;
error:
    ABORT("critical error in " F("[%.*s]", BOLD) " -- aborting", STR_F(group->desc));
    return 0;
}

/*}}}*/

/* 1) ASSIGN MAIN ID {{{ */

void argx_str(ArgX *x, Str *val, Str *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_STRING;
    x->val.s = val;
    x->ref.s = ref;
}
void argx_col(ArgX *x, Color *val, Color *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    x->id = ARG_COLOR;
    x->val.c = val;
    x->ref.c = ref;
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
void argx_bool_require_tf(ArgX *x, bool require_tf) {
    ASSERT_ARG(x);
    ASSERT_ARG(x->id == ARG_BOOL);
    x->attr.require_tf = true;
}
void argx_flag_set(ArgX *x, bool *val, bool *ref) {
    ASSERT_ARG(x);
    ASSERT_ARG(val);
    ASSERT(x->group && x->group->parent && x->group->parent->id == ARG_FLAGS, "expect flags");
    x->id = ARG_FLAG;
    x->val.b = val;
    x->ref.b = ref;
}
void argx_type(struct ArgX *x, StrC type) {
    ASSERT_ARG(x);
    x->type = type;
}
void argx_none(ArgX *x) {
    ASSERT_ARG(x);
    x->id = ARG_NONE;
}
void argx_vstr(struct ArgX *x, VStr *val, VStr *ref) {
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
    x->attr.callback.quit_early = true;
    x->attr.callback.priority = -(ssize_t)1;
    arg->parse.help.helpx = x;
}

struct ArgX *argx_pos(struct Arg *arg, Str opt, Str desc) {
    ASSERT_ARG(arg);
    ArgX *x = argx_init(&arg->pos, 0, opt, desc);
    return x;
}

struct ArgX *argx_env(struct ArgXGroup *group, StrC opt, StrC desc, bool hide_value) {
    ASSERT_ARG(group);
    ArgX *x = argx_init(group, 0, opt, desc);
    x->attr.is_env = true;
    x->attr.hide_value = hide_value;
    return x;
}

/* }}} */

/* 2) ASSIGN SPECIFIC OPTIONS {{{ */

void argx_int_mm(ArgX *x, int min, int max) {
    ASSERT_ARG(x);
    if(x->id != ARG_INT) ABORT("wrong argx type in '%.*s' to set min/max: %s", STR_F(x->info.opt), arglist_str(x->id));
    x->attr.min.i = min;
    x->attr.max.i = max;
}

void argx_ssz_mm(ArgX *x, ssize_t min, ssize_t max) {
    ASSERT_ARG(x);
    if(x->id != ARG_SSZ) ABORT("wrong argx type in '%.*s' to set min/max: %s", STR_F(x->info.opt), arglist_str(x->id));
    x->attr.min.z = min;
    x->attr.max.z = max;
}

void argx_dbl_mm(ArgX *x, double min, double max) {
    ASSERT_ARG(x);
    if(x->id != ARG_FLOAT) ABORT("wrong argx type in '%.*s' to set min/max: %s", STR_F(x->info.opt), arglist_str(x->id));
    x->attr.min.f = min;
    x->attr.max.f = max;
}

void argx_func(struct ArgX *x, ssize_t priority, void *func, void *data, bool quit_early) {
    ASSERT_ARG(x);
    ASSERT_ARG(func);
    x->attr.callback.priority = priority;
    x->attr.callback.func = func;
    x->attr.callback.data = data;
    x->attr.callback.quit_early = quit_early;
}
void argx_opt_enum(struct ArgX *x, int val) {
    ASSERT_ARG(x);
    /* TODO: order of this thing below (the || ... ) ? */
    if(!(x->group && x->group->parent) || x->group->parent->id != ARG_OPTION) {
        ABORT("can only set enums to child nodes of options " F("[%.*s]", BOLD), STR_F(x->info.opt));
    }
    if(!x->group->parent->val.i) {
        ABORT("parent " F("[%.*s]", BOLD) " has to be assigned to an enum", STR_F(x->group->parent->info.opt));
    }
    x->e = val;
    //printff("SET [%.*s] ENUM TO %i", STR_F(x->info.opt), val);
}
void argx_hide_value(struct ArgX *x, bool hide_value) {
    ASSERT_ARG(x);
    x->attr.hide_value = hide_value;
}

/* }}}*/

/* ~~~ implementing the argument parser to actually work ~~~ */

/* PRINTING FUNCTIONS {{{ */

void argx_fmt_type(Str *out, Arg *arg, ArgX *argx) { /*{{{*/
    switch(argx->id) {
        case ARG_COLOR:
        case ARG_STRING:
        case ARG_SSZ:
        case ARG_INT:
        case ARG_FLAG:
        case ARG_BOOL:
        case ARG_VECTOR:
        case ARG_FLOAT: {
            str_fmtx(out, arg->fmt.type_delim, "<");
            str_fmtx(out, arg->fmt.type, "%.*s", STR_F(argx->type.len ? argx->type : str_l(arglist_str(argx->id))));
            str_fmtx(out, arg->fmt.type_delim, ">");
        } break;
        case ARG_OPTION: {
            ArgXGroup *g = argx->o;
            if(array_len(g->list)) {
                str_fmtx(out, arg->fmt.one_of_delim, "<");
                for(size_t i = 0; i < array_len(g->list); ++i) {
                    if(i) str_fmtx(out, arg->fmt.one_of_delim, "|");
                    ArgX *x = array_at(g->list, i);
                    if(g && g->parent && g->parent->val.i && *g->parent->val.i == x->e) {
                        str_fmtx(out, arg->fmt.one_of_set, "%.*s", STR_F(x->info.opt));
                    } else {
                        str_fmtx(out, arg->fmt.one_of, "%.*s", STR_F(x->info.opt));
                    }
                }
                str_fmtx(out, arg->fmt.one_of_delim, ">");
            }
        } break;
        case ARG_FLAGS: {
            ArgXGroup *g = argx->o;
            if(array_len(g->list)) {
                str_fmtx(out, arg->fmt.flag_delim, "<");
                for(size_t i = 0; i < array_len(g->list); ++i) {
                    if(i) str_fmtx(out, arg->fmt.flag_delim, "|");
                    ArgX *x = array_at(g->list, i);
                    ASSERT_ARG(x->group);
                    ASSERT_ARG(x->group->parent);
                    ASSERT(x->id == ARG_FLAG, "the option [%.*s] in [--%.*s] should be set as a %s", STR_F(x->info.opt), STR_F(x->group->parent->info.opt), arglist_str(ARG_FLAG));
                    if(*x->val.b) {
                        str_fmtx(out, arg->fmt.flag_set, "%.*s", STR_F(x->info.opt));
                    } else {
                        str_fmtx(out, arg->fmt.flag, "%.*s", STR_F(x->info.opt));
                    }
                }
                str_fmtx(out, arg->fmt.flag_delim, ">");
            }
        } break;
        case ARG_HELP: {
            str_fmtx(out, arg->fmt.type_delim, "<");
            str_fmtx(out, arg->fmt.type, "arg");
            str_fmtx(out, arg->fmt.type_delim, ">");
        } break;
        case ARG_NONE:
        case ARG__COUNT: break;
    }
    //////str_push(out, ' ');
    //////TODO maybe set??? (above)
} /*}}}*/

void arg_help_set(struct Arg *arg, struct ArgX *x) {
    ASSERT_ARG(arg);
    ASSERT_ARG(x);
    if(!arg->parse.help.get) {
        arg->parse.help.get = true;
        arg->parse.help.x = x;
    }
}

bool argx_fmt_val(Str *out, Arg *arg, ArgX *x, ArgXVal val, StrC prefix) {
    ASSERT_ARG(out);
    ASSERT_ARG(arg);
    ASSERT_ARG(x);
    if(x->attr.hide_value) return false;
    switch(x->id) {
        case ARG_NONE: break;
        case ARG_OPTION: {} break;
        case ARG_FLAGS: {} break;
        case ARG_BOOL: {
            if(!val.b) break;
            str_fmtx(out, arg->fmt.val_delim, "%.*s", STR_F(prefix));
            str_fmtx(out, arg->fmt.val, "%s", *val.b ? "true" : "false");
        } break;
        case ARG_COLOR: {
            if(!val.c) break;
            str_fmtx(out, arg->fmt.val_delim, "%.*s", STR_F(prefix));
            color_fmt_rgb(out, *val.c);
        } break;
        case ARG_FLOAT: {
            if(!val.f) break;
            str_fmtx(out, arg->fmt.val_delim, "%.*s", STR_F(prefix));
            str_fmtx(out, arg->fmt.val, "%f", *val.f);
        } break;
        case ARG_HELP: {
        } break;
        case ARG_INT: {
            if(!val.i) break;
            str_fmtx(out, arg->fmt.val_delim, "%.*s", STR_F(prefix));
            str_fmtx(out, arg->fmt.val, "%i", *val.i);
        } break;
        case ARG_FLAG: {
            if(!val.b) break;
            str_fmtx(out, arg->fmt.val_delim, "%.*s", STR_F(prefix));
            str_fmtx(out, arg->fmt.val, "%s", *val.b ? "true" : "false");
        } break;
        case ARG_SSZ: {
            if(!val.z) break;
            str_fmtx(out, arg->fmt.val_delim, "%.*s", STR_F(prefix));
            str_fmtx(out, arg->fmt.val, "%zu", *val.z);
        } break;
        case ARG_STRING: {
            if(!val.s) break;
            if(!val.s->len) break;
            str_fmtx(out, arg->fmt.val_delim, "%.*s", STR_F(prefix));
            str_fmtx(out, arg->fmt.val, "%.*s", STR_F(*val.s));
        } break;
        case ARG_VECTOR: {
            if(!val.v) break;
            str_fmtx(out, arg->fmt.val_delim, "%.*s", STR_F(prefix));
            str_fmtx(out, arg->fmt.val_delim, "[");
            for(size_t i = 0; i < array_len(*val.v); ++i) {
                if(x) str_fmtx(out, arg->fmt.val_delim, ",");
                str_push(out, '\n');
                Str s = array_at(*val.v, i);
                str_fmtx(out, arg->fmt.val, "%.*s", STR_F(s));
            }
            str_fmtx(out, arg->fmt.val_delim, "]");
        } break;
        case ARG__COUNT:
        default: THROW("UKNOWN FMT, ID:%u", x->id);
    }
    return true;
error:
    return false;
}

void argx_fmt(Str *out, Arg *arg, ArgX *x, bool detailed) {
    ASSERT_ARG(out);
    ASSERT_ARG(arg);
    ASSERT_ARG(x);
    Str tmp = {0};
    bool no_type = false;
    if(x->group == &arg->pos) {
        /* format POSITIONAL values: full option */
        str_clear(&tmp);
        str_fmtx(&tmp, arg->fmt.pos, "%.*s", STR_F(x->info.opt));
        str_fmt_al(out, &arg->print.p_al2, arg->print.bounds.opt, arg->print.bounds.opt + 2, arg->print.bounds.max, "%.*s", STR_F(tmp));
        //no_type = true;
    } else if(x->group->table == &arg->tables.opt && !x->attr.is_env) {
        /* format OPTIONAL value: short option + full option */
        if(x->info.c) {
            str_clear(&tmp);
            str_fmtx(&tmp, arg->fmt.c, "%c%c", arg->base.prefix, x->info.c);
            str_fmt_al(out, &arg->print.p_al2, arg->print.bounds.c, arg->print.bounds.c + 2, arg->print.bounds.max, "%.*s", STR_F(tmp));
        }
        str_clear(&tmp);
        str_fmtx(&tmp, arg->fmt.opt, "%c%c%.*s", arg->base.prefix, arg->base.prefix, STR_F(x->info.opt));
        str_fmt_al(out, &arg->print.p_al2, arg->print.bounds.opt, arg->print.bounds.opt + 2, arg->print.bounds.max, "%.*s", STR_F(tmp));
    } else {
        ////desc = false;
        //size_t i0 = x->group && x->group.parent ? 
        str_clear(&tmp);
        //StrFmtX fmt = x->attr.is_env ? arg->fmt.env : arg->fmt.pos;
        StrFmtX fmt = x->attr.is_env ? arg->fmt.env : arg->fmt.one_of;
        size_t i0 = x->attr.is_env ? arg->print.bounds.c : arg->print.bounds.opt + 2;
        str_fmtx(&tmp, fmt, "%.*s", STR_F(x->info.opt));
        str_fmt_al(out, &arg->print.p_al2, i0, arg->print.bounds.opt + 2, arg->print.bounds.max, "%.*s", STR_F(tmp));
    }
    if(!no_type) {
        str_clear(&tmp);
        argx_fmt_type(&tmp, arg, x);
        str_fmt_al(out, &arg->print.p_al2, arg->print.p_al2.i0_prev, arg->print.bounds.opt + 2, arg->print.bounds.max, " %.*s", STR_F(tmp));
    }
    if(!no_type) {
        str_clear(&tmp);
        str_fmtx(&tmp, arg->fmt.desc, "%.*s", STR_F(x->info.desc));
        str_fmt_al(out, &arg->print.p_al2, arg->print.bounds.desc, arg->print.bounds.opt + 4, arg->print.bounds.max, "%.*s", STR_F(tmp));

        str_clear(&tmp);
        if(argx_fmt_val(&tmp, arg, x, x->val, str("="))) {
            str_fmt_al(out, &arg->print.p_al2, arg->print.p_al2.progress, arg->print.bounds.opt + 4, arg->print.bounds.max, " ");
        }
        str_fmt_al(out, &arg->print.p_al2, arg->print.bounds.desc, arg->print.bounds.opt + 4, arg->print.bounds.max, "%.*s", STR_F(tmp));
    }
    str_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
    if(detailed) {
        if(x->id == ARG_OPTION && x->o) {
            for(size_t i = 0; i < array_len(x->o->list); ++i) {
                ArgX *argx = array_at(x->o->list, i);
                argx_fmt(out, arg, argx, false);
            }
        } else if(x->id == ARG_FLAGS) {
            //str_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
            //for(size_t i = 0; i < vargx_length(x->o->vec); ++i) {
            //    ArgX *argx = vargx_get_at(&x->o->vec, i);
            //    str_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
            //    argx_fmt(out, arg, argx, false);
            //}
        }
        str_clear(&tmp);
        if(argx_fmt_val(&tmp, arg, x, x->val, str("current value: "))) {
            str_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
            str_fmt_al(out, &arg->print.p_al2, 0, arg->print.bounds.opt + 2, arg->print.bounds.max, "%.*s", STR_F(tmp));
        }
        str_clear(&tmp);
        if(argx_fmt_val(&tmp, arg, x, x->ref, str("default value: "))) {
            str_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
            str_fmt_al(out, &arg->print.p_al2, 0, arg->print.bounds.opt + 2, arg->print.bounds.max, "%.*s", STR_F(tmp));
        }
        str_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
        /* done */
    }
    str_free(&tmp);
}

void argx_fmt_group(Str *out, Arg *arg, ArgXGroup *group) {
    ASSERT_ARG(out);
    ASSERT_ARG(arg);
    ASSERT_ARG(group);
    /* empty groups we don't care about */
    Str tmp = {0};
    if(!array_len(group->list) && !(group == &arg->pos)) {
        return;
    }
    /* group title */
    if(group->desc.len) {
        str_clear(&tmp);
        str_fmtx(&tmp, arg->fmt.group, "%.*s:", STR_F(group->desc));
        str_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "%.*s", STR_F(tmp));
    }
    if(arg->parse.help.get && arg->parse.help.group) {
        if(arg->parse.help.group != group) {
            str_clear(&tmp);
            str_fmtx(&tmp, arg->fmt.group_delim, "<collapsed>", STR_F(group->desc));
            str_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, " %.*s\n", STR_F(tmp));
            return;
        }
    } else if(group->explicit_help) {
        str_clear(&tmp);
        str_fmtx(&tmp, arg->fmt.group_delim, "--help '%.*s'", STR_F(group->desc), STR_F(group->desc));
        str_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, " %.*s\n", STR_F(tmp));
        return;
    }
    /* usage / group title */
    if(group != &arg->pos) {
        str_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
    } else {
        str_clear(&tmp);
        str_fmtx(&tmp, arg->fmt.program, "%s ", arg->parse.instream.argv[0]);
        str_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "%.*s", STR_F(tmp));
    }
    /* each thing */
    for(size_t i = 0; i < array_len(group->list); ++i) {
        ArgX *x = array_at(group->list, i);
        str_clear(&tmp);
        argx_fmt(out, arg, x, false);
    }
    str_free(&tmp);
}

void argx_fmt_specific(Str *out, Arg *arg, ArgParse *parse, ArgX *x) { /*{{{*/
    Str tmp = {0};
    if(x->group) {
        if(x->group->parent) {
            argx_fmt_specific(out, arg, parse, x->group->parent);
        } else {
            str_fmtx(&tmp, arg->fmt.group, "%.*s:", STR_F(x->group->desc));
            str_fmt_al(out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "%.*s\n", STR_F(tmp));
        }
    }
    argx_fmt(out, arg, x, (x == parse->help.x));
    str_free(&tmp);
    //argx_print(base, x, false);
} /*}}}*/

void argx_print_opt(bool *re, const char *fmt, ...) {
    va_list args;
    if(*re) printf("%c", 0);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    *re = true;
}

void argx_compgen(struct Arg *arg, struct ArgX *x) {
    ASSERT_ARG(arg);
    if(!x) return;
    switch(x->id) {
        case ARG_OPTION:
        case ARG_FLAGS: {
            if(!x->o) break;
            for(size_t i = 0; i < array_len(x->o->list); ++i) {
                ArgX *argx = array_at(x->o->list, i);
                argx_print_opt(&arg->print.compgen_nfirst, "%.*s", STR_F(argx->info.opt));
            }
        } break;
        case ARG_BOOL: {
            printf("true%cfalse", 0);
        } break;
        case ARG_HELP: {
            for(size_t i = 0; i < vargxgroup_length(arg->groups); ++i) {
                ArgXGroup *group = *vargxgroup_get_at(&arg->groups, i);
                argx_print_opt(&arg->print.compgen_nfirst, "%.*s", STR_F(group->desc));
            }
        } break;
        default: break;
    }
}

void arg_compgen(struct Arg *arg) {
    ASSERT_ARG(arg);
    if(arg->parse.done_compgen) return;
    arg->parse.done_compgen = true;
    if(arg->parse.help.group) return;
    arg->parse.done_compgen = true;
    /* optional help */
    TArgXKV **kv = 0;
    ArgXTable *opt_table = &arg->tables.opt;
    if(arg->parse.help.get) {
        ArgX *x = arg->parse.help.x;
        if(x) {
            opt_table = x->o ? x->o->table : 0;
            if(!opt_table && x->id != ARG_HELP) argx_compgen(arg, x);
        }
        if(arg->parse.help.get_explicit) {
            if(!x || (x && x->id != ARG_HELP)) {
                argx_compgen(arg, arg->parse.help.helpx);
            }
        }
    }
    if(opt_table) {
        while((kv = targx_iter_all(&opt_table->lut, kv))) {
            ArgX *x = (*kv)->val;
            if(x->attr.is_env) continue;
            if(x->group && x->group->parent) {
                argx_print_opt(&arg->print.compgen_nfirst, "%.*s", STR_F(x->info.opt));
            } else {
                int len = arg->parse.instream.argc;
                if(len > 1 && *arg->parse.instream.argv[len - 1] == '-') {
                    argx_print_opt(&arg->print.compgen_nfirst, "%c%c%.*s", arg->base.prefix, arg->base.prefix, STR_F(x->info.opt));
                }
            }
        }
    }
    if(!arg->parse.help.x) {
        /* positional help */
        size_t i = arg->parse.instream.n_pos_parsed;
        if(i < array_len(arg->pos.list)) {
            ArgX *x = array_at(arg->pos.list, i);
            //printff("X = %.*s", STR_F(x->info.opt));
            argx_compgen(arg, x);
        }
    }
    printf("\n");
}

int arg_help(struct Arg *arg) { /*{{{*/
    if(arg->base.compgen_wordlist) {
        arg_compgen(arg);
        return 0;
    }
    ASSERT_ARG(arg);
    Str out = {0};
    Str tmp = {0};
    /* if term width < min, adjust */
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width_restore = arg->print.bounds.max;
    int width = w.ws_col < arg->print.bounds.max ? w.ws_col : arg->print.bounds.max;
    arg->print.bounds.max = width;
    /* now print */
    if(arg->parse.help.x && arg->parse.help.get) {
        /* specific help */
        argx_fmt_specific(&out, arg, &arg->parse, arg->parse.help.x);
    } else {
        /* default help */
        str_clear(&tmp);
        str_fmtx(&tmp, arg->fmt.program, "%.*s:", STR_F(arg->base.program));
        str_fmtx(&tmp, (StrFmtX){0}, " %.*s\n", STR_F(arg->base.desc));
        str_fmt_al(&out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "%.*s", STR_F(tmp));

        str_clear(&tmp);
        str_fmtx(&tmp, arg->fmt.group, "%.*s:", STR_F(arg->pos.desc));
        str_fmt_al(&out, &arg->print.p_al2, 0, arg->print.bounds.c, arg->print.bounds.max, "%.*s\n", STR_F(tmp));
        str_clear(&tmp);
        str_fmtx(&tmp, arg->fmt.program, "%.*s", STR_F(arg->base.program));
        str_fmt_al(&out, &arg->print.p_al2, arg->print.bounds.c, arg->print.bounds.c + 2, arg->print.bounds.max, "%.*s", STR_F(tmp));

        for(size_t i = 0; i < array_len(arg->pos.list); ++i) {
            ArgX *argx = array_at(arg->pos.list, i);
            str_clear(&tmp);
            str_fmtx(&tmp, arg->fmt.pos, "%.*s", STR_F(argx->info.opt));
            str_fmt_al(&out, &arg->print.p_al2, 0, arg->print.bounds.c, arg->print.bounds.max, " %.*s", STR_F(tmp));
        }
        /*  */
        str_fmt_al(&out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "\n");
        for(size_t i = 0; i < array_len(arg->pos.list); ++i) {
            ArgX *argx = array_at(arg->pos.list, i);
            argx_fmt(&out, arg, argx, false);
        }
        /* all other stuff */
        for(size_t i = 0; i < vargxgroup_length(arg->groups); ++i) {
            ArgXGroup **group = vargxgroup_get_at(&arg->groups, i);
            argx_fmt_group(&out, arg, *group);
        }
        if(str_len_raw(arg->base.epilog)) {
            //arg_handle_print(arg, ARG_PRINT_NONE, "%.*s", STR_F(arg->base.epilog));
            //printf("\n");
            str_fmt_al(&out, &arg->print.p_al2, 0, 0, arg->print.bounds.max, "%.*s\n", STR_F(arg->base.epilog));
        }
    }
    str_free(&tmp);
    str_print(out);
    str_free(&out);
    arg->print.bounds.max = width_restore;
    return 0;
} /*}}}*/

void arg_config(struct Arg *arg, StrC conf) {
    ASSERT_ARG(arg);
    array_push(arg->parse.config, conf);
}

int arg_config_from_file(struct Arg *arg, Str filename) {
    ASSERT_ARG(arg);
    Str expanded = {0};
    str_fmt_expath(&expanded, filename, true);
    if(!expanded.len) return 0;
    for(size_t i = 0; i < array_len(arg->parse.config_files_expand); ++i) {
        if(!str_cmp(array_at(arg->parse.config_files_expand, i), expanded)) {
            str_free(&expanded);
            return 0;
        }
    }
    array_push(arg->parse.config_files_expand, expanded);
    Str text = {0};
    if(file_str_read(expanded, &text)) goto error;
    array_push(arg->parse.config, text);
    if(!text.len) {
        str_free(&text);
        return 0;
    }
    TRYC(arg_config_from_str(arg, text));
    return 0;
error:
    ERR_PRINTF("failed reading file: '%.*s'", STR_F(expanded));
    return -1;
}

/* }}} */

#if 0
bool arg_group_info_opt(struct ArgXTable *g, void *x, RStr *found) {
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

#define ERR_arg_parse_getopt(table, ...) "failed getting option for " F("[%.*s]", BOLD), STR_F((table)->desc)
ErrDecl arg_parse_getopt(ArgXTable *table, ArgX **x, StrC opt) {
    ASSERT_ARG(table);
    ASSERT_ARG(x);
    *x = targx_get(&table->lut, opt);
    if(!*x) THROW("value " F("%.*s", FG_BL_B) " is not a valid option", STR_F(opt));
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
ErrDecl arg_parse_getv(ArgParse *parse, ArgStream *stream, Str *argV, bool *need_help) {
    ASSERT_ARG(parse);
    ASSERT_ARG(parse->instream.argv);
    ASSERT_ARG(argV);
    /* parse->compgen? */
    unsigned int pfx = parse->base->prefix;
    StrC result;
repeat:
    if(stream->i < stream->argc) {
        char *argv = stream->argv[stream->i++];
        result = str_l(argv);
        if(!parse->force_done_parsing && str_len_raw(result) == 2 && str_at(result, 0) == pfx && str_at(result, 1) == pfx) {
            parse->force_done_parsing = true;
            goto repeat;
        }
        *argV = result;
        //printff("GOT ARGUMENT %.*s", STR_F(*argV));
    } else {
        if(parse->force_done_parsing) {
        } else if(parse->base->compgen_wordlist) {
            *need_help = true;
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

void arg_parse_getv_undo(ArgParse *parse, ArgStream *stream) {
    ASSERT_ARG(parse);
    ASSERT_ARG(stream);
    ASSERT_ARG(stream->argv);
    ASSERT(stream->i, "nothing left to undo");
    --stream->i;
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

#define ERR_argx_parse(parse, stream, argx, ...) "failed parsing argument " F("[%.*s]", BOLD FG_WT_B) " " F("%s", ARG_TYPE_F), STR_F(argx->info.opt), arglist_str(argx->id)
ErrDecl argx_parse(ArgParse *parse, ArgStream *stream, ArgX *argx, bool *quit_early) {
    ASSERT_ARG(parse);
    ASSERT_ARG(argx);
    //printff("PARSE [%.*s]", STR_F(argx->info.opt));
    /* add to queue for post processing */
    TRYG(vargx_push_back(&parse->queue, argx));
    StrC argV = str("");
    /* check if we want to get help for this */
    if(parse->help.get && !argx->attr.is_env) {
        if(!parse->help.x || (argx->group ? parse->help.x == argx->group->parent : false)) {
            parse->help.x = argx;
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
    //printff("SETTING VALUE FOR %.*s", STR_F(argx->info.opt));
    switch(argx->id) {
        case ARG_BOOL: { //printff("GET VALUE FOR BOOL");
            if(stream->i < stream->argc) {
                TRYC(arg_parse_getv(parse, stream, &argV, &need_help)); //printff("GOT VALUE [%.*s]", STR_F(argV));
                if(need_help) break;
                if(str_as_bool(argV, argx->val.b)) {
                    if(argx->attr.require_tf) {
                        THROW("failed parsing bool");
                    } else {
                        *argx->val.b = true;
                        arg_parse_getv_undo(parse, stream);
                    }
                }
            } else if(argx->attr.require_tf) {
                THROW("failed parsing bool");
            } else {
                *argx->val.b = true;
            }
            if(need_help) break;
            ++argx->count;
        } break;
        case ARG_FLAG: {
            *argx->val.b = true;
            ++argx->count;
        } break;
        case ARG_SSZ: {
            TRYC(arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            TRYC(str_as_ssize(argV, argx->val.z, 0));
            ++argx->count;
        } break;
        case ARG_INT: {
            TRYC(arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            ssize_t z = 0;
            TRYC(str_as_ssize(argV, &z, 0));
            *argx->val.i = (int)z;
            ++argx->count;
        } break;
        case ARG_FLOAT: {
            TRYC(arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            TRYC(str_as_double(argV, argx->val.f));
            ++argx->count;
        } break;
        case ARG_COLOR: {
            TRYC(arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            TRYC(str_as_color(argV, argx->val.c));
            ++argx->count;
        } break;
        case ARG_STRING: {
            TRYC(arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            *argx->val.s = argV;
            ++argx->count;
        } break;
        case ARG_VECTOR: {
            TRYC(arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            array_push(*argx->val.v, argV);
            ++argx->count;
            if(argx_parse_is_origin_from_pos(parse, argx)) {
                parse->rest.vec = argx->val.v;
                parse->rest.desc = argx->info.desc;
            }
        } break;
        case ARG_OPTION: {
            TRYC(arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            ArgX *x = 0;
            TRYC(arg_parse_getopt(argx->o->table, &x, argV));
            TRYC(argx_parse(parse, stream, x, quit_early));
            ++argx->count;
        } break;
        case ARG_FLAGS: {
            TRYC(arg_parse_getv(parse, stream, &argV, &need_help));
            if(need_help) break;
            ASSERT(argx->o, ERR_NULLPTR);
            if(!argx->count) {
                for(size_t i = 0; i < array_len(argx->o->list); ++i) {
                    ArgX *x = array_at(argx->o->list, i);
                    *x->val.b = false;
                }
            }
            for(Str flag = {0}; str_splice(argV, &flag, parse->base->flag_sep); ) {
                if(!flag.str) continue;
                ArgX *x = 0;
                TRYC(arg_parse_getopt(argx->o->table, &x, flag));
                TRYC(argx_parse(parse, stream, x, quit_early));
                ++argx->count;
            }
        } break;
        case ARG_HELP: {
            parse->help.get_explicit = true;
            parse->help.get = true;
        } break;
        /* above */
        case ARG__COUNT:
        case ARG_NONE: break;
    }
    /* TODO DRY */
    if(argx && argx->attr.callback.func && !argx->attr.callback.priority) {
        if(argx->attr.callback.func(argx->attr.callback.data)) {
            THROW_PRINT("failed executing function for " F("[%.*s]", BOLD) "\n", STR_F(argx->info.opt));
            goto error_skip_help;
        }
        *quit_early = argx->attr.callback.quit_early;
        //if(*quit_early) break;
    }
    if(!(parse->base->compgen_wordlist && need_help)) {
        return 0;
    }
error:
    if(!parse->help.get) {
        //printff("HELP GET X: %.*s", STR_F(argx->info.opt));
        parse->help.get = true;
        parse->help.x = argx;
    }
    if(parse->base->compgen_wordlist) {
        return 0;
    }
error_skip_help:
    return -1;
}

void arg_parse_setref_table(struct ArgXTable *table);

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
        case ARG_COLOR: {
            if(argx->ref.c) *argx->val.c = *argx->ref.c;
        } break;
        case ARG_STRING: {
            //printff("SETTING STR %.*s=%.*s", STR_F(argx->info.opt), STR_F(*argx->ref.s));
            if(argx->ref.s) *argx->val.s = *argx->ref.s;
        } break;
        case ARG_VECTOR: {
            if(argx->ref.v) *argx->val.v = *argx->ref.v;
        } break;
        case ARG_OPTION: {
            if(argx->ref.i) *argx->val.i = *argx->ref.i;
            if(argx->o) arg_parse_setref_table(argx->o->table);
        } break;
        case ARG_FLAGS: {
            if(argx->o) arg_parse_setref_table(argx->o->table);
        } break;
        case ARG_HELP:
        case ARG_NONE:
        case ARG__COUNT: break;
    }
}

void arg_parse_setref_table(struct ArgXTable *table) {
    ASSERT_ARG(table);
    TArgXKV **kv = 0;
    while((kv = targx_iter_all(&table->lut, kv))) {
        ArgX *x = (*kv)->val;
        arg_parse_setref_argx(x);
    }
}

void arg_parse_setref(struct Arg *arg) {
    ASSERT_ARG(arg);
    /* first verify some things */
    /* finally assign */
    arg_parse_setref_table(&arg->tables.opt);
    arg_parse_setref_table(&arg->tables.pos);
    //arg_parse_setref_group(&arg->pos);
}

ErrDecl arg_parse(struct Arg *arg, const unsigned int argc, const char **argv, bool *quit_early) {
    ASSERT_ARG(arg);
    ASSERT_ARG(arg->parse.base);
    ASSERT_ARG(quit_early);
    ASSERT_ARG(argv);
    arg_parse_setref(arg);
    ArgParse *parse = &arg->parse;
    parse->instream.argv = (char **)argv;
    parse->instream.argc = argc;
    parse->rest.vec = arg->base.rest_vec;
    parse->rest.desc = arg->base.rest_desc;
    parse->rest.pos = &arg->pos;
    Str temp_clean_env = {0};
    ArgX *argx = 0;
    parse->instream.i = 1;
    int err = 0;
    /* prepare parsing */
    unsigned char pfx = arg->base.prefix;
    bool need_help = false;
    /* start parsing */
    int config_status = 0;
    for(size_t i = 0; i < array_len(arg->parse.config_files_base); ++i) {
        config_status |= arg_config_from_file(arg, array_at(arg->parse.config_files_base, i));
    }
    /* gather environment variables */
    TArgXKV **kv = 0;
    while((kv = targx_iter_all(&arg->tables.opt.lut, kv))) {
        ArgX *x = (*kv)->val;
        if(!x->attr.is_env) continue;
        str_copy(&temp_clean_env, x->info.opt);
        char *cenv = getenv(temp_clean_env.str);
        ArgStream stream = {
            .argc = 1,
            .argv = &cenv,
        };
        if(!cenv) continue;
        TRYC(argx_parse(parse, &stream, x, quit_early));
        //if(parse->help.get) goto error;
    }
    /* check optional arguments */
    while(parse->instream.i < parse->instream.argc) {
        StrC argV = str("");
        TRYC(arg_parse_getv(parse, &parse->instream, &argV, &need_help));
        if(need_help) break;
        if(*quit_early) goto quit_early;
        if(!str_len_raw(argV)) continue;
        //printff(" [%.*s] %zu / %zu", STR_F(argV), parse->i, parse->argc);
        if(!parse->force_done_parsing && str_len_raw(argV) >= 1 && str_at(argV, 0) == pfx) {
            /* regular checking for options */
            if(str_len_raw(argV) >= 2 && str_at(argV, 1) == pfx) {
                ASSERT(str_len_raw(argV) > 2, ERR_UNREACHABLE);
                StrC arg_query = str_i0(argV, 2);
                /* long option */
                TRYC(arg_parse_getopt(&arg->tables.opt, &argx, arg_query));
                if(!argx->attr.is_env) {
                    TRYC(argx_parse(parse, &parse->instream, argx, quit_early));
                } else {
                    ASSERT_ARG(0);
                }
            } else {
                StrC arg_queries = str_i0(argV, 1);
                /* short option */
                for(size_t i = 0; i < str_len_raw(arg_queries); ++i) {
                    const unsigned char query = str_at(arg_queries, i);
                    TRYC(arg_parse_getopt_short(arg, &argx, query));
                    if(!argx->attr.is_env) {
                        TRYC(argx_parse(parse, &parse->instream, argx, quit_early));
                    } else {
                        ASSERT_ARG(0);
                    }
                    //printff("SHORT OPTION! %.*s", STR_F(arg_queries));
                }
                //ArgX *argx = arg->opt_short[
            }
        } else if(parse->instream.n_pos_parsed < array_len(arg->pos.list)) {
            /* check for positional */
            arg_parse_getv_undo(parse, &parse->instream);
            ArgX *x = array_at(arg->pos.list, parse->instream.n_pos_parsed);
            TRYC(argx_parse(parse, &parse->instream, x, quit_early));
            ++parse->instream.n_pos_parsed;
        } else if(parse->rest.vec) {
            /* no argument, push rest */
            array_push(*parse->rest.vec, argV);
        }
        /* in case of trying to get help, also search pos and then env and then group */
        if(parse->help.get_explicit && parse->instream.i < parse->instream.argc) {
            (void)arg_parse_getv(parse, &parse->instream, &argV, &need_help);
            ArgX *x = targx_get(&arg->tables.opt.lut, argV);
            //printff("GET HELP [%.*s]", STR_F(argV));
            if(argV.len) {
                for(size_t j = 0; j < vargxgroup_length(arg->groups); ++j) {
                    ArgXGroup **group = vargxgroup_get_at(&arg->groups, j);
                    if(!str_cmp(argV, (*group)->desc)) {
                        arg->parse.help.group = *group;
                    }
                }
            }
            if(x) {
                arg->parse.help.x = x;
            }
            if(!x && !arg->parse.help.group) {
                arg_parse_getv_undo(parse, &parse->instream);
            }
        }
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
        //printff("CHECK QUEUE [%.*s]", STR_F(x->info.opt));
        if(x && x->attr.callback.func) {
            if(x->attr.callback.func(x->attr.callback.data)) {
                THROW_PRINT("failed executing function for " F("[%.*s]", BOLD) "\n", STR_F(x->info.opt));
                goto error_skip_help;
            }
            *quit_early = x->attr.callback.quit_early;
            if(*quit_early) goto quit_early;
        }
    }
#endif
quit_early:
    if(arg->base.compgen_wordlist) {
        arg_help(arg);
        *quit_early = true;
        goto clean;
    }
    if((parse->instream.argc < 2 && arg->base.show_help)) {
        arg_help(arg);
        *quit_early = true;
    } else if(!arg->parse.help.get && parse->instream.n_pos_parsed < array_len(arg->pos.list)) {
        THROW("missing %zu positional arguments", array_len(arg->pos.list) - parse->instream.n_pos_parsed);
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

int arg_config_error(struct Arg *arg, StrC line, size_t line_nb, StrC opt, ArgX *argx) {
    ASSERT_ARG(arg);
    //if(!arg->base.source_check) return 0;
    int done = 0;
    if(arg->base.compgen_wordlist) return 0;
    if(line.str) {
        THROW_PRINT("config error on " F("line %zu", BOLD FG_MG_B) ":\n", line_nb);
        if(!opt.str) {
            ERR_PRINTF("        %.*s:\n", STR_F(line));
            ERR_PRINTF("        ^");
        } else {
            Str pre = str_ll(line.str, opt.str - line.str);
            Str at = opt;
            Str post = str_i0(line, str_len_raw(pre) + str_len_raw(at));
            ERR_PRINTF("        %.*s" F("%.*s", BOLD FG_RD_B) "%.*s\n", STR_F(pre), STR_F(at), STR_F(post));
            ERR_PRINTF("        %*s", (int)(opt.str - line.str), "");
            for(size_t i = 0; i < str_len_raw(opt); ++i) {
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

ErrDecl arg_config_from_str(struct Arg *arg, StrC text) {
    ASSERT_ARG(arg);
    int err = 0;
    size_t line_nb = 0;
    StrC line = {0}, opt = {0}, conf = text;
    if(!str_len_raw(conf)) return 0;
    ArgX *argx = 0;
    for(memset(&line, 0, sizeof(line)); str_splice(conf, &line, '\n'); ++line_nb) {
        if(!line.str) continue;
        line = str_trim(line);
        line = str_iE(line, str_find_ch(line, '#'));
        argx = 0;
        if(!str_len_raw(line)) continue;
        //printff("CONFIG:%.*s",STR_F(line));
        for(memset(&opt, 0, sizeof(opt)); str_splice(line, &opt, '='); ) {
            if(!opt.str) continue;
            opt = str_trim(opt);
            //printff(" OPT:%.*s",STR_F(opt));
            if(!argx) {
                TRYC(arg_parse_getopt(&arg->tables.opt, &argx, opt));
                if(argx->id == ARG_HELP) {
                    THROW("cannot configure help");
                } else if(argx->attr.is_env) {
                    THROW("cannot configure env");
                } else if(argx->id == ARG_NONE) {
                    THROW("cannot configure non-value option");
                }
            } else {
                //printff("setting value for [%.*s] : %.*s", STR_F(argx->info.opt), STR_F(opt));
                switch(argx->id) {
                    case ARG_OPTION: {
                        ASSERT_ARG(argx->o);
                        ArgXTable *table = argx->o->table;
                        ArgX *x = 0;
                        TRYC(arg_parse_getopt(table, &x, opt));
                        argx = x;
                    } break;
                    case ARG_BOOL: {
                        bool *b = argx->ref.b ? argx->ref.b : argx->val.b;
                        TRYC(str_as_bool(opt, b));
                    } break;
                    case ARG_SSZ: {
                        ssize_t *z = argx->ref.z ? argx->ref.z : argx->val.z;
                        TRYC(str_as_ssize(opt, z, 0));
                    } break;
                    case ARG_INT: {
                        int *i = argx->ref.i ? argx->ref.i : argx->val.i;
                        ssize_t z = 0;
                        TRYC(str_as_ssize(opt, &z, 0));
                        *i = (int)z;
                    } break;
                    case ARG_FLOAT: {
                        double *f = argx->ref.f ? argx->ref.f : argx->val.f;
                        TRYC(str_as_double(opt, f));
                    } break;
                    case ARG_COLOR: {
                        Color *c = argx->ref.c ? argx->ref.c : argx->val.c;
                        TRYC(str_as_color(opt, c));
                    } break;
                    case ARG_STRING: {
                        Str *s = argx->ref.s ? argx->ref.s : argx->val.s;
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
                        for(size_t i = 0; i < array_len(argx->o->list); ++i) {
                            ArgX *x = array_at(argx->o->list, i);
                            bool *b = x->ref.b ? x->ref.b : x->val.b;
                            *b = false;
                        }
                        for(Str flag = {0}; str_splice(opt, &flag, arg->base.flag_sep); ) {
                            if(!flag.str) continue;
                            ArgX *x = 0;
                            TRYC(arg_parse_getopt(argx->o->table, &x, flag));
                            bool *b = x->ref.b ? x->ref.b : x->val.b;
                            *b = true;
                        }
                    } break;
                    case ARG_VECTOR: {
                        VStr *v = argx->ref.v ? argx->ref.v : argx->val.v;
                        array_push(*v, opt);
                    } break;
                    case ARG_HELP:
                    case ARG__COUNT: ABORT(ERR_UNREACHABLE);
                }
                //printff(" GROUP %p", argx->group);
                //printff(" PARENT %p", argx->group ? argx->group->parent : 0);
                //printff(" ID %s", argx->group ? argx->group->parent ? arglist_str(argx->group->parent->id) : "" : 0);
            }
            /* check enum / option; TODO DRY */
            if(argx->group && argx->group && argx->group->parent && argx->group->parent->id == ARG_OPTION) {
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

void argx_table_free(ArgXTable *table) {
    ASSERT_ARG(table);
    targx_free(&table->lut);
    memset(table, 0, sizeof(*table));
}

void argx_group_free(ArgXGroup *group) {
    ASSERT_ARG(group);
    argx_table_free(group->table);
    array_free(group->list);
    if(group->parent) free(group->table);
    memset(group, 0, sizeof(*group));
}

void argx_group_free_array(ArgXGroup **group) {
    ASSERT_ARG(group);
    if(*group) {
        argx_group_free(*group);
        free(*group);
    }
    memset(group, 0, sizeof(*group));
}

void argx_free(ArgX *argx) {
    ASSERT_ARG(argx);
    if((argx->id == ARG_OPTION || argx->id == ARG_FLAGS)) {
        argx_group_free(argx->o);
        if(argx->group && argx->group->parent) free(argx->table);
        free(argx->o);
    }
    if(argx->id == ARG_VECTOR) {
        array_free(*argx->val.v);
    }
};


void arg_free(struct Arg **parg) {
    //printff("FREE ARGS");
    ASSERT_ARG(parg);
    Arg *arg = *parg;

    vargxgroup_free(&arg->groups);
    //array_free_set(arg->groups, ArgXGroup, (ArrayFree)argx_group_free_array);
    //array_free(arg->groups);
    argx_group_free(&arg->pos);

    argx_table_free(&arg->tables.opt);
    argx_table_free(&arg->tables.pos);

    vstr_free_set(&arg->parse.config);
    vstr_free_set(&arg->parse.config_files_base);
    vstr_free_set(&arg->parse.config_files_expand);
    array_free(arg->parse.config);
    array_free(arg->parse.config_files_base);
    array_free(arg->parse.config_files_expand);
    if(arg->base.rest_vec) array_free(*arg->base.rest_vec);
    free(*parg);
    *parg = 0;
    //printff("FREED ARGS");
}

/*}}}*/

void argx_builtin_env_compgen(ArgXGroup *group) {
    ASSERT_ARG(group);
    struct ArgX *x = argx_env(group, str("COMPGEN_WORDLIST"), str("Generate input for autocompletion"), false);
    argx_bool(x, &group->root->base.compgen_wordlist, 0);
    argx_bool_require_tf(x, true);
    //argx_func(x, 0, argx_callback_env_compgen, arg, true);
}

void argx_builtin_opt_help(ArgXGroup *group) {
    ASSERT_ARG(group);
    struct ArgX *x = argx_init(group, 'h', str("help"), str("print this help"));
    argx_help(x, group->root);
}

void argx_builtin_opt_fmtx(ArgX *x, StrFmtX *fmt, StrFmtX *ref) {
    struct ArgXGroup *g = 0;
    g=argx_opt(x, 0, 0);
      x=argx_init(g, 0, str("fg"), str("foreground"));
        argx_col(x, &fmt->fg, ref ? &ref->fg : 0);
      x=argx_init(g, 0, str("bg"), str("background"));
        argx_col(x, &fmt->bg, ref ? &ref->bg : 0);
      x=argx_init(g, 0, str("bold"), str("bold"));
        argx_bool(x, &fmt->bold, ref ? &ref->bold : 0);
      x=argx_init(g, 0, str("it"), str("italic"));
        argx_bool(x, &fmt->italic, ref ? &ref->italic : 0);
      x=argx_init(g, 0, str("ul"), str("underline"));
        argx_bool(x, &fmt->underline, ref ? &ref->underline : 0);
}

void argx_builtin_opt_rice(ArgXGroup *group) {
    ASSERT_ARG(group);

    struct Arg *arg = group->root;
    struct ArgX *x = 0;
    x=argx_init(group, 0, str("fmt-program"), str("program formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.program, 0);

    x=argx_init(group, 0, str("fmt-group"), str("group formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.group, 0);
    x=argx_init(group, 0, str("fmt-group-delim"), str("group delimiter formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.group_delim, 0);

    x=argx_init(group, 0, str("fmt-pos"), str("positional formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.pos, 0);
    x=argx_init(group, 0, str("fmt-short"), str("short option formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.c, 0);
    x=argx_init(group, 0, str("fmt-long"), str("long option formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.opt, 0);
    x=argx_init(group, 0, str("fmt-env"), str("environmental formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.env, 0);
    x=argx_init(group, 0, str("fmt-desc"), str("description formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.desc, 0);

    x=argx_init(group, 0, str("fmt-one"), str("one-of formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.one_of, 0);
    x=argx_init(group, 0, str("fmt-one-set"), str("one-of set formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.one_of_set, 0);
    x=argx_init(group, 0, str("fmt-one-delim"), str("one-of delimiter formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.one_of_delim, 0);

    x=argx_init(group, 0, str("fmt-flag"), str("flag formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.flag, 0);
    x=argx_init(group, 0, str("fmt-flag-set"), str("flag set formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.flag_set, 0);
    x=argx_init(group, 0, str("fmt-flag-delim"), str("flag delimiter formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.flag_delim, 0);

    x=argx_init(group, 0, str("fmt-type"), str("type formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.type, 0);
    x=argx_init(group, 0, str("fmt-type-delim"), str("type delimiter formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.type_delim, 0);

    x=argx_init(group, 0, str("fmt-val"), str("value formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.val, 0);
    x=argx_init(group, 0, str("fmt-val-delim"), str("value delimiter formatting"));
      argx_builtin_opt_fmtx(x, &arg->fmt.val_delim, 0);
}

void argx_builtin_opt_source(struct ArgXGroup *group, Str source) {
    ASSERT_ARG(group);
    struct Arg *arg = group->root;
    array_push(arg->parse.config_files_base, source);
    struct ArgX *x = targx_get(&group->table->lut, str("source"));
    if(!x) {
        x=argx_init(group, 0, str("source"), str("source other config files"));
          argx_vstr(x, &arg->parse.config_files_base, 0);
        // TODO: need better error handling!
        //x=argx_init(group, 0, str("source-check"), str("check sources for validity"));
        //  argx_bool(x, &arg->base.source_check, 0);
    }
}

