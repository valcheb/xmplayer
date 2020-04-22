#ifndef FS_WRAPPER_GLOBAL_H_
#define FS_WRAPPER_GLOBAL_H_

#define LINUX_PLATFORM // temporary define

#ifdef LINUX_PLATFORM
#include <dirent.h>
#else
#include "pff.h"
#endif

typedef enum
{
    DFRESULT_OK = 0,
    DFRESULT_ERROR
} DFRESULT;

typedef struct wrapper_dir
{
    #ifdef LINUX_PLATFORM
    DIR *data;
    #else
    DIR data;
    #endif
} fs_wrapper_dir_t; //attention: DIR and DIR is different for different plarforms

#endif /*FS_WRAPPER_GLOBAL_H_*/