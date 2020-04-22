#ifndef FS_WRAPPER_LINUX_H_
#define FS_WRAPPER_LINUX_H_

#include "fs_wrapper_global.h"
#include <stdio.h>

fswresult_e fs_wrapper_linux_opendir(fs_wrapper_dir_t *dir, const char *dirpath);
fswresult_e fs_wrapper_linux_open(const char *fname);

#endif /*FS_WRAPPER_LINUX_H_*/