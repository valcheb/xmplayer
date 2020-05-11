#include "xmparser.h"

static xm_song_info_t song_info;

#define SIZE_OF_ARRAY(arr) sizeof((arr))/sizeof((arr)[0])
#define ADD_END_OF_STRING(string) (string)[SIZE_OF_ARRAY((string))-1] = '\0'
#define READ_VALUE(value) do {\
    if (fs_access_ctx.read(&(value), sizeof((value))) != FSWRESULT_OK)\
        return XMRESULT_ERROR;\
} while(0);
#define READ_STRING(string) do {\
    if (fs_access_ctx.read((string), SIZE_OF_ARRAY((string))-1) != FSWRESULT_OK)\
        return XMRESULT_ERROR;\
    ADD_END_OF_STRING((string));\
} while(0);

xmresult_e xm_read_main_header(xm_main_header_t *head)
{
    if (fs_access_ctx.seek(0) != FSWRESULT_OK)
        return XMRESULT_ERROR;
    
    READ_VALUE(*(head));

    return XMRESULT_OK;
}

xmresult_e xm_read_pattern_header(uint32_t offset, xm_pattern_header_t *phead)
{
    if (fs_access_ctx.seek(offset) != FSWRESULT_OK)
        return XMRESULT_ERROR;

    READ_VALUE(*(phead));

    return XMRESULT_OK;
}