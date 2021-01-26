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

typedef struct
{
    xm_pattern_header_t pattern_header;
    uint32_t            pattern_offset;
} pattern_data_t;

#define MAX_SUPPORT_CHANNELS 32
typedef struct
{
    note_info_t notes[MAX_SUPPORT_CHANNELS];
    uint32_t    bytes_in_row;
} row_info_t;

typedef note_info_t channel_state_t;

typedef struct
{
    xm_instrument_header_t instrument_header;
    xm_sample_header_t     sample_header;
    uint32_t               sample_start_offset;
    uint32_t               volume;
    uint32_t               frequency;
} channel_info_t;

static void read_new_pattern(pattern_data_t *pattern_data, uint16_t order_table_pos, xm_song_info_t *song)
{
    uint8_t pattern_index;

    xm_read_pattern_index_in_order_table(song->pattern_order_table + order_table_pos,
                                         &pattern_index);
    xm_read_pattern_offset(pattern_index, song->first_pattern, &pattern_data->pattern_offset);
    xm_read_pattern_header(pattern_data->pattern_offset, &pattern_data->pattern_header);

    #if 1
        printf("<<<Order_table_%d = pattern_%d>>>\n", order_table_pos, pattern_index);
        printf("    <<<Pattern %d header info>>>\n", pattern_index);
        printf("    hsize = %d, packtype = %d, rows = %d, dsize = %d\n",
               pattern_data->pattern_header.header_size, pattern_data->pattern_header.packing_type,
               pattern_data->pattern_header.rows_number, pattern_data->pattern_header.data_size);
    #endif
}

static void clear_row(row_info_t *note_row, uint16_t size)
{
    for (int chan_c = 0; chan_c < size; chan_c++)
    {
        note_row->notes[chan_c].note               = 0;
        note_row->notes[chan_c].instrument         = 0;
        note_row->notes[chan_c].volume_column_byte = 0;
        note_row->notes[chan_c].effect_type        = 0;
        note_row->notes[chan_c].effect_parameter   = 0;
        note_row->bytes_in_row                     = 0;
    }
}

static void read_new_row(row_info_t *note_row, uint32_t row_offset, uint16_t chan_number)
{
    fs_access_ctx.seek(row_offset);
    for (int chan_c = 0; chan_c < chan_number; chan_c++)
    {
        xm_read_note(&note_row->notes[chan_c], &note_row->bytes_in_row);
    }

    #if 1
        printf("        ");
        for (int chan_c = 0; chan_c < chan_number; chan_c++)
        {
            printf("|%d %d %d %d %d|", note_row->notes[chan_c].note,
                   note_row->notes[chan_c].instrument,
                   note_row->notes[chan_c].volume_column_byte,
                   note_row->notes[chan_c].effect_type,
                   note_row->notes[chan_c].effect_parameter);
        }
        printf("\n");
    #endif
}

static void set_mute_channels(channel_state_t *channels_state, uint16_t chan_num)
{
    for (int chan_c = 0; chan_c < chan_num; chan_c++)
    {
        channels_state[chan_c].note = 97; // key off
        channels_state[chan_c].instrument = 0;
        channels_state[chan_c].volume_column_byte = 64; // max volume
        channels_state[chan_c].effect_type = 0;
        channels_state[chan_c].effect_parameter = 0;
    }
}

static void prepare_states(row_info_t *note_row, channel_state_t *current_states, xm_song_info_t *song)
{
    for (int chan_c = 0; chan_c < song->main_header.channels_number; chan_c++)
    {
        note_info_t new_note = note_row->notes[chan_c];
        channel_state_t *current_state = &current_states[chan_c];

        if (new_note.note != 0)
        {
            current_state->note = new_note.note;
            //mb optimaze: if note is same - dont calc freq, but phase == 0
        }

        if (new_note.instrument == 0)
        {
            //nothing changes
        }
        else if (new_note.instrument > song->main_header.instruments_number)
        {
            //mute
            current_state->note = 97;
        }
        else if (new_note.instrument != current_state->instrument)
        {
            //get new instr
            current_state->instrument = new_note.instrument;
        }

        //TODO volume and effects state
    }
}

static uint32_t calculate_frequency()
{

}

static void prepare_instrument_and_sample(channel_state_t *current_state, channel_info_t *channel, xm_song_info_t *song)
{
    //read instrument
    uint32_t instrument_offset = song->instruments[current_state->instrument - 1];
    xm_read_instrument_header(instrument_offset, &channel->instrument_header);

    xm_instrument_main_header_t *instr_main_header = &channel->instrument_header.main_header;
    xm_instrument_extra_header_t *instr_extra_header = &channel->instrument_header.extra_header;

    #if 1
        printf("        ");
        printf("Name of instr%d = %.*s, offset = %d, size = %d ", current_state->instrument,
            (int)(sizeof(instr_main_header->instrument_name) / sizeof(instr_main_header->instrument_name[0])),
            instr_main_header->instrument_name,
            song->instruments[current_state->instrument - 1], instr_main_header->instrument_size);
        printf("\n");
    #endif

    //read sample
    uint8_t sample_number_for_note = instr_extra_header->sample_number_for_notes[current_state->note];
    channel->sample_start_offset = instrument_offset + instr_main_header->instrument_size; //here sample headers start

    for (int sample_c = 0; sample_c <= sample_number_for_note; sample_c++)
    {
        xm_read_sample_header(channel->sample_start_offset, &channel->sample_header);
        channel->sample_start_offset += instr_extra_header->sample_header_size; // move to next sample header
    }
    //TODO debug this for instruments with more than one sample

    #if 1
        printf("        ");
        printf("Sample number = %d ", sample_number_for_note);
        printf("sample name = %.*s", (int)(sizeof(channel->sample_header.sample_name) / sizeof(channel->sample_header.sample_name[0])),
               channel->sample_header.sample_name);
        printf("\n");
    #endif
}

