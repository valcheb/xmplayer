#include "fs_wrapper_linux.h"

static FILE *file = NULL;

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

    if(di == NULL)
        return FSWRESULT_ENDOFDIR;

    uint8_t i;
    for (i=0;i<DIRITEM_NAME-1;i++)
        item->name[i] = di->d_name[i];
    item->name[DIRITEM_NAME-1] = '\0';

    return FSWRESULT_OK;
}

fswresult_e fs_wrapper_linux_open(const char *fname)
{
    if(file != NULL)
        fclose(file);

    file = fopen(fname, "r"); // read only for compliance with fatpetit
    if (file == NULL)
        return FSWRESULT_ERROR;

    return FSWRESULT_OK;
}

fswresult_e fs_wrapper_linux_read(void *readbuf, uint16_t size, uint16_t *bread)
{
    *bread = (uint16_t)fread(readbuf, 1, size, file);

    if (ferror(file))
        return FSWRESULT_ERROR;

    return FSWRESULT_OK;
}
