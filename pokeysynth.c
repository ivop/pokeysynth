#include "lv2/atom/atom.h"
#include "lv2/atom/util.h"
#include "lv2/core/lv2.h"
#include "lv2/core/lv2_util.h"
#include "lv2/log/log.h"
#include "lv2/log/logger.h"
#include "lv2/midi/midi.h"
#include "lv2/urid/urid.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mzpokey.h"

#define POKEYSYNTH_URI "https://github.com/ivop/pokeysynth"

typedef enum { POKEYSYNTH_MIDI_IN, POKEYSYNTH_AUDIO_OUT } PortIndex;

typedef struct {
    // ports
    const LV2_Atom_Sequence *midi_in;
    float *audio_out;

    // features
    LV2_URID_Map *map;
    LV2_Log_Logger logger;

    struct {
        LV2_URID midi_MidiEvent;
    } uris;

    struct mzpokey_context *mzp;
} PokeySynth;

// ****************************************************************************

static LV2_Handle instantiate(const LV2_Descriptor *descriptor,
                              double rate,
                              const char *bundle_path,
                              const LV2_Feature *const *features) {
    PokeySynth *self = calloc(1, sizeof(PokeySynth));
    if (!self) return NULL;

    const char *missing = lv2_features_query(
            features,
            LV2_LOG__log,  &self->logger.log, false,
            LV2_URID__map, &self->map,        true,
            NULL);

    lv2_log_logger_set_map(&self->logger, self->map);
    if (missing) {
        lv2_log_error(&self->logger, "Missing feature <%s>\n", missing);
        free(self);
        return NULL;
    }

    self->uris.midi_MidiEvent =
        self->map->map(self->map->handle, LV2_MIDI__MidiEvent);

    self->mzp = mzpokey_create(1773447, rate, 1, 0);
    if (!self->mzp) {
        free(self);
        return NULL;
    }

    mzpokey_write_register(self->mzp, SKCTL, 3, 0);
    mzpokey_write_register(self->mzp, AUDC1, 0xaf, 0);
    mzpokey_write_register(self->mzp, AUDF1, 0x81, 0);

    return (LV2_Handle) self;
}

// ****************************************************************************

static void connect_port(LV2_Handle instance, uint32_t port, void *data) {
    PokeySynth *self = (PokeySynth *) instance;

    switch ((PortIndex) port) {
    case POKEYSYNTH_MIDI_IN:
        self->midi_in = (const LV2_Atom_Sequence *) data;
        break;
    case POKEYSYNTH_AUDIO_OUT:
        self->audio_out = (float *) data;
        break;
    }
}

// ****************************************************************************

static void activate(LV2_Handle instance) {
}

// ****************************************************************************

static void run(LV2_Handle instance, uint32_t sample_count) {
    PokeySynth *self = (PokeySynth *) instance;

    mzpokey_process_float(self->mzp, self->audio_out, sample_count);
}

// ****************************************************************************

static void deactivate(LV2_Handle instance) {
}

// ****************************************************************************

static void cleanup(LV2_Handle instance) {
    free(instance);
}

// ****************************************************************************

static const void *extension_data(const char *URI) {
    return NULL;
}

// ****************************************************************************

static const LV2_Descriptor descriptor = {
    POKEYSYNTH_URI,
    instantiate,
    connect_port,
    activate,
    run,
    deactivate,
    cleanup,
    extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor(uint32_t index) {
    return !index ? &descriptor : NULL;
}
