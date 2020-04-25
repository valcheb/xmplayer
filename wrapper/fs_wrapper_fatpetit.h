#ifndef FS_WRAPPER_FATPETIT_H_
#define FS_WRAPPER_FATPETIT_H_

#include "fs_wrapper_global.h"

fswresult_e fs_wrapper_fatpetit_opendir(fs_wrapper_dir_t *dir, const char *dirpath);
fswresult_e fs_wrapper_fatpetit_readdir(fs_wrapper_dir_t *dir, fs_wrapper_diritem_t *item);
fswresult_e fs_wrapper_fatpetit_open(const char *fname);
fswresult_e fs_wrapper_fatpetit_read(void *readbuf, uint16_t size, uint16_t *bread);

#endif /*FS_WRAPPER_FATPETIT_H_*/