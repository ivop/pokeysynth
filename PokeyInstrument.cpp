#include <stdint.h>
#include "PokeyInstrument.h"

#define INSTRUMENT_LENGTH 64

enum distortions {
    DIST_PURE,
    DIST_NOISE,
    DIST_BUZZY_BASS,
    DIST_GRITTY_BASS,
    DIST_POLY5_SQUARE
};

static uint8_t dist_values[] = {
    0xa0,
    0x80,
    0xc0,
    0xc0
};

enum note_types {
    TYPE_NOTE,              // frequency depends on MIDI Note
    TYPE_NOTE_PLUS_NOTE,    // same as note, but with +/- whole semitones
    TYPE_NOTE_PLUS_CENTS,   // same as note, but offset +/- by x cents
    TYPE_FIXED_DIVIDER      // fixed divider/frequency, e.g. for drum sounds
};

struct pokey_instrument {
    char name[32];

    enum channels_type channels;

    bool base_clock;                        // 15kHz or 64kHz

    uint8_t volume[INSTRUMENT_LENGTH];
    uint8_t distortion[INSTRUMENT_LENGTH];
    uint8_t sustain_loop_start;
    uint8_t sustain_loop_end;               // also release_start
    uint8_t release_end;

    uint8_t types[INSTRUMENT_LENGTH];       // note_types, 0 has no value
    uint16_t values[INSTRUMENT_LENGTH];     // 8/16-bit value for types >= 1
    uint8_t types_end;
    uint8_t types_loop;
    uint8_t types_speed;

    float filtered_detune;              // detune 2nd channel in cents
    float filtered_vol2;                // volume of 2nd channel
    bool  filtered_transpose;           // transpose 1 octave down
};

struct pokey_instrument instruments[128];

struct pokey_instrument instrument = {
    .name = "Test",

    .channels = CHANNELS_1CH,

    .base_clock = CLOCK_64kHz,

    .volume = { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 },

    .distortion = { 
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
    },

    .sustain_loop_start = 7,
    .sustain_loop_end = 7,
    .release_end =  15,

    .types = { TYPE_NOTE },
    .types_end = 0,
    .types_loop = 0,
    .types_speed = 0
};

// *************************************************************************

PokeyInstrument::PokeyInstrument(void) :
    program(0),
    note(0),
    velocity(1.0),
    release(false),
    silent(false),
    voldis_idx(0),
    types_idx(0),
    types_speed_cnt(0) {
}

void PokeyInstrument::Restart(void) {
    release = silent = false;
    voldis_idx = types_idx = 0;
    types_speed_cnt = instruments[program].types_speed;
}

void PokeyInstrument::Start(const uint8_t midi_note, const uint8_t velo, const uint8_t pgm) {
    note = midi_note;
    velocity = (float) velo / 127.0;
    program = pgm;
    Restart();
}

void PokeyInstrument::Next(void) {
    if (silent) return;

    types_speed_cnt--;
    if (types_speed_cnt < 0) {
        types_speed_cnt = instruments[program].types_speed;
        types_idx++;
        if (types_idx > instruments[program].types_end) {
            types_idx = instruments[program].types_loop;
        }
    }
    voldis_idx++;
    if (!release) {
        if (voldis_idx > instruments[program].sustain_loop_end) {
            voldis_idx = instruments[program].sustain_loop_start;
        }
    } else {
        if (voldis_idx > instruments[program].release_end) {
            silent = true;
        }
    }
}

void PokeyInstrument::Release(void) {
    if (silent || release) return;
    release = true;
    voldis_idx = instruments[program].sustain_loop_end;
    if (voldis_idx > instruments[program].release_end) {
        silent = true;
    }
}

enum base_clocks PokeyInstrument::GetClock(void) {
    if (silent) return CLOCK_DONT_CARE;
    if (instruments[program].channels >= CHANNELS_1CH_HIFRQ)
        return CLOCK_1M8Hz;
    else
        return (enum base_clocks) instruments[program].base_clock;
}

enum channels_type PokeyInstrument::GetChannels(void) {
    if (silent) return CHANNELS_NONE;
    return instruments[program].channels;
}

uint32_t PokeyInstrument::GetAudc(void) {
    if (silent) return 0;
    return 0;
}

uint32_t PokeyInstrument::GetAudf(void) {
    if (silent) return 0;
    return 0;
}

const char *PokeyInstrument::GetName(void) {
    return instruments[program].name;
}
