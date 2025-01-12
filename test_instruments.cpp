
struct pokey_instrument test_instrument0 = {
    .name = "Test",

    .channels = CHANNELS_1CH,

    .clock = CLOCK_DIV28,

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
    .values = {},
    .types_end = 0,
    .types_loop = 0,
    .types_speed = 0,

    .filtered_detune = 0.0,
    .filtered_vol2 = 0.0,
    .filtered_transpose = false,

    .bender_range = 1200.0
};

struct pokey_instrument test_instrument1 = {
    .name = "Test Arp",

    .channels = CHANNELS_1CH,

    .clock = CLOCK_DIV28,

    .volume = { 15,15,14,14,13,13,12,12,
                11,11,10,10,9,9,8,8,
                7,7,7,7,6,6,6,6,
                5,5,5,5,4,4,4,4,
                3,3,3,3,2,2,2,2,
                1,1,1,1,0,0,0,0 },

    .distortion = {
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
    },

    .sustain_loop_start = 15,
    .sustain_loop_end = 15,
    .release_end = 47,

    .types = { TYPE_NOTE_PLUS_NOTE, TYPE_NOTE_PLUS_NOTE, TYPE_NOTE_PLUS_NOTE },
    .values = { 0, 4, 7 },
    .types_end = 2,
    .types_loop = 0,
    .types_speed = 3,

    .filtered_detune = 0.0,
    .filtered_vol2 = 0.0,
    .filtered_transpose = false,

    .bender_range = 200.0
};

struct pokey_instrument test_instrument2 = {
    .name = "Test 15kHz Bass",

    .channels = CHANNELS_1CH,

    .clock = CLOCK_DIV114,

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
    .values = {},
    .types_end = 0,
    .types_loop = 0,
    .types_speed = 0,

    .filtered_detune = 0.0,
    .filtered_vol2 = 0.0,
    .filtered_transpose = false,

    .bender_range = 200.0
};

struct pokey_instrument test_instrument3 = {
    .name = "Test Drum",

    .channels = CHANNELS_1CH,

    .clock = CLOCK_DIV28,

    .volume = { 15,13,11,9,7,7,7,3,0 },

    .distortion = {
        DIST_PURE, DIST_PURE, DIST_PURE, DIST_PURE,
        DIST_NOISE, DIST_NOISE, DIST_NOISE, DIST_NOISE,
        DIST_NOISE,
    },

    .sustain_loop_start = 8,
    .sustain_loop_end = 8,
    .release_end =  8,

    .types = {
        TYPE_FIXED_DIVIDER, TYPE_FIXED_DIVIDER, TYPE_FIXED_DIVIDER,
        TYPE_FIXED_DIVIDER, TYPE_FIXED_DIVIDER, TYPE_FIXED_DIVIDER,
        TYPE_FIXED_DIVIDER, TYPE_FIXED_DIVIDER, TYPE_FIXED_DIVIDER,
    },
    .values = {
        0xc0, 0xd0, 0xe0, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    .types_end = 8,
    .types_loop = 0,
    .types_speed = 0,

    .filtered_detune = 0.0,
    .filtered_vol2 = 0.0,
    .filtered_transpose = false,

    .bender_range = 200.0
};

struct pokey_instrument test_instrument4 = {
    .name = "Test Dist C Buzzy Bass",

    .channels = CHANNELS_1CH,

    .clock = CLOCK_DIV28,

    .volume = { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 },

    .distortion = {
        DIST_BUZZY_BASS, DIST_BUZZY_BASS, DIST_BUZZY_BASS, DIST_BUZZY_BASS,
        DIST_BUZZY_BASS, DIST_BUZZY_BASS, DIST_BUZZY_BASS, DIST_BUZZY_BASS,
        DIST_BUZZY_BASS, DIST_BUZZY_BASS, DIST_BUZZY_BASS, DIST_BUZZY_BASS,
        DIST_BUZZY_BASS, DIST_BUZZY_BASS, DIST_BUZZY_BASS, DIST_BUZZY_BASS,
    },

    .sustain_loop_start = 7,
    .sustain_loop_end = 7,
    .release_end =  15,

    .types = { TYPE_NOTE },
    .values = {},
    .types_end = 0,
    .types_loop = 0,
    .types_speed = 0,

    .filtered_detune = 0.0,
    .filtered_vol2 = 0.0,
    .filtered_transpose = false,

    .bender_range = 200.0
};

struct pokey_instrument test_instrument5 = {
    .name = "Test Dist C Gritty Bass",

