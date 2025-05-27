#include <errno.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <string.h>

#include "file.h"

/******************************************************************************/
/* PUBLIC FUNCTION IMPLEMENTATIONS ********************************************/
/******************************************************************************/

FileTypeList file_get_type(Str2 filename) {
    struct stat s;
    char path[FILE_PATH_MAX];
    str2_as_cstr(filename, path, FILE_PATH_MAX);
    int r = lstat(path, &s);
    if(r) return FILE_TYPE_ERROR;
    if(S_ISREG(s.st_mode)) return FILE_TYPE_FILE;
    if(S_ISDIR(s.st_mode)) return FILE_TYPE_DIR;
    return 0;
}

int file_is_dir(const Str2 filename)
{
    struct stat s;
    char path[FILE_PATH_MAX];
    str2_as_cstr(filename, path, FILE_PATH_MAX);
    int r = lstat(path, &s);
    if(r) return 0;
    return S_ISDIR(s.st_mode);
    return 0;
}

size_t file_size(Str2 filename) {/*{{{*/
    char path[FILE_PATH_MAX];
    str2_as_cstr(filename, path, FILE_PATH_MAX);
    FILE *fp = fopen(path, "rb");
    size_t result = SIZE_MAX;
    if(fp) {
        if(!fseek(fp, 0L, SEEK_END)) {
            result = ftell(fp);
        }
        fclose(fp);
    }
    return result;
}/*}}}*/

int file_is_file(Str2 *filename)
{
    struct stat s;
    char path[4094];
    str2_as_cstr(*filename, path, FILE_PATH_MAX);
    int r = lstat(path, &s);
    if(r) return 0;
    return S_ISREG(s.st_mode);
    return 0;
}

int file_fp_write(FILE *file, Str2 *content)
{
    if(!file) THROW("invalid filename");
    if(!content) THROW("invalid output buffer");

    /* write file */
    size_t bytes_written = fwrite(content->str, 1, str2_len(*content), file);
    if(bytes_written != str2_len(*content)) {
        THROW("bytes written (%zu) mismatch bytes to write (%zu)!", bytes_written, str2_len(*content));
    }

    /* close file outside */
    return 0;
error:
    return -1;
}

int file_fp_read(FILE *file, Str2 *content)
{
    if(!file) THROW("invalid filename");
    if(!content) THROW("invalid output buffer");

    /* get file length */
    fseek(file, 0, SEEK_END);
    size_t bytes_file = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    printff("file length %zu", bytes_file);
    /* allocate memory */
    str2_resize(content, bytes_file);

    /* read file */
    size_t bytes_read = fread(content->str, 1, bytes_file, file);
    //if(bytes_file != bytes_read) THROW("mismatch read vs expected bytes");
    content->str[bytes_read] = 0;
    content->len = bytes_read;

    /* close file outside */
    return 0;
error:
    return -1;
}

int file_str_read(Str2 filename, Str2 *content)
{
    int err = 0;
    FILE *file = 0;
    if(!content) THROW("invalid output buffer");

    /* open the file */
    errno = 0;
    if(filename.len && (
                filename.str[filename.len] == PLATFORM_CH_SUBDIR ||
                filename.str[filename.len] == '/')) {
        THROW("won't open directories");
    }

    char path[FILE_PATH_MAX] = {0};
    str2_as_cstr(filename, path, FILE_PATH_MAX);
    file = fopen(path, "r");
    if(!file || errno) {
        //goto clean;
        THROW("failed to open file named '%s'", path);
    }

    TRYC(file_fp_read(file, content));
    /* close file */
clean:
    if(file) fclose(file);
    return err;
error: ERR_CLEAN;
}

#if 1
int file_str2_write(Str2 filename, Str2 *content)
{
    int err = 0;
    FILE *file = 0;
    if(!content) THROW("invalid output buffer");

    /* open the file */
    errno = 0;
    if(filename.len && (
                filename.str[filename.len] == PLATFORM_CH_SUBDIR ||
                filename.str[filename.len] == '/')) {
        THROW("won't open directories");
    }

    char path[FILE_PATH_MAX] = {0};
    str2_as_cstr(filename, path, FILE_PATH_MAX);
    file = fopen(path, "w");
    if(!file || errno) {
        //goto clean;
        THROW("failed to open file named '%s'", path);
    }

    TRYC(file_fp_write(file, content));
clean:
    if(file) fclose(file);
    return err;
error: ERR_CLEAN;
}
#endif

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

