#ifndef FILE_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "err.h"
#include "str.h"

/******************************************************************************/
/* PUBLIC FUNCTION PROTOTYPES *************************************************/
/******************************************************************************/

typedef enum {
    FILE_TYPE_NONE,
    FILE_TYPE_FILE,
    FILE_TYPE_DIR,
    FILE_TYPE_ERROR,
} FileTypeList;

typedef int (*FileFunc)(const Str filename, void *);

#define ERR_file_exec(dirname, ...) "an error occured executing function on files '%.*s'", STR_F(dirname)
ErrDecl file_exec(Str path, VStr *subdirs, bool recursive, bool hidden, FileFunc exec, void *args);

#define FILE_PATH_MAX   4096

FileTypeList file_get_type(Str filename);

int file_is_dir(Str filename);
size_t file_size(Str filename);

#define ERR_file_str_read(filename, content) "failed reading file '%.*s'", STR_F(filename)
ErrDecl file_str_read(Str filename, Str *content);

#define ERR_file_str_write(filename, content) "failed writing file '%.*s'", STR_F(filename)
ErrDecl file_str_write(Str filename, Str *content);

#define ERR_file_fp_write(file, content) "failed writing file pointer '%p'", file
ErrDecl file_fp_write(FILE *file, Str *content);

#define ERR_file_fp_read(file, content) "failed reading file pointer '%p'", file
ErrDecl file_fp_read(FILE *file, Str *content);

#define ERR_file_dir_read(dirname, files) "failed reading directory"
ErrDecl file_dir_read(Str dirname, VStr *files);

#define FILE_H
#endif
