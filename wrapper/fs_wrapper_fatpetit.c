#include "fs_wrapper_fatpetit.h"

DFRESULT fs_wrapper_fatpetit_opendir(fs_wrapper_dir_t *dir, const char* dirpath)
{
    DIR pf_dir;
    FRESULT res;

    res = pf_opendir(&pf_dir,dirpath);
 
    if(res == FR_OK)
    {
        dir->data = pf_dir;
        return DFRESULT_OK;
    }
    else
    {
        dir->data = NULL; //TODO: find NULL declaration for stm32
        return DFRESULT_ERROR;
    }
}