#include "fs_wrapper_linux.h"

static FILE *file = NULL;

fs_access_t fs_access_ctx = 
{
    .opendir = fs_wrapper_linux_opendir,
    .readdir = fs_wrapper_linux_readdir,
    .open = fs_wrapper_linux_open,
    .read = fs_wrapper_linux_read,
    .seek = fs_wrapper_linux_seek
};

fswresult_e fs_wrapper_linux_opendir(fs_wrapper_dir_t *dir, const char *dirpath)
{
    dir->data = opendir(dirpath);
    if (dir->data == NULL)
        return FSWRESULT_ERROR;

    return FSWRESULT_OK;
}

fswresult_e fs_wrapper_linux_readdir(fs_wrapper_dir_t *dir, fs_wrapper_diritem_t *item)
{
    struct dirent *di;
    di = readdir(dir->data);

    if (di == NULL)
        return FSWRESULT_ENDOFDIR;

    strncpy(item->name,di->d_name,DIRITEM_NAME-1);
    item->name[DIRITEM_NAME-1] = '\0';

    return FSWRESULT_OK;
}

fswresult_e fs_wrapper_linux_open(const char *fname)
{
    if (file != NULL)
        fclose(file);

    file = fopen(fname, "r"); // read only for compliance with fatpetit
    if (file == NULL)
        return FSWRESULT_ERROR;

    return FSWRESULT_OK;
}

fswresult_e fs_wrapper_linux_read(void *readbuf, uint16_t size)
{
    fread(readbuf, 1, size, file);

    if (ferror(file))
        return FSWRESULT_ERROR;

    return FSWRESULT_OK;
}

fswresult_e fs_wrapper_linux_seek(uint32_t offset)
{
    if (fseek(file, offset, SEEK_SET))
        return FSWRESULT_ERROR;

    return FSWRESULT_OK;
}