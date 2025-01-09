#pragma once
#include <stdint.h>
#include "PokeyInstrument.h"

class Tuning {
public:
    Tuning(void);
    void SetPokeyFrequency(float pokey_frequency);
    uint8_t GetDividerSingle(enum distortions distortion,
                             enum clocks clock,
                             float frequency);
    uint16_t GetDividerLinked(enum distortions distortion,
                              enum clocks clock,
                              float frequency);

private:
    float pokey_frequency;
};
