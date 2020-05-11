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
    char idtext[17];
    char module_name[20];
    uint8_t sym; // always 0x1a
    char tracker_name[20];
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

typedef struct xm_pattern_header
{
    uint32_t header_size;
    uint8_t packing_type; // always 0?
    uint16_t rows_number;
    uint16_t data_size;
} xm_pattern_header_t;

typedef struct xm_instrument_header
{
    uint32_t instrument_size;
    char instrument_name[22];
    uint8_t instrument_type; // always 0?
    uint16_t samples_number;
    /*extra header for samples_number > 0*/
    uint32_t sample_header_size;
    uint8_t sample_number_for_notes[96];
    uint8_t volume_envelope_points[48];
    uint8_t panning_envelope_points[48];
    uint8_t volume_points_number;
    uint8_t panning_points_number;
    uint8_t volume_sustain_point;
    uint8_t volume_loop_start_point;
    uint8_t volume_loop_end_point;
    uint8_t panning_sustain_point;
    uint8_t panning_loop_start_point;
    uint8_t panning_loop_end_point;
    uint8_t volume_type; // bit 0: On; 1: Sustain; 2: Loop
    uint8_t panning_type; // bit 0: On; 1: Sustain; 2: Loop
    uint8_t vibrato_type;
    uint8_t vibrato_sweep;
    uint8_t vibrato_depth;
    uint8_t vibrato_rate;
    uint16_t volume_fadeout;
    uint16_t reserved;
} xm_instrument_header_t;

typedef struct xm_sample_header
{
    uint32_t sample_length;
    uint32_t loop_start;
    uint32_t loop_length;
    uint8_t volume;
    int8_t finetune;
    uint8_t loop_type;
    uint8_t panning;
    int8_t relative_note_number;
    uint8_t reserved;
    char sample_name[22];
} xm_sample_header_t;

typedef struct song_info
{
    xm_main_header_t main_header;
    uint32_t pattern_order_table;
    uint32_t pattern_order_table_size;
    uint32_t first_pattern;
    uint32_t instruments[128];
} xm_song_info_t;

xmresult_e xm_read_main_header(xm_main_header_t *head);
xmresult_e xm_read_pattern_header(uint32_t offset, xm_pattern_header_t *phead);

#endif /*XMPARSER_H_*/