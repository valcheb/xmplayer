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
    
    READ_VALUE(head->idtext);
    READ_VALUE(head->module_name);
    READ_VALUE(head->sym);
    READ_VALUE(head->tracker_name);
    READ_VALUE(head->version);
    READ_VALUE(head->header_size);
    READ_VALUE(head->song_len);
    READ_VALUE(head->restart_pos);
    READ_VALUE(head->channels_number);
    READ_VALUE(head->patterns_number);
    READ_VALUE(head->instruments_number);
    READ_VALUE(head->freq_table);
    READ_VALUE(head->default_tempo);
    READ_VALUE(head->default_bpm);

    return XMRESULT_OK;
}

xmresult_e xm_read_pattern_header(uint32_t offset, xm_pattern_header_t *phead)
{
    if (fs_access_ctx.seek(offset) != FSWRESULT_OK)
        return XMRESULT_ERROR;

    READ_VALUE(*(phead));
    
    return XMRESULT_OK;
}