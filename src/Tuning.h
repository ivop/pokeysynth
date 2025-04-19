#pragma once
// ****************************************************************************
//
// This file is part of PokeySynth.
//
// Copyright © 2024, 2025, by Ivo van Poorten
//
// Licensed under the terms of the General Public License, version 2.
// See the LICENSE file in the root of the prohect directory for the full text.
//
// ****************************************************************************

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
    uint8_t GetSawtoothDivider(float frequency);

private:
    float pokey_frequency;
    uint16_t FindClosest(uint16_t div,
                         enum distortions dist,
                         float div1,
                         float div2,
                         int xcycles);
};
