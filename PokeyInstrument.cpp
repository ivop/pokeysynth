#include <stdint.h>
#include <math.h>
#include <stdio.h>
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
    0xc0,
    0x20
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

    enum clocks clock;

    uint8_t volume[INSTRUMENT_LENGTH];
    uint8_t distortion[INSTRUMENT_LENGTH];
    uint8_t sustain_loop_start;
    uint8_t sustain_loop_end;             // also release_start
    uint8_t release_end;

    uint8_t types[INSTRUMENT_LENGTH];     // note_types, 0 has no value
    int32_t values[INSTRUMENT_LENGTH];    // 8/16/32-bit values for types >= 1
    uint8_t types_end;
    uint8_t types_loop;
    uint8_t types_speed;

    float filtered_detune;              // detune 2nd channel in cents
    float filtered_vol2;                // volume of 2nd channel
    bool  filtered_transpose;           // transpose 1 octave down
};

struct pokey_instrument instruments[128];

#include "test_instruments.cpp"

// *************************************************************************

PokeyInstrument::PokeyInstrument(void) :
    program(0),
    note(0),
    velocity(1.0),
    release(false),
    silent(true),
    voldis_idx(0),
    types_idx(0),
    types_speed_cnt(0),
    pokey_freq(0) {

    instruments[0] = test_instrument0;
    instruments[1] = test_instrument1;
    instruments[2] = test_instrument2;
}

void PokeyInstrument::SetPokeyFrequency(int frequency) {
    pokey_freq = frequency;
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

enum clocks PokeyInstrument::GetClock(void) {
    if (silent) return CLOCK_NONE;
    return instruments[program].clock;
}

enum channels_type PokeyInstrument::GetChannel(void) {
    if (silent) return CHANNELS_NONE;
    return instruments[program].channels;
}

uint32_t PokeyInstrument::GetAudc(void) {
    if (silent) return 0;
    int channels = instruments[program].channels;

    if (channels == CHANNELS_1CH) {
        int volume = instruments[program].volume[voldis_idx];
        volume = round(velocity * volume);
        int dist = dist_values[instruments[program].distortion[voldis_idx]];
        return dist | volume;
    }

    return 0;
}

uint32_t PokeyInstrument::GetAudf(void) {
    if (silent) return 0;

    uint8_t type = instruments[program].types[types_idx];
    int32_t value = instruments[program].values[types_idx];

    if (type == TYPE_FIXED_DIVIDER) return value;

    int xnote = note;

    if (TYPE_NOTE_PLUS_NOTE) xnote += value;

    float freq = pow(2.0, (xnote-69) / 12.0) * 440.0;

    if (type == TYPE_NOTE_PLUS_CENTS) freq *= pow(2.0, value / 1200.0);

    if (!freq) return 0;

    int channels = instruments[program].channels;
    int dist     = instruments[program].distortion[voldis_idx];
    int clock    = instruments[program].clock;
    int pokdiv = 0;

    if (channels == CHANNELS_1CH) {
        if (dist == DIST_PURE) {
recalc:
            if (clock == CLOCK_DIV114) {
                pokdiv = round((pokey_freq / 114.0 / 2.0 / freq) - 1);
            } else if (clock == CLOCK_DIV28) {
                pokdiv = round((pokey_freq / 28.0 / 2.0 / freq) - 1);
            } else {
                pokdiv = round((pokey_freq / 1.0 / 2.0 / freq) - 7);
            }
            if (pokdiv < 0) return 0;
            if (pokdiv > 255) {
                freq *= 2.0;
                goto recalc;
            }
            return pokdiv;
        }
    }
    return 0;
}

const char *PokeyInstrument::GetName(void) {
    return instruments[program].name;
}
