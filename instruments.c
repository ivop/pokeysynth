#define INSTRUMENT_LENGTH 64

enum channels_type {
    CHANNELS_1CH,                 // 1 or 2 or 3 or 4, 8-bit divider
    CHANNELS_2CH_LINKED,          // 1+2 or 3+4, 16-bit divider
    CHANNELS_2CH_FILTERED,        // 1+3 or 2+4, 8-bit divider, 8-bit filtered
    CHANNELS_4CH_LINKED_FILTERED, // 1+2+3+4, 16-bit divider, 16-bit filtered
    CHANNELS_1CH_HIFRQ            // 1 or 3, 8-bit divider, high frequency
};

enum distortions {
    DIST_1CH_PURE,
    DIST_1CH_NOISE,
    DIST_1CH_BUZZY_BASS,
    DIST_1CH_GRITTY_BASS
};

enum base_clocks {
    CLOCK_15kHz,
    CLOCK_64kHz
};

enum note_types {
    NOTE,               // frequency depends on MIDI Note
    NOTE_PLUS_NOTE,     // same as note, but with +/- whole semitones
    NOTE_PLUS_CENTS,    // same as note, but offset +/- by x cents
    FIXED_DIVIDER       // fixed divider/frequency, e.g. for drum sounds
};

struct pokey_instrument {
    char name[32];

    uint8_t num_pokey_channels;             // 1, 2, or 4

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
};
