#pragma once
#include <stdint.h>

struct mzpokey_context;

enum pokey_register {
    AUDF1, AUDC1, AUDF2, AUDC2, AUDF3, AUDC3, AUDF4, AUDC4,
    AUDCTL, STIMER, SKCTL = 15
};

struct mzpokey_context *mzpokey_create(int pokey_frequency,
                                       int playback_frequency,
                                       int number_of_pokeys,
                                       int filter_quality);

void mzpokey_write_register(struct mzpokey_context *mzp,
                            enum pokey_register reg,
                            uint8_t value,
                            uint8_t chip);

void mzpokey_process_int16(struct mzpokey_context *mzp,
                           void *sndbuffer,
                           int sndn);

void mzpokey_process_float(struct mzpokey_context *mzp,
                           void *sndbuffer,
                           int sndn);
