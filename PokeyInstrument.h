#pragma once
#include <stdint.h>

enum channels_type {
    CHANNELS_NONE,                // silent channel

    // BASE CLOCK
    CHANNELS_1CH,                 // 1 or 2 or 3 or 4, 8-bit divider
                                  //   ("normal" channels)
                                  //
    CHANNELS_2CH_LINKED,          // 1+2 or 3+4, 16-bit divider
                                  //   (not sure how useful on base clock)
                                  //
    CHANNELS_2CH_FILTERED,        // 1+3 or 2+4, 8-bit divider, 8-bit filter
                                  //   ("normal" -bit filter sound)
                                  //
    CHANNELS_4CH_LINKED_FILTERED, // 1+2+3+4, 16-bit divider, 16-bit filter
                                  //   (again, not that useful without HIFRQ)

    // HIGH CLOCK
    CHANNELS_1CH_HIFRQ,           // 1 or 3, 8-bit divider, high frequency
                                  //  (poly5 square)
                                  //
    CHANNELS_2CH_LINKED_HIFRQ,    // 1+2 or 3+4, 16-bit divider, high frequency
                                  //  (common for 16-bit full range pure)
                                  //
    CHANNELS_2CH_FILTERED_HIFRQ,  // 1+3, 8-bit divider, 8-bit filter,
                                  //  (sawtooth trick)
                                  //
    CHANNELS_4CH_LINKED_FILTERED_HIFRQ  // 1+2+3+4, 16-bit divider, 16-bit
                                  //  (full pokey for full range filter sound)
};

enum base_clocks {
    CLOCK_15kHz,    // 0, false
    CLOCK_64kHz,    // 1, true
    CLOCK_1M8Hz,    // as a return value, _not_ the internal base clock field(!)
    CLOCK_DONT_CARE // idem, when channel is silent anyway
};

class PokeyInstrument {
public:
    PokeyInstrument(void);
    void Start(const uint8_t midi_note, const uint8_t velocity, const uint8_t program);
    void Restart(void);
    void Next(void);
    void Release(void);
    enum base_clocks GetClock(void);
    enum channels_type GetChannels(void);
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
};