    .channels = CHANNELS_1CH,

    .clock = CLOCK_DIV28,

    .volume = { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 },

    .distortion = {
        DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS,
        DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS,
        DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS,
        DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS,
    },

    .sustain_loop_start = 7,
    .sustain_loop_end = 7,
    .release_end =  15,

    .types = { TYPE_NOTE },
    .values = {},
    .types_end = 0,
    .types_loop = 0,
    .types_speed = 0,

    .filtered_detune = 0.0,
    .filtered_vol2 = 0.0,
    .filtered_transpose = false,

    .bender_range = 200.0
};

struct pokey_instrument test_instrument6 = {
    .name = "Test Dist C Buzzy Bass 15kHz",

    .channels = CHANNELS_1CH,

    .clock = CLOCK_DIV114,

    .volume = { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 },

    .distortion = {
        DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS,
        DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS,
        DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS,
        DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS, DIST_GRITTY_BASS,
    },

    .sustain_loop_start = 7,
    .sustain_loop_end = 7,
    .release_end =  15,

    .types = { TYPE_NOTE },
    .values = {},
    .types_end = 0,
    .types_loop = 0,
    .types_speed = 0,

    .filtered_detune = 0.0,
    .filtered_vol2 = 0.0,
    .filtered_transpose = false,

    .bender_range = 200.0
};

struct pokey_instrument test_instrument7 = {
    .name = "Test Dist Poly 5 Square 1.8MHz",

    .channels = CHANNELS_1CH,

    .clock = CLOCK_DIV1,

    .volume = { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 },

    .distortion = {
        DIST_POLY5_SQUARE, DIST_POLY5_SQUARE, DIST_POLY5_SQUARE,
        DIST_POLY5_SQUARE, DIST_POLY5_SQUARE, DIST_POLY5_SQUARE,
        DIST_POLY5_SQUARE, DIST_POLY5_SQUARE, DIST_POLY5_SQUARE,
        DIST_POLY5_SQUARE, DIST_POLY5_SQUARE, DIST_POLY5_SQUARE,
        DIST_POLY5_SQUARE, DIST_POLY5_SQUARE, DIST_POLY5_SQUARE,
        DIST_POLY5_SQUARE,
    },

    .sustain_loop_start = 7,
    .sustain_loop_end = 7,
    .release_end =  15,

    .types = { TYPE_NOTE },
    .values = {},
    .types_end = 0,
    .types_loop = 0,
    .types_speed = 0,

    .filtered_detune = 0.0,
    .filtered_vol2 = 0.0,
    .filtered_transpose = false,

    .bender_range = 200.0
};

struct pokey_instrument test_instrument8 = {
    .name = "Test",

    .channels = CHANNELS_2CH_LINKED,

    .clock = CLOCK_DIV1,

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
    .values = {},
    .types_end = 0,
    .types_loop = 0,
    .types_speed = 0,

    .filtered_detune = 0.0,
    .filtered_vol2 = 0.0,
    .filtered_transpose = false,

    .bender_range = 200.0
};

struct pokey_instrument test_instrument9 = {
    .name = "Test Filtered Pure Tone 64kHz",

    .channels = CHANNELS_2CH_FILTERED,

    .clock = CLOCK_DIV28,

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
    .values = {},
    .types_end = 0,
    .types_loop = 0,
    .types_speed = 0,

    .filtered_detune = 0.0,
    .filtered_vol2 = 0.5,
    .filtered_transpose = true,

    .bender_range = 200.0
};

struct pokey_instrument test_instrument10 = {
    .name = "Test Filtered Tone 1.8MHz (Sawtooth)",

    .channels = CHANNELS_2CH_FILTERED,

    .clock = CLOCK_DIV1,

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
    .values = {},
    .types_end = 0,
    .types_loop = 0,
    .types_speed = 0,

    .filtered_detune = 0.0,
    .filtered_vol2 = 0.5,
    .filtered_transpose = true,

    .bender_range = 200.0
};

struct pokey_instrument test_instrument11 = {
    .name = "Test 32-bit full pokey filter",

    .channels = CHANNELS_4CH_LINKED_FILTERED,

    .clock = CLOCK_DIV1,

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
    .values = {},
    .types_end = 0,
    .types_loop = 0,
    .types_speed = 0,

    .filtered_detune = 10.0,
    .filtered_vol2 = 0.5,
    .filtered_transpose = true,

    .bender_range = 200.0
};