ErrDecl file_exec(Str2 path, VStr2 *subdirs, bool recursive, bool hidden, FileFunc exec, void *args) {
    ASSERT_ARG(subdirs);
    ASSERT_ARG(exec);
    int err = 0;
    DIR *dir = 0;
    Str2 dot = str2(".");
    Str2 dotdot = str2("..");
    //printf("FILENAME: %.*s\n", STR_F(path));
    FileTypeList type = file_get_type(path);
    array_free_set(*subdirs, Str2, (ArrayFree)str2_free);
    Str2 filename = {0};
    if(type == FILE_TYPE_DIR) {
        if(!recursive) {
            THROW("will not go over '%.*s' (enable recursion to do so)", STR2_F(path));
        }
        struct dirent *dp = 0;
        Str2 direns = str2_ensure_dir(path);
        char cdir[FILE_PATH_MAX];
        str2_as_cstr(direns, cdir, FILE_PATH_MAX);
        if((dir = opendir(cdir)) == NULL) {
            goto clean;
            THROW("can't open directory '%.*s'", STR2_F(direns));
        }
        while((dp = readdir(dir)) != NULL) {
            Str2 dname = str2_l(dp->d_name);
            if(!str2_hcmp(&dname, &dot)) continue;
            if(!str2_hcmp(&dname, &dotdot)) continue;
            if(!hidden && !str2_cmp0(dname, dot)) continue;
            str2_extend(&filename, direns);
            if(str2_len(direns) > 1) str2_push(&filename, PLATFORM_CH_SUBDIR);
            str2_extend(&filename, dname);
            FileTypeList type2 = file_get_type(filename);
            if(type2 == FILE_TYPE_DIR) {
                array_push(*subdirs, filename);
                str2_zero(&filename);
            } else if(type2 == FILE_TYPE_FILE) {
                TRY(exec(filename, args), "an error occured while executing the function");
                str2_clear(&filename);
            } else {
                str2_clear(&filename);
                //info(INFO_skipping_nofile_nodir, "skipping '%.*s' since no regular file nor directory", STR_F(*path));
            }
        }
    } else if(type == FILE_TYPE_FILE) {
        TRY(exec(path, args), "an error occured while executing the function");
    } else if(type == FILE_TYPE_ERROR) {
        THROW("failed checking type of '%.*s' (maybe it doesn't exist?)", STR2_F(path));
    } else {
        //info(INFO_skipping_nofile_nodir, "skipping '%.*s' since no regular file nor directory", STR_F(*path));
    }
clean:
    str2_free(&filename);
    if(dir) closedir(dir);
    return err;
error: ERR_CLEAN;
}

ErrDecl file_dir_read(Str2 dirname, VStr2 *files) {
    int err = 0;
    DIR *dir = 0;
    size_t len = str2_rfind_ch(dirname, PLATFORM_CH_SUBDIR);
    if(len < str2_len(dirname) && str2_at(dirname, len) != PLATFORM_CH_SUBDIR) ++len;
    struct dirent *dp = 0;
    if ((dir = opendir(dirname.str)) == NULL) {
        //goto clean;
        THROW("can't open directory '%.*s'", (int)len, dirname.str);
    }
    array_free_set(*files, Str2, (ArrayFree)str2_free);
    while ((dp = readdir(dir)) != NULL) {
        Str2 filename = {0};
        if(dp->d_name[0] == '.') continue; // TODO add an argument for this
        if(!str2_cmp(str2_l(dp->d_name), str2(".")) || !str2_cmp(str2_l(dp->d_name), str2(".."))) continue;
        str2_fmt(&filename, "%.*s/%s", (int)len, dirname.str, dp->d_name);
        //printf("FILE: %.*s\n", STR_F(&filename));
        array_push(*files, filename);
    }
clean:
    if(dir) closedir(dir);
    return err;
error: ERR_CLEAN;
}

