#include "xmparser.h"

static xm_song_info_t song_info;

#define XM_MAIN_HEADER_OFFSET         0
#define XM_REST_OF_MAIN_HEADER_OFFSET 60
#define XM_PATTERN_ORDER_TABLE_OFFSET 80
#define SIZE_OF_ARRAY(arr) sizeof(arr)/sizeof((arr)[0])
#define ADD_END_OF_STRING(string) (string)[SIZE_OF_ARRAY(string)-1] = '\0'
#define READ_VALUE(value) do {\
    if (fs_access_ctx.read(&(value), sizeof(value)) != FSWRESULT_OK)\
        return XMRESULT_ERROR;\
} while(0);

xmresult_e xm_read_main_header(xm_main_header_t *head)
{
    if (fs_access_ctx.seek(0) != FSWRESULT_OK)
        return XMRESULT_ERROR;
    
    READ_VALUE(*head);

    return XMRESULT_OK;
}

xmresult_e xm_read_pattern_header(uint32_t offset, xm_pattern_header_t *phead)
{
    if (fs_access_ctx.seek(offset) != FSWRESULT_OK)
        return XMRESULT_ERROR;

    READ_VALUE(*phead);

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

    READ_VALUE(*shead);

    return XMRESULT_OK;
}

xmresult_e xm_read_song_info(xm_song_info_t *song)
{
    if (xm_read_main_header(&song->main_header) != XMRESULT_OK)
        return XMRESULT_ERROR;

    song->pattern_order_table = XM_PATTERN_ORDER_TABLE_OFFSET;
    song->pattern_order_table_size = song->main_header.header_size - XM_PATTERN_ORDER_TABLE_OFFSET;
    song->first_pattern = XM_REST_OF_MAIN_HEADER_OFFSET + song->main_header.header_size;

    uint32_t current_offset;
    if (xm_read_pattern_offset(song->main_header.patterns_number, song->first_pattern, &current_offset) != XMRESULT_OK)
        return XMRESULT_ERROR;

    if (xm_read_instruments(current_offset, song->instruments, song->main_header.instruments_number) != XMRESULT_OK)
        return XMRESULT_ERROR;

    return XMRESULT_OK;
}

xmresult_e xm_read_instruments(uint32_t offset, uint32_t *instruments, uint16_t instrument_number)
{
    for (int i = 0; i < instrument_number; i++)
    {
        instruments[i] = offset;

        xm_instrument_header_t instrument_header;
        if (xm_read_instrument_header(instruments[i], &instrument_header) != XMRESULT_OK)
            return XMRESULT_ERROR;

        uint32_t samples_len = 0;
        offset += instrument_header.main_header.instrument_size; // move to first sample header in instrument
        for (int s = 0; s < instrument_header.main_header.samples_number; s++)
        {   
            xm_sample_header_t sample_header;
            if (xm_read_sample_header(offset, &sample_header) != XMRESULT_OK)
                return XMRESULT_ERROR;

            samples_len += sample_header.sample_length;
            offset += instrument_header.extra_header.sample_header_size; // move to next sample header
        }
        offset += samples_len; // jump over all sample datas
    }

    return XMRESULT_OK;
}

xmresult_e xm_init_song(char *song_name, xm_song_info_t *song_info)
{
    if (fs_access_ctx.open(song_name) != FSWRESULT_OK)
        return XMRESULT_ERROR;
    if (xm_read_song_info(song_info) != XMRESULT_OK)
        return XMRESULT_ERROR;

    return XMRESULT_OK;
}

xmresult_e xm_read_pattern_offset(uint16_t index, uint32_t first_pattern, uint32_t *offset)
{
    *offset = first_pattern;

    xm_pattern_header_t pattern_header;
    for (int i = 0; i < index; i++)
    {
        if (xm_read_pattern_header(*offset, &pattern_header) != XMRESULT_OK)
            return XMRESULT_ERROR;
        *offset += pattern_header.header_size + pattern_header.data_size;
    }

    return XMRESULT_OK;
}