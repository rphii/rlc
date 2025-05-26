#ifndef FILE_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "err.h"
#include "str2.h"

/******************************************************************************/
/* PUBLIC FUNCTION PROTOTYPES *************************************************/
/******************************************************************************/

typedef enum {
    FILE_TYPE_NONE,
    FILE_TYPE_FILE,
    FILE_TYPE_DIR,
    FILE_TYPE_ERROR,
} FileTypeList;

typedef int (*FileFunc)(Str2 filename, void *);

#define ERR_file_exec(dirname, subdirs, rec, exec, args) "an error occured executing function on files '%.*s'", RSTR_F(dirname)
ErrDecl file_exec(Str2 path, VStr2 subdirs, bool recursive, FileFunc exec, void *args);

#define FILE_PATH_MAX   4096

FileTypeList file_get_type(Str2 filename);

int file_is_dir(Str2 filename);
size_t file_size(Str2 filename);

#define ERR_file_str_read(filename, content) "failed reading file '%.*s'", RSTR_F(filename)
ErrDecl file_str_read(Str2 filename, Str2 *content);

#define ERR_file_str_write(filename, content) "failed writing file '%.*s'", RSTR_F(filename)
ErrDecl file_str_write(Str2 filename, Str2 *content);

#define ERR_file_fp_write(file, content) "failed writing file pointer '%p'", file
ErrDecl file_fp_write(FILE *file, Str2 *content);

#define ERR_file_fp_read(file, content) "failed reading file pointer '%p'", file
ErrDecl file_fp_read(FILE *file, Str2 *content);

#define ERR_file_dir_read(dirname, files) "failed reading directory"
ErrDecl file_dir_read(Str2 dirname, VStr2 *files);

#define FILE_H
#endif
