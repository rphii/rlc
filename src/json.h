#ifndef JSON_H

#include <stdbool.h>
#include "str.h"

typedef enum {
    JSON_NONE,
    JSON_OBJ,
    JSON_ARR,
    JSON_VAL,
    JSON_STR,
    JSON_DBL,
    JSON_INT,
} JsonList;

typedef struct JsonResult {
    bool match;
    bool end;
    char front;
} JsonResult;

typedef struct JsonCallback JsonCallback;

typedef int (*JsonFunc)(JsonCallback *call, JsonList id, RStr val);

typedef struct JsonCallback {
    JsonFunc func;
    void *data;
} JsonCallback;

JsonResult json_verify(RStr *in);
JsonResult json_parse(RStr *in, JsonFunc func, void *data);

#define JSON_H
#endif

