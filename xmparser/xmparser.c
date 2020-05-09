#include "xmparser.h"

static xm_song_info_t song_info;

xmresult_e xm_fill_main_header(xm_main_header_t *head)
{
    if (head == NULL)
        return XMRESULT_ERROR;

    uint16_t bread;
    if (fs_access_ctx.seek(0) != FSWRESULT_OK)
        return XMRESULT_ERROR;
    
    fs_access_ctx.read(head->idtext,sizeof(head->idtext)/sizeof(head->idtext[0])-1,&bread);
    head->idtext[sizeof(head->idtext)/sizeof(head->idtext[0])-1] = '\0';

    fs_access_ctx.read(head->module_name,sizeof(head->module_name)/sizeof(head->module_name[0])-1,&bread);
    head->module_name[sizeof(head->module_name)/sizeof(head->module_name[0])-1] = '\0';

    fs_access_ctx.read(&(head->sym),sizeof(head->sym),&bread);

    fs_access_ctx.read(head->tracker_name,sizeof(head->tracker_name)/sizeof(head->tracker_name[0])-1,&bread);
    head->tracker_name[sizeof(head->tracker_name)/sizeof(head->tracker_name[0])-1] = '\0';

    fs_access_ctx.read(&(head->version),sizeof(head->version),&bread);
    fs_access_ctx.read(&(head->header_size),sizeof(head->header_size),&bread);
    fs_access_ctx.read(&(head->song_len),sizeof(head->song_len),&bread);
    fs_access_ctx.read(&(head->restart_pos),sizeof(head->restart_pos),&bread);
    fs_access_ctx.read(&(head->channels_number),sizeof(head->channels_number),&bread);
    fs_access_ctx.read(&(head->patterns_number),sizeof(head->patterns_number),&bread);
    fs_access_ctx.read(&(head->instruments_number),sizeof(head->instruments_number),&bread);
    fs_access_ctx.read(&(head->freq_table),sizeof(head->freq_table),&bread);
    fs_access_ctx.read(&(head->default_tempo),sizeof(head->default_tempo),&bread);
    fs_access_ctx.read(&(head->default_bpm),sizeof(head->default_bpm),&bread);

    return XMRESULT_OK;
}