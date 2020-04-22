#include "fs_wrapper_fatpetit.h"

fswresult_e fs_wrapper_fatpetit_opendir(fs_wrapper_dir_t *dir, const char* dirpath)
{
    DIR pf_dir;
    FRESULT res;

    res = pf_opendir(&pf_dir,dirpath);
 
    if(res == FR_OK)
    {
        dir->data = pf_dir;
        return FSWRESULT_OK;
    }
    else
    {
        dir->data = NULL; //TODO: find NULL declaration for stm32
        return FSWRESULT_ERROR;
    }
}