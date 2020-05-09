#ifndef XMPARSER_H_
#define XMPARSER_H_

#ifdef LINUX_PLATFORM
# include "fs_wrapper_linux.h"
#else
# include "fs_wrapper_fatpetit.h"
#endif

typedef enum
{
    XMRESULT_OK = 0,
    XMRESULT_ERROR
} xmresult_e;

typedef struct main_header
{
    char idtext[18];
    char module_name[21];
    uint8_t sym; // always 0x1a
    char tracker_name[21];
    uint16_t version; //check this, should be 0x0104
    uint32_t header_size;
    uint16_t song_len;
    uint16_t restart_pos;
    uint16_t channels_number;
    uint16_t patterns_number;
    uint16_t instruments_number;
    uint16_t freq_table;
    uint16_t default_tempo;
    uint16_t default_bpm;
} xm_main_header_t;

typedef struct song_info
{
    xm_main_header_t main_header;
    uint32_t pattern_order_table;
    uint32_t pattern_order_table_size;
    uint32_t first_pattern;
    uint32_t instruments[128];
} xm_song_info_t;

xmresult_e xm_fill_main_header(xm_main_header_t *head);

#endif /*XMPARSER_H_*/