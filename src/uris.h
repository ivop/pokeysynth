#pragma once
#include "lv2.h"

#define POKEYSYNTH_URI "https://github.com/ivop/pokeysynth"

extern struct uris_s {
    LV2_URID midi_MidiEvent;
    LV2_URID atom_Int;
    LV2_URID atom_eventTransfer;

    LV2_URID instrument_data;
    LV2_URID program_number;
    LV2_URID program_data;
    LV2_URID request_program;

    LV2_URID filename_object;
    LV2_URID bank_filename;
    LV2_URID sapr_filename;

    LV2_URID reload_bank;
    LV2_URID request_filenames;

    LV2_URID enable_midi_sapr;
    LV2_URID start_sapr;
    LV2_URID stop_sapr;

} uris;

void init_uris(LV2_URID_Map *map);
