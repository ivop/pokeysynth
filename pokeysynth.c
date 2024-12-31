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

typedef enum {
    POKEYSYNTH_MIDI_IN,
    POKEYSYNTH_AUDIO_OUT,
    POKEYSYNTH_CONTROL_CHANNELS,
    POKEYSYNTH_CONTROL_MONO_ARP1,
    POKEYSYNTH_CONTROL_MONO_ARP2,
    POKEYSYNTH_CONTROL_MONO_ARP3,
    POKEYSYNTH_CONTROL_MONO_ARP4,
    POKEYSYNTH_CONTROL_ARP_SPEED1,
    POKEYSYNTH_CONTROL_ARP_SPEED2,
    POKEYSYNTH_CONTROL_ARP_SPEED3,
    POKEYSYNTH_CONTROL_ARP_SPEED4,
} PortIndex;

typedef struct {
    // ports
    const LV2_Atom_Sequence *midi_in;
    float *audio_out;
    float *control_channels;        // react to MIDI chs 0-3,4-7,8-11,12-15
    float *control_mono_arp[4];     // pokey channels modes mono/auto-arp
    float *control_arp_speed[4];    // pokey channels arp speeds

    // features
    LV2_URID_Map *map;
    LV2_Log_Logger logger;

    struct {
        LV2_URID midi_MidiEvent;
    } uris;

    struct mzpokey_context *mzp;
} PokeySynth;

// ****************************************************************************

#define INSTRUMENT_LENGTH 64

enum channels_type {
    CHANNELS_1CH,               // 1 or 2 or 3 or 4, 8-bit divider
    CHANNELS_LINKED,            // 1,2 or 3,4, 16-bit divider
    CHANNELS_FILTERED,          // 1,3 or 2,4, 8-bit divider, 8-bit filtered
    CHANNELS_LINKED_FILTERED,   // 1,2,3,4, 16-bit divider, 16-bit filtered
    CHANNELS_HIFRQ              // 1 or 3, 8-bit divider, full pokey frequency
};

enum distortions {
    DIST_1CH_PURE,
    DIST_1CH_0x80,
    DIST_1CH_BUZZY_BASS,
    DIST_1CH_GRITTY_BASS
};

enum note_tables {
    NOTETBL_AUTOMATIC,
    NOTETBL_CALCULATE,
    NOTETBL_BUZZY_BASS,
    NOTETBL_GRITTY_BASS
};

enum note_types {
    NOTE,               // frequency depends on MIDI Note
    NOTE_PLUS_NOTE,     // same as note, but with +/- whole semitones
    NOTE_PLUS_CENTS,    // same as note, but offset +/- by x cents
    FIXED_FREQ          // fixed frequency, e.g. for drum sounds
};

struct pokey_instrument {
    char name[32];

    uint8_t num_pokey_channels;             // 1, 2, or 4

    bool base_clock;                        // 15kHz or 64kHz

    uint8_t volume[INSTRUMENT_LENGTH];
    uint8_t distortion[INSTRUMENT_LENGTH];
    uint8_t end;
    uint8_t loop;

    uint8_t types[INSTRUMENT_LENGTH];       // note_types, 0 has no value
    uint16_t values[INSTRUMENT_LENGTH];     // 8/16-bit value for types >= 1
    uint8_t types_end;
    uint8_t types_loop;
    uint8_t types_speed;
};

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
    case POKEYSYNTH_CONTROL_CHANNELS:
        self->control_channels = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP1:
        self->control_mono_arp[0] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP2:
        self->control_mono_arp[1] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP3:
        self->control_mono_arp[2] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP4:
        self->control_mono_arp[3] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED1:
        self->control_arp_speed[0] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED2:
        self->control_arp_speed[1] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED3:
        self->control_arp_speed[2] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED4:
        self->control_arp_speed[3] = (float *) data;
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

    LV2_ATOM_SEQUENCE_FOREACH (self->midi_in, ev) {
        if (ev->body.type == self->uris.midi_MidiEvent) {
            const uint8_t *const msg = (const uint8_t *) (ev + 1);
            const uint8_t type = lv2_midi_message_type(msg);
            const uint8_t channel = msg[0] & 0x0f;

            switch (type) {
            case LV2_MIDI_MSG_NOTE_ON:
                printf("note on, channel %d\n", channel);
                break;
            case LV2_MIDI_MSG_NOTE_OFF:
                printf("note off, channel %d\n", channel);
                break;
            case LV2_MIDI_MSG_PGM_CHANGE:
                printf("pgm change, channel %d, pgm %d\n", channel, msg[1]);
                break;
            }
        }
    }
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
