#ifndef ERR_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "colorprint.h"
#include "attr.h"

#define SIZE_IS_NEG(x)      (((SIZE_MAX >> 1) + 1) & (x))

#ifndef SSIZE_MIN
#define SSIZE_MIN           ((ssize_t)1 << (8*sizeof(ssize_t)-1))
#endif
#ifndef SSIZE_MAX
#define SSIZE_MAX           ~((ssize_t)1 << (8*sizeof(ssize_t)-1))
#endif

#define SIZE_ARRAY(x)       (sizeof(x)/sizeof(*x))

#define printff(fmt, ...)   do { \
        printf(fmt, ##__VA_ARGS__); \
        printf(F(" * ", FG_BL_B) F("%s:%i \n", FG_BK_B) , __func__, __LINE__); \
    } while(0);

/* use this when declaring function that can error */
#define ErrDecl             ATTR_NODISCARD int
#define ErrImpl             ATTR_NODISCARD inline int 
#define ErrImplStatic       ATTR_NODISCARD static inline int 
#define ErrDeclStatic       ATTR_NODISCARD static inline int

#define ERR_STRINGIFY(S)    #S
#define ERR_CLEAN           do { err = -1; goto clean; } while(0)
#define ERR(id)             do { err = (id); goto clean; } while(0)

/* general error messages */
#define ERR_INTERNAL(msg)       "internal error: " msg
#define ERR_UNHANDLED_ID(id)    "unhandled id: %u", id
#define ERR_UNREACHABLE(msg)    "unreachable error: " msg
#define ERR_MEMORY              "failed to allocate memory"
#define ERR_NULL_ARG            "unexpected null pointer argument received"

#define ERR_FILE_STREAM     stderr

#if DEBUG_DISABLE_ERR_MESSAGES
    #define ERR_PRINTF(fmt, ...)    {}
#else
    #define ERR_PRINTF(fmt, ...)    do { fprintf(ERR_FILE_STREAM, fmt, ##__VA_ARGS__); } while(0)
#endif

/* macros */

#define THROW_PRINT(fmt, ...)   do { \
        ERR_PRINTF(F("[ERROR]", BOLD FG_RD_B) " " F("%s:%d:%s", FG_WT_B) " " fmt "" , __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while(0)

#define THROW(fmt, ...)      do { \
        THROW_PRINT(fmt "\n", ##__VA_ARGS__); \
    goto error; } while(0)

#define THROW_P(print_err, fmt, ...)    do { \
        if(print_err) { \
            THROW(fmt, ##__VA_ARGS__); \
        } else { \
            goto error; \
        } \
    } while(0)

#define ABORT(fmt, ...)      do { \
        ERR_PRINTF(F("[ABORT]", BOLD FG_BK BG_RD_B) " " F("%s:%d:%s (end of trace)", FG_WT_B) " " fmt "\n" , __FILE__, __LINE__, __func__, ##__VA_ARGS__); exit(-1); \
    } while(0)

#define INFO(fmt, ...)       do { \
        ERR_PRINTF(F("[INFO]", BOLD FG_YL_B) " " F("%s:%d:%s", FG_WT_B) " " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while(0)

#ifndef NDEBUG
#define ASSERT(stmt, fmt, ...)   do { \
    if (!(stmt)) { \
        ABORT("assertion of '" ERR_STRINGIFY(stmt) "' failed... " fmt, ##__VA_ARGS__); } \
    } while(0)
#else
#define ASSERT(stmt, fmt, ...)   do { } while(0)
#endif

#define ASSERT_ARG(arg)     ASSERT(arg, ERR_NULL_ARG)

#define TRY(stmt, fmt, ...)                     do { if (stmt) { THROW(fmt, ##__VA_ARGS__); } } while(0)
#define TRYC(function)                          TRY(function, ERR_##function)           // try; err is const-defined-string
#define TRYG(function)                          TRY(function, "%s", ERR_##function)     // try; err is generic-string

#define TRY_P(print_err, stmt, fmt, ...)        do { if (stmt) { THROW_P(print_err, fmt, ##__VA_ARGS__); } } while(0)
#define TRYC_P(print_err, function)             TRY_P(print_err, function, ERR_##function)
#define TRYG_P(print_err, function)             TRY_P(print_err, function, "%s", ERR_##function)     // try; err is generic-string

#define NEW(T, result) do { \
        result = malloc(sizeof(T)); \
        if(!result) ABORT(ERR_MEMORY); \
        memset(result, 0, sizeof(T)); \
    } while(0) 

#define ERR_H
#endif

