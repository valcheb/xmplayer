#include "fs_wrapper_linux.h"

DFRESULT fs_wrapper_linux_opendir(fs_wrapper_dir_t *dir, const char * dirpath)
{
    dir->data = opendir(dirpath);
    if(dir->data == NULL)
        return DFRESULT_ERROR;
    else
        return DFRESULT_OK;
}
