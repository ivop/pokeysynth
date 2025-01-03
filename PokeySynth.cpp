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

#include <map>

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
    POKEYSYNTH_CONTROL_UPDATE_FREQ,
} PortIndex;

enum pokey_update_frequency {
    UPDATE_50HZ,
    UPDATE_100HZ,
    UPDATE_150HZ,
    UPDATE_200HZ,
    UPDATE_INSTANT          // no SAP-R recording
};

// ****************************************************************************

struct note {
    uint64_t time;
    uint8_t velocity;
    uint8_t program;
};

class PokeySynth {
public:
    PokeySynth(const double sample_rate, const char *bundle_path, const LV2_Feature *const *features);
    void connect_port(const uint32_t port, void *data);
    void run(const uint32_t sample_count);

private:
    // ports
    const LV2_Atom_Sequence *midi_in;
    float *audio_out;
    float *control_channels;        // react to MIDI chs 0-3,4-7,8-11,12-15
    float *control_mono_arp[4];     // pokey channels modes mono/auto-arp
    float *control_arp_speed[4];    // pokey channels arp speeds
    float *control_update_freq;     // update frequency 50/100/150/200/instant

    // features
    LV2_URID_Map *map;
    LV2_Log_Logger logger;

    struct {
        LV2_URID midi_MidiEvent;
    } uris;

    int sample_rate;
    int pokey_rate;
    uint64_t current_timestamp;

    struct mzpokey_context *mzp;

    // synth
    std::map<uint8_t,struct note> notes_on[4];
    uint8_t programs[4] = {};

    int map_midi_to_pokey_channel(int channel);
};

// ****************************************************************************

PokeySynth::PokeySynth(const double sample_rate,
                       const char *bundle_path,
                       const LV2_Feature *const *features) :
    midi_in(nullptr),
    audio_out(nullptr),
    control_channels(nullptr),
    control_mono_arp{nullptr},
    control_arp_speed{nullptr},
    control_update_freq(nullptr),
    current_timestamp(0),
    mzp(nullptr) {

    const char *missing = lv2_features_query(
            features,
            LV2_LOG__log,  &logger.log, false,
            LV2_URID__map, &map,        true,
            NULL);

    lv2_log_logger_set_map(&logger, map);
    if (missing) {
        lv2_log_error(&logger, "Missing feature <%s>\n", missing);
        throw;
    }

    uris.midi_MidiEvent = map->map(map->handle, LV2_MIDI__MidiEvent);

    pokey_rate = 1773447;
    this->sample_rate = sample_rate;

    mzp = mzpokey_create(pokey_rate, sample_rate, 1, 0);
    if (!mzp) throw;

    mzpokey_write_register(mzp, SKCTL, 3, 0);
    mzpokey_write_register(mzp, AUDC1, 0xaf, 0);
    mzpokey_write_register(mzp, AUDF1, 0x81, 0);
}

// ****************************************************************************

void PokeySynth::connect_port(uint32_t port, void *data) {
    switch ((PortIndex) port) {
    case POKEYSYNTH_MIDI_IN:
        midi_in = (const LV2_Atom_Sequence *) data;
        break;
    case POKEYSYNTH_AUDIO_OUT:
        audio_out = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_CHANNELS:
        control_channels = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP1:
        control_mono_arp[0] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP2:
        control_mono_arp[1] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP3:
        control_mono_arp[2] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP4:
        control_mono_arp[3] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED1:
        control_arp_speed[0] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED2:
        control_arp_speed[1] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED3:
        control_arp_speed[2] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED4:
        control_arp_speed[3] = (float *) data;
        break;
    }
}

// ****************************************************************************

int PokeySynth::map_midi_to_pokey_channel(int channel) {
    unsigned int range = (int) *control_channels;

    if (range > 3) range = 3;

    int ranges[5]  = { 0, 4, 8, 12, 16 };

    if (channel >= ranges[range] && channel < ranges[range+1])
        return channel & 3;
    else
        return -1;
}

// ****************************************************************************

void PokeySynth::run(uint32_t sample_count) {
    mzpokey_process_float(mzp, audio_out, sample_count);

    LV2_ATOM_SEQUENCE_FOREACH (midi_in, ev) {
        if (ev->body.type == uris.midi_MidiEvent) {
            const uint8_t *const msg = (const uint8_t *) (ev + 1);
            const uint8_t type = lv2_midi_message_type(msg);
            uint8_t channel = msg[0] & 0x0f;

            switch (type) {
            case LV2_MIDI_MSG_NOTE_ON:
                channel = map_midi_to_pokey_channel(channel);
                if (channel < 0) continue;
                notes_on[channel][msg[1]] =
                            { current_timestamp, msg[2], programs[channel] };
                break;
            case LV2_MIDI_MSG_NOTE_OFF:
                channel = map_midi_to_pokey_channel(channel);
                if (channel < 0) continue;
                notes_on[channel].erase(msg[1]);
                break;
            case LV2_MIDI_MSG_PGM_CHANGE:
                channel = map_midi_to_pokey_channel(channel);
                if (channel < 0) continue;
                programs[channel] = msg[1];
                break;
            }
        }
    }

    current_timestamp += sample_count;
}

// ****************************************************************************
//
// C API
//
static LV2_Handle instantiate(const LV2_Descriptor *descriptor,
                              double sample_rate,
                              const char *bundle_path,
                              const LV2_Feature *const *features) {
    PokeySynth *ps = new PokeySynth(sample_rate, bundle_path, features);
    return ps;
}

static void connect_port(LV2_Handle instance, uint32_t port, void *data) {
    PokeySynth *ps = (PokeySynth *) instance;
    if (ps) ps->connect_port(port, data);
}

static void activate(LV2_Handle instance) {
}

static void run(LV2_Handle instance, uint32_t sample_count) {
    PokeySynth *ps = (PokeySynth *) instance;
    if (ps) ps->run(sample_count);
}

static void deactivate(LV2_Handle instance) {
}

static void cleanup(LV2_Handle instance) {
    free(instance);
}

static const void *extension_data(const char *URI) {
    return NULL;
}

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
