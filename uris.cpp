#include "lv2.h"
#include "uris.h"

struct uris_s uris;

void init_uris(LV2_URID_Map *map) {
    uris.midi_MidiEvent     = map->map(map->handle, LV2_MIDI__MidiEvent);
    uris.atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);
    uris.atom_Int           = map->map(map->handle, LV2_ATOM__Int);

    uris.instrument_data    = map->map(map->handle,
                                       POKEYSYNTH_URI"#instrument_data");
    uris.program_number     = map->map(map->handle,
                                       POKEYSYNTH_URI"#program_number");
    uris.program_data       = map->map(map->handle,
                                       POKEYSYNTH_URI"#program_data");
    uris.request_program    = map->map(map->handle,
                                       POKEYSYNTH_URI"#request_program");

    uris.filename_object    = map->map(map->handle,
                                       POKEYSYNTH_URI"#filename_object");
    uris.bank_filename      = map->map(map->handle,
                                       POKEYSYNTH_URI"#bank_filename");

    uris.request_bank_filename = map->map(map->handle,
                                      POKEYSYNTH_URI"#request_bank_filename");
    uris.reload_bank        = map->map(map->handle,
                                       POKEYSYNTH_URI"#reload_bank");
}
