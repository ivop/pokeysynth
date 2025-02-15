#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include "PokeyInstrument.h"
#include "Tuning.h"

Tuning tuning;

static uint8_t dist_values[] = {
    0xa0,
    0x80,
    0xc0,
    0xc0,
    0x20
};

// *************************************************************************

PokeyInstrument::PokeyInstrument(struct pokey_instrument (&instrdata)[128]) :
    program(0),
    note(0),
    velocity(1.0),
    release(false),
    silent(true),
    voldis_idx(0),
    types_idx(0),
    types_speed_cnt(0),
    pokey_freq(0),
    pitch_shift(1.0),
    mod_lfo_angle(0.0),
    mod_amount(0.0),
    volume_cc(1.0),
    overdrive_compensation(1.0),
    instrdata(instrdata) {
}

void PokeyInstrument::SetPokeyFrequency(int frequency) {
    pokey_freq = frequency;
    tuning.SetPokeyFrequency(frequency);
}

void PokeyInstrument::Restart(void) {
    release = silent = false;
    voldis_idx = types_idx = 0;
    types_speed_cnt = instrdata[program].types_speed;
    mod_lfo_angle = 0.0;
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
        types_speed_cnt = instrdata[program].types_speed;
        types_idx++;
        if (types_idx > instrdata[program].types_end) {
            types_idx = instrdata[program].types_loop;
        }
    }
    voldis_idx++;
    if (!release) {
        if (voldis_idx > instrdata[program].sustain_loop_end) {
            voldis_idx = instrdata[program].sustain_loop_start;
        }
        if (voldis_idx > instrdata[program].release_end) {
            silent = true;
        }
    } else {
        if (voldis_idx > instrdata[program].release_end) {
            silent = true;
        }
    }

    mod_lfo_angle += instrdata[program].mod_lfo_speed;
}

void PokeyInstrument::Release(void) {
    if (silent || release) return;
    release = true;
    voldis_idx = instrdata[program].sustain_loop_end;
    if (voldis_idx > instrdata[program].release_end) {
        silent = true;
    }
}

enum clocks PokeyInstrument::GetClock(void) {
    if (silent) return CLOCK_NONE;
    return instrdata[program].clock;
}

enum channels_type PokeyInstrument::GetChannel(void) {
    if (silent) return CHANNELS_NONE;
    return instrdata[program].channels;
}

uint32_t PokeyInstrument::GetAudc(void) {
    if (silent) return 0;
    int channels = instrdata[program].channels;

    int volume  = instrdata[program].volume[voldis_idx];
    volume = round(overdrive_compensation * volume_cc * velocity *
                                                            (float) volume);
    int volume2 = round(volume * instrdata[program].filtered_vol2);

    int dist = dist_values[instrdata[program].distortion[voldis_idx]];

    if (channels == CHANNELS_1CH) {
        return dist | volume;
    }

    else if (channels == CHANNELS_2CH_LINKED) {
        return (dist | volume) << 8;
    }

    else if (channels == CHANNELS_2CH_FILTERED) {
        if (instrdata[program].clock == CLOCK_DIV1) {
            volume2 = volume;       // sawtooth
        }
        return ((dist | volume2) << 8 ) | dist | volume;
    }

    else if (channels == CHANNELS_4CH_LINKED_FILTERED) {
        return ((dist | volume2) << 8 ) | dist | volume;
    }

    return 0;
}

uint32_t PokeyInstrument::GetAudf(void) {
    if (silent) return 0;

    uint8_t type = instrdata[program].types[types_idx];
    int32_t value = instrdata[program].values[types_idx];

    if (type == TYPE_FIXED_DIVIDER) return value;

    int xnote = note;

    if (type == TYPE_NOTE_PLUS_NOTE) xnote += value;

    float freq = pow(2.0, (xnote-69) / 12.0) * 440.0;

    if (type == TYPE_NOTE_PLUS_CENTS) freq *= pow(2.0, value / 1200.0);

    freq *= pitch_shift;

    float maxdepth = instrdata[program].mod_lfo_maxdepth;
    float modwheel = sin(mod_lfo_angle) * mod_amount * maxdepth;

    freq *= pow(2.0, modwheel / 1200.0);

    if (!freq) return 0;

    enum channels_type channels = instrdata[program].channels;
    enum distortions   dist     = instrdata[program].distortion[voldis_idx];
    enum clocks        clock    = instrdata[program].clock;

    switch (channels) {
    case CHANNELS_1CH:
        return tuning.GetPokeyDivider(dist, clock, false, freq);
        break;
    case CHANNELS_2CH_LINKED:
        return tuning.GetPokeyDivider(dist, clock, true, freq);
        break;
    case CHANNELS_2CH_FILTERED: {
        if (clock == CLOCK_DIV1) {  // sawtooth exception
            int div = tuning.GetSawtoothDivider(freq);
            return div | ((div+1) << 8);
        }

        if (instrdata[program].filtered_transpose) freq /= 2.0;

        int bch = tuning.GetPokeyDivider(dist, clock, false, freq);

        freq *= pow(2.0, instrdata[program].filtered_detune / 1200.0);
        int dch = tuning.GetPokeyDivider(dist, clock, false, freq);

        if (dch == bch) dch++;
        if (dch > 0xff) dch = 0xff;

        return bch | (dch << 8);
        break;
        }
    case CHANNELS_4CH_LINKED_FILTERED: {
        if (instrdata[program].filtered_transpose) freq /= 2.0;

        int bch = tuning.GetPokeyDivider(dist, clock, true, freq);

        freq *= pow(2.0, instrdata[program].filtered_detune / 1200.0);
        int dch = tuning.GetPokeyDivider(dist, clock, true, freq);

        if (dch == bch) dch++;
        if (dch > 0xffff) dch = 0xffff;

        return bch | (dch << 16);
        }
        break;
    case CHANNELS_NONE:
    default:
        break;
    }

    return 0;
}

const char *PokeyInstrument::GetName(void) {
    return instrdata[program].name;
}

void PokeyInstrument::SetPitchShift(int value) {
    float cents = (float) (value - 0x2000) / 0x2000 * instrdata[program].bender_range;
    pitch_shift = pow(2.0, cents / 1200.0);
}

void PokeyInstrument::SetModWheel(int value) {
    mod_amount = (float) value / 127.0;
}

void PokeyInstrument::SetVolumeCC(int value) {
    volume_cc = (float) value / 127.0;
}

void PokeyInstrument::SetOverdriveCompensation(int value) {
    overdrive_compensation = (float) value / 15.0;
}
