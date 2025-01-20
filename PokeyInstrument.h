#pragma once
#include <stdint.h>

enum channels_type : uint8_t {
    CHANNELS_NONE,                // silent channel

    CHANNELS_1CH,                 // 1 or 2 or 3 or 4, 8-bit divider
                                  //   div114 -> bass notes
                                  //   div28  -> "normal" notes
                                  //   div1   -> channel 1 and 3 only,
                                  //                    e.g. poly5 square
                                  //
    CHANNELS_2CH_LINKED,          // 1+2 or 3+4, 16-bit divider
                                  //   div114 -> not paricualarly useful?
                                  //   div28  -> not paricualarly useful?
                                  //   div1   -> common full 16-bit range notes
                                  //
    CHANNELS_2CH_FILTERED,        // 1+3 or 2+4, 8-bit divider, 8-bit filter
                                  //   div114 -> bass 8-bit filtered sound
                                  //   div28  -> "normal" 8-bit filter sound
                                  //   div1   -> channel 1 and 3 only,
                                  //                    e.g. sawtooth trick
                                  //
    CHANNELS_4CH_LINKED_FILTERED, // 1+2+3+4, 16-bit divider, 16-bit filter
                                  //   div114 -> not paricualarly useful?
                                  //   div28  -> not paricualarly useful?
                                  //   div1   -> 16-bit filter sound, full
                                  //             pokey for one voice
};

enum audctl_bits {
    AUDCTL_CLOCK_15KHZ = 0x01,
    AUDCTL_HIPASS_24   = 0x02,
    AUDCTL_HIPASS_13   = 0x04,
    AUDCTL_LINK_34     = 0x08,
    AUDCTL_LINK_12     = 0x10,
    AUDCTL_CH3_HIFRQ   = 0x20,
    AUDCTL_CH1_HIFRQ   = 0x40,
    AUDCTL_POLY917     = 0x80
};

enum distortions : uint8_t {
    DIST_PURE,
    DIST_NOISE,
    DIST_BUZZY_BASS,
    DIST_GRITTY_BASS,
    DIST_POLY5_SQUARE
};
#define DIST_COUNT (DIST_POLY5_SQUARE+1)

enum clocks : uint8_t {
    CLOCK_DIV114,
    CLOCK_DIV28,
    CLOCK_DIV1,
    CLOCK_NONE
};

class PokeyInstrument {
public:
    PokeyInstrument(void);
    void SetPokeyFrequency(int frequency);
    void Start(const uint8_t midi_note,
               const uint8_t velocity,
               const uint8_t program);
    void Restart(void);
    void Next(void);
    void Release(void);
    enum clocks GetClock(void);
    enum channels_type GetChannel(void);
    uint32_t GetAudf(void);
    uint32_t GetAudc(void);
    const char *GetName(void);
    void SetPitchShift(int value);
    void SetModWheel(int value);
    void SetVolumeCC(int value);

private:
    uint8_t program;
    uint8_t note;                       // MIDI note number
    float velocity;                     // 0.0-1.0
    bool release;
    bool silent;
    unsigned int voldis_idx;
    unsigned int types_idx;
    int types_speed_cnt;
    int pokey_freq;
    float pitch_shift;                  // in cents
    float mod_lfo_angle;                // angle in radians
    float mod_amount;                   // 0-127 mapped to 0.0-1.0
    float volume_cc;                    // 0-127 mapped to 0.0-1.0
};

#define INSTRUMENT_LENGTH 64

struct pokey_instrument {
    char name[64];

    enum channels_type channels;

    enum clocks clock;

    uint8_t volume[INSTRUMENT_LENGTH];
    enum distortions distortion[INSTRUMENT_LENGTH];
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

    float bender_range;                 // pitch wheel +/- range in cents

    float mod_lfo_maxdepth;             // modulation wheel maxdepth in cents
    float mod_lfo_speed;                // angle step in radians per frame
};

extern struct pokey_instrument instrdata[128];
