#include <math.h>
#include "Tuning.h"

Tuning::Tuning(void) :
    pokey_frequency(1773447.0) {
}

void Tuning::SetPokeyFrequency(float f) {
    pokey_frequency = f;
}

static float dividers1[4] = { 114, 28, 1, 1 };
static float dividers2[DIST_COUNT] = {
    [DIST_PURE]         =  1.0,
    [DIST_NOISE]        =  1.0,
    [DIST_BUZZY_BASS]   =  2.5,
    [DIST_GRITTY_BASS]  =  7.5,
    [DIST_POLY5_SQUARE] = 31.0
};

uint16_t Tuning::GetPokeyDivider(enum distortions dist,
                                 enum clocks clk,
                                 bool linked,
                                 float frequency) {
    float div1 = dividers1[clk],
          div2 = dividers2[dist],
          xcycles = 1.0;
    int pokdiv = 0;

    if (clk == CLOCK_DIV1) {
        xcycles += 3.0;
        if (linked) xcycles += 3.0;
    }

    pokdiv = round((pokey_frequency / div1 / div2 / 2.0 / frequency) - xcycles);

    if (!linked && pokdiv > 0xff) pokdiv = 0xff;
    if (pokdiv > 0xffff) pokdiv = 0xffff;

    return pokdiv;
}
