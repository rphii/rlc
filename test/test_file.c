#include "../src/file.h"

int do_work(Str2 file, void *p) {
    printff("%.*s", STR2_F(file));
    return 0;
}

int main(void) {

    Str2 content = {0};
    try(file_str_read(str2("test_file.c"), &content));
    printff("%.*s", STR2_F(content));

    VStr2 subdirs = 0;
    try(file_dir_read(str2(".."), &subdirs));
    for(size_t i = 0; i < array_len(subdirs); ++i) {
        Str2 *sub = array_it(subdirs, i);
        printff(" %.*s", STR2_F(*sub));
        str2_free(sub);
    }
    array_clear(subdirs);

    try(file_exec(str2("/home/rphii///"), &subdirs, true, do_work, 0));
    while(array_len(subdirs)) {
        Str2 subdir = array_pop(subdirs);
        try(file_exec(subdir, &subdirs, true, do_work, 0));
        str2_free(&subdir);
    }
    
    array_free(subdirs);
    str2_free(&content);
    return 0;
}

