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
    void Start(const uint8_t midi_note, const uint8_t velocity, const uint8_t program);
    void Restart(void);
    void Next(void);
    void Release(void);
    enum clocks GetClock(void);
    enum channels_type GetChannel(void);
    uint32_t GetAudf(void);
    uint32_t GetAudc(void);
    const char *GetName(void);

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
};
