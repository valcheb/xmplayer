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
    (*bread)++;
    bool is_packed = byte & 0x80;
    if (is_packed)
    {
        for (int i = 0; i < 5; i++) // 5 == members in note_info_t
        {
            if (byte & (1<<i))
            {
                READ_VALUE(ptr[i]);
                (*bread)++;
            }
        }
    }
    else
    {
        note->note = byte;
        for (int i = 1; i < 5; i++)
        {
            READ_VALUE(ptr[i]);
            (*bread)++;
        }
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
    for (int i = 0; i < pattern_header.rows_number; i++)
    {
        printf("Row_%d\n",i);
        for (int j = 0; j < 8; j++) // j < channels_number
        {
            note_info_t note = {0};
            xm_read_note(&note, NULL);
            counter++;
            printf("|%d %d %d %d %d|",note.note,note.instrument,
            note.volume_column_byte,note.effect_type,note.effect_parameter);
        }
        printf("\n");
    }
}

//flags
static bool is_data_for_calc = true;
static bool is_new_pattern_need = true;
static bool is_new_row_need = true;

static void read_new_pattern(xm_pattern_header_t *pattern_header, uint16_t *order_table_pos, uint32_t *pattern_offset, xm_song_info_t *song)
{
    uint32_t order_table_offset = song->pattern_order_table + (uint32_t)*order_table_pos;

    uint16_t pattern_index;
    xm_read_pattern_index_in_order_table(order_table_offset, &pattern_index);
    #if 1
        printf("<<<Order_table_%d = pattern_%d>>>\n", *order_table_pos, pattern_index);
    #endif
    (*order_table_pos)++; //control limit of order table

    //uint32_t pattern_offset;
    xm_read_pattern_offset(pattern_index, song->first_pattern, pattern_offset);
    xm_read_pattern_header(*pattern_offset, pattern_header);
    #if 1
        printf("    <<<Pattern %d header info>>>\n", pattern_index);
        printf("    hsize = %d, packtype = %d, rows = %d, dsize = %d\n",
               pattern_header->header_size, pattern_header->packing_type,
               pattern_header->rows_number, pattern_header->data_size);
    #endif
}

typedef struct
{
    note_info_t notes[32]; //TODO define max
    uint32_t    bread;
} row_info_t;

static void read_new_row(row_info_t *note_row, uint32_t start_offset, uint16_t size)
{
    //fs_access_ctx.seek(); TODO после чтения инструмента теряется оффсет, откуда читать новую строку
    fs_access_ctx.seek(start_offset);
    for (int chan_c = 0; chan_c < size; chan_c++)
    {
        xm_read_note(&note_row->notes[chan_c], &note_row->bread);
    }
}

static void clear_row(row_info_t *note_row, uint16_t size)
{
    for (int chan_c = 0; chan_c < size; chan_c++)
    {
        note_row->notes[chan_c].note = 0;
        note_row->notes[chan_c].instrument = 0;
        note_row->notes[chan_c].volume_column_byte = 0;
        note_row->notes[chan_c].effect_type = 0;
        note_row->notes[chan_c].effect_parameter = 0;
        note_row->bread = 0;
    }
}

/*
TODO рассуждения о частоте и времени
есть две таблицы частот.
как работает amiga - не понятно.
по линейной таблице вычисления представляют собой

Period = 7680 - (Note * 64) - (FineTune / 2)
Frequency = 8363 * 2^((4608 - Period) / 768)

где Note = PatternNote + RelativeTone

например
C-4
Note = 48 + 0 = 48;
Period = 7680 - 3072 - 0 = 4608
Frequency = 8363

Под Frequency здесь понимается (скорее всего)
скорость воспроизведения отчетов семпла.

Длина семпла влияет на результирующую частоту звука - чем короче семпл, тем выше звук
при прочих равных условиях.

Рассмотрим следующий момент.
Пусть BPM = 125, Speed = 25, Rate = 48000 (rate == частота дискретизации)
Количетсво строк = 10
Тогда tick_time (ms) = 2500 / BPM = 20 (ms)
Row_time = speed * tick_time = 500 (ms)
Pattern lenght = row_number * row_time = 5 (s)

Допустим нота C-4 -> частота отчетов 8363 о/с
За эти 5 секунд с такой частотой полностью воспроизведется семпл длиной 5*8363 = 41815

Но сколько отчетов будет для rate?
5 * 48000 = 240000
Как совместить Frequancy и rate?
По методу синтеза волновыми таблицами 
Phase_increment = wave_lenght * freq_note / rate
Предполагаем, что Frequancy = wave_lenght * freq_note
получим, что 
phase_increment = Frequancy / rate = 0,174229167
Данное утверждение нужно проверить

TODO рассуждения о инструменте
инструменты нумеруются от 1
Что значит, если мы считаем 0?
Предполагаю, что 0 - это не изменение инструмента.
Т.е. при чтении 0 нужно продолжать использовать выбранный ранее инструмент.
Так, на момент старта все инструменты должны быть инициированы, но замучены (note key off)

*/

