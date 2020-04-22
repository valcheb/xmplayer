#include "fs_wrapper_fatpetit.h"

fs_wrapper_dir_t fs_wrapper_fatpetit_opendir(const char* dirpath)
{
    fs_wrapper_dir_t wr_dir;
    FRESULT res;
    DIR pf_dir;
    
    res = pf_opendir(&pf_dir,dirpath);

    if(res == FR_OK)
        wr_dir.data = (DIR*)&pf_dir;
    else
        wr_dir.data = NULL: //TODO: find NULL declaration for stm32

    return wr_dir;
}