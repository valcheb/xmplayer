#include "xmparser.h"
#include <stdio.h>
#include <stdbool.h>


static void print_main_header(xm_main_header_t *head)
{
    printf("IDtext = %.*s\n", (int)sizeof(head->idtext), head->idtext);
    printf("Module name = %.*s\n", (int)sizeof(head->module_name), head->module_name);
    printf("Symbol = 0x%x\n",head->sym);
    printf("Tracker name = %.*s\n", (int)sizeof(head->tracker_name), head->tracker_name);
    printf("Version = 0x%x\n",head->version);
    printf("Main header size = %d\n",head->header_size);
    printf("Song length in pattern order table = %d\n",head->song_len);
    printf("Restart position = %d\n",head->restart_pos);
    printf("Number of channels = %d\n",head->channels_number);
    printf("Number of patterns = %d\n",head->patterns_number);
    printf("Number of instruments = %d\n",head->instruments_number);
    printf("Frequance table type = %d\n",head->freq_table);
    printf("Default tempo = %d\n",head->default_tempo);
    printf("Default BPM = %d\n",head->default_bpm);
}

/**/

static xmresult_e xm_deinit_song(char *song_name, xm_song_info_t *song_info)
{
    /*
    1) если файл открыт, закрыть.
    2) обнулить оффсеты
    3) как обнулить маин_хедер?
    */
    return XMRESULT_OK;
}

static void print_instruments_names(xm_song_info_t *song)
{
    for (int i = 0; i < song->main_header.instruments_number; i++)
    {
        xm_instrument_header_t instrument_header;
        xm_read_instrument_header(song->instruments[i], &instrument_header);
        printf("Name of instr%d = %.*s, offset = %x, size = %d\n", i,
            (int)(sizeof(instrument_header.main_header.instrument_name)/sizeof(instrument_header.main_header.instrument_name[0])),
            instrument_header.main_header.instrument_name,
            song->instruments[i], instrument_header.main_header.instrument_size);
    }
}

void print_order_table(uint32_t offset, uint32_t size)
{
    for(int i = 0; i < size; i++)
    {
        uint8_t index;
        xm_read_pattern_index_in_order_table(offset, &index);
        printf("Order_%d = pattern_%d\n", i, index);
        offset++;
    }
}

void print_patterns_headers(uint32_t offset, uint16_t size)
{
    for (int i = 0; i < size; i++)
    {
        xm_pattern_header_t pattern_header;
        xm_read_pattern_header(offset,&pattern_header);
        printf("Pattern_%d: hsize = %d, packtype = %d, rows = %d, dsize = %d\n",
            i, pattern_header.header_size, pattern_header.packing_type, pattern_header.rows_number, pattern_header.data_size);
        offset += pattern_header.header_size + pattern_header.data_size;
    }
}

#define READ_VALUE(value) do {\
    if (fs_access_ctx.read(&(value), sizeof(value)) != FSWRESULT_OK)\
        return XMRESULT_ERROR;\
} while(0);

xmresult_e xm_read_note(note_info_t *note, uint32_t *bread)
{
    uint8_t byte;
    uint8_t *ptr = (uint8_t *)note;
    READ_VALUE(byte);
    bool is_packed = byte & 0x80;
    if (is_packed)
    {
        for (int i = 0; i < 5; i++) // 5 == members in note_info_t
            if (byte & (1<<i)) READ_VALUE(ptr[i]);
    }
    else
    {
        note->note = byte;
        for (int i = 1; i < 5; i++)
            READ_VALUE(ptr[i]);
    }

    return XMRESULT_OK;
};

xmresult_e xm_read_row(uint32_t offset, uint8_t channels_number, note_info_t *notes)
{
    uint32_t bread;
    //если читать по строкам, то как следить за размером строки и находить оффсет новой строки?
    fs_access_ctx.seek(offset);
    for (int i = 0; i < channels_number; i++)
        xm_read_note(notes+i,&bread);

    return XMRESULT_OK;
}

void print_pattern_data(uint32_t offset)
{
    xm_pattern_header_t pattern_header;
    uint32_t counter = 0;
    xm_read_pattern_header(offset, &pattern_header);
    offset += pattern_header.header_size;
    //fs_access_ctx.seek(offset);
    for(int i = 0; i < pattern_header.rows_number; i++)
    {
        printf("Row_%d\n",i);
        for(int j = 0; j < 2; j++) // j < channels_number
        {
            note_info_t note = {0};
            xm_read_note(&note);
            counter++;
            printf("|%d %d %d %d %d|",note.note,note.instrument,
            note.volume_column_byte,note.effect_type,note.effect_parameter);
        }
        printf("\n");
    }
}

int main()
{
    printf("<<<Test xm.>>>\n");

    xm_song_info_t song_info;
    //char song_name[] = "/home/vvman/workspace/stm32/xmplayer/test/song.xm";
    char song_name[] = "/home/vvman/workspace/stm32/xmplayer/test/song2.xm";
    //char song_name[] = "/home/vvman/workspace/stm32/xmplayer/test/song3.xm";
    //char song_name[] = "/home/vvman/workspace/stm32/xmplayer/test/mg_farou.xm";
    //char song_name[] = "/home/vvman/workspace/stm32/xmplayer/test/the_difference.xm";
    //char song_name[] = "/home/vvman/workspace/stm32/xmplayer/test/rockmonitor.xm";

    if (xm_init_song(song_name, &song_info) != XMRESULT_OK)
    {
        printf("Error song init\n");
        return 1;
    }
    printf("<<<Main header>>>\n");
    print_main_header(&song_info.main_header);
    /*printf("<<<Ins names>>>\n");
    print_instruments_names(&song_info);
    printf("<<Pattern order table>>\n");
    print_order_table(song_info.pattern_order_table,song_info.main_header.song_len);
    */printf("<<Pattern header info>>\n");
    print_patterns_headers(song_info.first_pattern,song_info.main_header.patterns_number);
    printf("<<<Notes>>>\n");
    print_pattern_data(song_info.first_pattern);

    return 0;
}