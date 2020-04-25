#include "fs_wrapper_fatpetit.h"

// TODO: debug all
fswresult_e fs_wrapper_fatpetit_opendir(fs_wrapper_dir_t *dir, const char* dirpath)
{
    DIR pf_dir;
    FRESULT res;

    res = pf_opendir(&pf_dir,dirpath); //TODO: change pf_dir >>> dir->data
 
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

fswresult_e fs_wrapper_fatpetit_readdir(fs_wrapper_dir_t *dir, fs_wrapper_diritem_t *item)
{
    FILINFO fil;
    FRESULT res = pf_readdir(&(dir->data),&fil);

    if(res != FR_OK)
        return FSWRESULT_ERROR;

    strncpy(item->name,fil.fname,DIRITEM_NAME-1);
    item->name[DIRITEM_NAME-1] = '\0';

    return FSWRESULT_OK;
}