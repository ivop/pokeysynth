#pragma once
#include <stdint.h>
#include "PokeyInstrument.h"

class Tuning {
public:
    Tuning(void);
    void SetPokeyFrequency(float pokey_frequency);
    uint16_t GetPokeyDivider(enum distortions dist,
                             enum clocks clk,
                             bool linked,
                             float frequency);

private:
    float pokey_frequency;
};
