#include "../src/file.h"

int main(void) {

    Str2 content = {0};
    try(file_str_read(str2("test_file.c"), &content));
    printff("%.*s", STR2_F(content));

    VStr2 subdirs = 0;
    try(file_dir_read(str2(".."), &subdirs));

    str2_free(&content);
    return 0;
}