static void prepare_channels(channel_state_t *current_states, channel_info_t *channels, xm_song_info_t *song)
{
    #if 1
        printf("state:  ");
        for (int chan_c = 0; chan_c < song->main_header.channels_number; chan_c++)
        {
            channel_state_t *current_state = &current_states[chan_c];
            printf("|%d %d %d   |", current_state->note, current_state->instrument, current_state->volume_column_byte);
        }
        printf("\n");
    #endif

    for (int chan_c = 0; chan_c < song->main_header.channels_number; chan_c++)
    {
        channel_state_t *current_state = &current_states[chan_c];
        channel_info_t  *channel = &channels[chan_c];

        if (current_state->instrument > 0)
        {
            prepare_instrument_and_sample(current_state, channel, song);
        }
        else
        {
            //TODO what to do with 0 instrument?
        }

        //uint16_t note = current_state->note + channel->sample_header.relative_note_number;
        //float period = 7680 - (note * 64) - (channel->sample_header.finetune / 2);
        //channel->frequency = 8363 * 2^((4608 - period) / 768);

        //calc start freq //start because there are effects, that change freq or volume
        /*
            PatternNote ranges in 1..96
            1   = C-0
            96  = B-7
            97  = Key Off (special 'note')

            FineTune ranges between -128..+127
            -128 = -1 halftone,
            +127 = +127/128 halftones

            RelativeTone is then in range -96..95, so in
            effect a note with RelativeTone value 0 is the
            note itself.

            Formula for calculating the real, final note value is:

            >> RealNote = PatternNote + RelativeTone;

            Linear frequency table
            ======================

            > Period = 7680 - (Note * 64) - (FineTune / 2)
            > Frequency = 8363 * 2^((4608 - Period) / 768)

            NOTICE: The values are reasonable. With note 119
                and finetune of +128, we get:

                > x = 7680 - (119*64) - (128/2)
                > x = 64 - 64
                > x = 0
        */

        //calc start volume
        /*
            The volume formula
            ==================

            FinalVol = (FadeOutVol/65536)*(EnvelopeVol/64)*(GlobalVol/64)*(Vol/64)*Scale;

            The panning formula
            ===================

            FinalPan = Pan + ( (EnvelopePan-32)*(128-Abs(Pan-128)) / 32 );

            NOTICE: The panning envelope values range
                    from 0...64, not -128...+127


            Fadeout
            =======
            The FadeOutVol is originally 65536 (after the note has been triggered) and
            is decremented by "Instrument.Fadeout" each tick after note is released
            (with KeyOff Effect or with KeyOff Note)

            NOTICE: Fadeout is not processed if Volume Envelope is DISABLED.
            (This means that the FadeOutVol stays at 65536)
        */
    }
}

static void row_process(row_info_t *current_note_row, row_info_t *old_note_row, uint16_t chan_num)
{
    /*
        clear_calculated_buffer
        for all channels
            calculate_freq_of_current_channel
            get_instrument_sample
            for all samples in tick (25 * rate / BPM / 10)
                calculated[i] += cook_data_with_effect_type()
        normalize calculated by headroom
        save in memory
    */

    for (int chan_c = 0; chan_c < chan_num; chan_c++)
    {
        note_info_t current_note = current_note_row->notes[chan_c];
        note_info_t old_note = old_note_row->notes[chan_c];

        if (current_note.note != old_note.note)
        {
            //get_instr
            //get_sample, clear phases
            //get_note_freq
            //calculate_increment
        }
        else if (current_note.instrument != current_note.instrument)
        {
            //get_instr
            //get_sample, clear phases
            //calculate_increment
        }

        //calculate_data(increment, sample, volume, effect, effect_type);


        /*xm_instrument_header_t instrument;
        note_info_t current_note = current_note_row->notes[chan_c];
        uint8_t instrument_pos = current_note.instrument;

        if (instrument_pos > 0)
        {
            read_new_instrument();
        }
        else
        {

        }*/
    }
}

static void test_play(xm_song_info_t *song)
{
    pattern_data_t current_pattern;
    uint16_t order_table_pos = 0;
    uint16_t chan_num = song->main_header.channels_number;
    row_info_t new_row;
    channel_info_t channels[chan_num];
    channel_state_t channels_states[chan_num];
    set_mute_channels(channels_states, chan_num); //set default volume, key off

    while (order_table_pos < song->main_header.song_len)
    {
        if (order_table_pos == 41){read_new_pattern(&current_pattern, order_table_pos, song);

        uint16_t row_counter      = 0;
        uint32_t row_pos          = 0;
        uint16_t max_row          = current_pattern.pattern_header.rows_number;
        uint32_t start_row_offset = current_pattern.pattern_offset + current_pattern.pattern_header.header_size;

        while (row_counter < max_row)
        {
            clear_row(&new_row, chan_num);
            read_new_row(&new_row, start_row_offset + row_pos, chan_num);
            prepare_states(&new_row, channels_states, song);
            prepare_channels(channels_states, channels, song);

            row_counter++;
            row_pos += new_row.bytes_in_row;
        }}

        order_table_pos++;
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


   test_play(&song_info);

    return 0;
}