static void row_process(row_info_t *note_row, row_info_t *old_note_row, xm_song_info_t *song, uint32_t *buffer, uint32_t buffer_size)
{
    
#if 0
    printf("        ");
    for (int chan_c = 0; chan_c < song->main_header.channels_number; chan_c++)
    {
        printf("|%d %d %d %d %d|", note_row->notes[chan_c].note,
               note_row->notes[chan_c].instrument,
               note_row->notes[chan_c].volume_column_byte,
               note_row->notes[chan_c].effect_type,
               note_row->notes[chan_c].effect_parameter);
    }
    printf("\n");
#endif

    /*
        clear_calculated_buffer
        for all channels
            calculate_freq_of_current_channel
            get_instrument_sample
            for all samples in tick (25 * rate / BPM / 10)
                calculated[i] += cook_data_with_effect_type()
        normalize calculated by headroom
    */

//TODO проблема - читаем ноту - и что там? есть там нота или нет? повторение ли это предыдущего или пустота?
    for (int chan_c = 0; chan_c < song->main_header.channels_number; chan_c++)
    {
        xm_instrument_header_t instrument;
        note_info_t note_info = note_row->notes[chan_c];
        uint8_t instrument_pos = note_info.instrument;
        
        if (instrument_pos > 0)
        {
            uint32_t instrument_offset = song->instruments[instrument_pos - 1];
            
            xm_read_instrument_header(instrument_offset, &instrument);

            //TODO возможно, если note_info.instrument == 0, это значит отсутствие звука или предыдущий инструмент
            //TODO на старте old_row неплохо бы инициализировать note off

            #if 1
            printf("        ");
            printf("Name of instr%d = %.*s, offset = %d, size = %d ", instrument_pos,
                (int)(sizeof(instrument.main_header.instrument_name)/sizeof(instrument.main_header.instrument_name[0])),
                instrument.main_header.instrument_name,
                song->instruments[instrument_pos - 1], instrument.main_header.instrument_size);
            printf("\n");
            #endif
        }
        /*
        else
        {
            //use instrument from previous row ???
        }
        */

        //uint8_t sample_number_for_note = instrument.extra_header.sample_number_for_notes[note_info.note];
        //uint32_t sample_offset = instrument_offset;
    }
}

static void calculate_buffer3(xm_song_info_t *song, uint32_t *buffer, uint32_t buffer_size)
{
    static xm_pattern_header_t pattern_header;
    static uint16_t order_table_pos = 0;
    static uint32_t pattern_offset = 0;
    static uint32_t row_counter = 0;
    static uint32_t row_pos = 0;
    static row_info_t new_row;
    static row_info_t old_row;

    if (order_table_pos >= song->main_header.song_len && row_counter == 0) //TODO что же является признаком
    {
        is_data_for_calc = false;
        return ;
    }

    if (is_new_pattern_need == true)
    {
        read_new_pattern(&pattern_header, &order_table_pos, &pattern_offset, song);
        is_new_pattern_need = false;
    }

    if (is_new_row_need == true)
    {
        //prepare_new_row(new_row, offset, size);
        //prepare_channels(new_row, channels);
        /*
        channel: frequance
                 sample_offset //for dynamic read if sample length larger than size of buffer
                 sample_buffer[buffer_size]
                 sample_full_size
                 current_sample_phase //index in sample_buffer
                 effect 

        */
        old_row = new_row;
        clear_row(&new_row, song->main_header.channels_number);
        read_new_row(&new_row, pattern_offset + pattern_header.header_size + row_pos, song->main_header.channels_number);
        row_pos += new_row.bread;
        is_new_row_need = false;
    }

    if (is_new_pattern_need == false && is_new_row_need == false)
    {
        row_process(&new_row, &old_row, song, buffer, buffer_size); //process notes of row
        row_counter++;
        is_new_row_need = true;

        if (row_counter >= pattern_header.rows_number)
        {
            row_counter = 0;
            row_pos = 0;
            is_new_pattern_need = true;
        }
    }
}

#define RATE 48000
static void test_play_3(xm_song_info_t *song)
{
    uint32_t calc_size = 25 * RATE / song->main_header.default_bpm / 10;
    uint32_t calculated[calc_size]; //25 * rate / BPM / 10
    while(is_data_for_calc == true)
    {
        calculate_buffer3(song, calculated, calc_size);
    }
}

int main()
{
    printf("<<<Test xm.>>>\n");

    xm_song_info_t song_info;
    //char song_name[] = "/home/vvman/workspace/stm32/xmplayer/test/song.xm";
    //char song_name[] = "/home/vvman/workspace/stm32/xmplayer/test/song2.xm";
    //char song_name[] = "/home/vvman/workspace/stm32/xmplayer/test/song3.xm";
    //char song_name[] = "/home/vvman/workspace/stm32/xmplayer/test/mg_farou.xm";
    //char song_name[] = "/home/vvman/workspace/stm32/xmplayer/test/the_difference.xm";
    //char song_name[] = "/home/vvman/workspace/stm32/xmplayer/test/rockmonitor.xm";

    char song_name[] = "/home/valentin/workspace/xmplayer/test/1.xm";

    if (xm_init_song(song_name, &song_info) != XMRESULT_OK)
    {
        printf("Error song init\n");
        return 1;
    }
    printf("<<<Main header>>>\n");
    print_main_header(&song_info.main_header);
    printf("<<<Ins names>>>\n");
    print_instruments_names(&song_info);
    /*printf("<<Pattern order table>>\n");
    print_order_table(song_info.pattern_order_table,song_info.main_header.song_len);
    printf("<<Pattern header info>>\n");
    print_patterns_headers(song_info.first_pattern, song_info.main_header.patterns_number);
    printf("<<<Notes>>>\n");
    print_pattern_data(song_info.first_pattern);*/
    

   test_play_3(&song_info);

    return 0;
}