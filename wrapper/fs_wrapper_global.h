#ifndef FS_WRAPPER_GLOBAL_H_
#define FS_WRAPPER_GLOBAL_H_

#define LINUX_PLATFORM // temporary define
#define DIRITEM_NAME 13

#ifdef LINUX_PLATFORM
#include <dirent.h>
#else
#include "pff.h"
#endif

typedef enum
{
    FSWRESULT_OK = 0,
    FSWRESULT_ERROR,
    FSWRESULT_ENDOFDIR
} fswresult_e;

typedef struct fs_wrapper_dir
{
    #ifdef LINUX_PLATFORM
    DIR *data;
    #else
    DIR data;
    #endif
} fs_wrapper_dir_t; //DIR and DIR are different for different platforms

typedef struct fs_wrapper_diritem
{
    char name[DIRITEM_NAME];
} fs_wrapper_diritem_t;

#endif /*FS_WRAPPER_GLOBAL_H_*/