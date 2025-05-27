#include "../src/file.h"

int do_work(const Str2 file, void *p) {
    //str2_extend((Str2 *)p, file);
    //str2_push((Str2 *)p, '\n');
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

    Str2 out = {0};
    try(file_exec(str2("/home/rphii"), &subdirs, true, true, do_work, &out));
    while(array_len(subdirs)) {
        Str2 subdir = array_pop(subdirs);
        try(file_exec(subdir, &subdirs, true, true, do_work, &out));
        str2_free(&subdir);
    }
    printf("%.*s", STR2_F(out));
    
    array_free(subdirs);
    str2_free(&content);
    str2_free(&out);
    return 0;
}

