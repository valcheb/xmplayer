#ifndef FS_WRAPPER_GLOBAL_H_
#define FS_WRAPPER_GLOBAL_H_

#include <stdint.h>

#define DIRITEM_NAME 13

#ifdef LINUX_PLATFORM
# include <dirent.h>
#else
# include "pff.h"
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

typedef fswresult_e (*fs_opendir_t)(fs_wrapper_dir_t *dir, const char *dirpath);
typedef fswresult_e (*fs_readdir_t)(fs_wrapper_dir_t *dir, fs_wrapper_diritem_t *item);
typedef fswresult_e (*fs_open_t)(const char *fname);
typedef fswresult_e (*fs_read_t)(void *readbuf, uint16_t size);
typedef fswresult_e (*fs_seek_t)(uint32_t offset);

typedef struct fs_access
{
    fs_opendir_t opendir;
    fs_readdir_t readdir;
    fs_open_t open;
    fs_read_t read;
    fs_seek_t seek;
} fs_access_t;

fs_access_t fs_access_ctx;

#endif /*FS_WRAPPER_GLOBAL_H_*/