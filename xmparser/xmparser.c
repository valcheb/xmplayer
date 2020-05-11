#include "xmparser.h"

static xm_song_info_t song_info;

#define XM_MAIN_HEADER_OFFSET         0
#define XM_REST_OF_MAIN_HEADER_OFFSET 60
#define XM_PATTERN_ORDER_TABLE_OFFSET 80
#define SIZE_OF_ARRAY(arr) sizeof((arr))/sizeof((arr)[0])
#define ADD_END_OF_STRING(string) (string)[SIZE_OF_ARRAY((string))-1] = '\0'
#define READ_VALUE(value) do {\
    if (fs_access_ctx.read(&(value), sizeof((value))) != FSWRESULT_OK)\
        return XMRESULT_ERROR;\
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

xmresult_e xm_read_instrument_header(uint32_t offset, xm_instrument_header_t *ihead)
{
    if (fs_access_ctx.seek(offset) != FSWRESULT_OK)
        return XMRESULT_ERROR;
    
    READ_VALUE(ihead->main_header);
    if (ihead->main_header.samples_number > 0)
        READ_VALUE(ihead->extra_header);
    
    return XMRESULT_OK;
}

xmresult_e xm_read_sample_header(uint32_t offset, xm_sample_header_t *shead)
{
    if (fs_access_ctx.seek(offset) != FSWRESULT_OK)
        return XMRESULT_ERROR;

    READ_VALUE(*(shead));

    return XMRESULT_OK;
}

xmresult_e xm_fill_song_info(xm_song_info_t *song)
{
    if (xm_read_main_header(&(song->main_header)) != XMRESULT_OK)
        return XMRESULT_ERROR;

    song->pattern_order_table = XM_PATTERN_ORDER_TABLE_OFFSET;
    song->pattern_order_table_size = song->main_header.header_size - XM_PATTERN_ORDER_TABLE_OFFSET;
    song->first_pattern = XM_REST_OF_MAIN_HEADER_OFFSET+song->main_header.header_size;
    if (xm_read_instruments(song) != XMRESULT_OK)
        return XMRESULT_ERROR;

    return XMRESULT_OK;
}