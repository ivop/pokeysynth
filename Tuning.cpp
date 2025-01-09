#include <math.h>
#include "Tuning.h"

// Docs: Altirra Reference Manual, RMT Raster/VinsCool code, GPL

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

    if (clk == CLOCK_DIV114 && dist == DIST_GRITTY_BASS)
        return GetPokeyDivider(DIST_BUZZY_BASS, clk, linked, frequency);

    float div1 = dividers1[clk],
          div2 = dividers2[dist];
    int xcycles = 1;

    if (clk == CLOCK_DIV1) {
        xcycles += 3;
        if (linked) xcycles += 3;
    }

    int div    = round(pokey_frequency / div1 / div2 / 2.0 / frequency);
    int pokdiv = div - xcycles;

    bool MOD3  = (div % 3) == 0;
    bool MOD5  = (div % 5) == 0;

    switch (dist) {
    case DIST_PURE:
    case DIST_NOISE:
        break;
    case DIST_BUZZY_BASS:
        if (clk == CLOCK_DIV114) {
            if (MOD5)
                pokdiv = FindClosest(div, DIST_BUZZY_BASS, div1, div2, xcycles);
        } else {
            if (!MOD3 || MOD5)
                pokdiv = FindClosest(div, DIST_BUZZY_BASS, div1, div2, xcycles);
        }
        if (!linked && clk != CLOCK_DIV114 && pokdiv > 0xff)
            return GetPokeyDivider(DIST_GRITTY_BASS, clk, linked, frequency);
        break;
    case DIST_GRITTY_BASS:
        // CLOCK_DIV114 redirect is handled at the beginning of this function
        if (MOD3 || MOD5)
            pokdiv = FindClosest(div, DIST_GRITTY_BASS, div1, div2, xcycles);
        break;
    case DIST_POLY5_SQUARE:
        // TODO
        break;
    }

    if (!linked && pokdiv > 0xff) pokdiv = 0xff;
    if (pokdiv > 0xffff) pokdiv = 0xffff;

    return pokdiv;
}

uint16_t Tuning::FindClosest(uint16_t div,
                             enum distortions dist,
                             float div1,
                             float div2,
                             int xcycles) {
    float target = pokey_frequency / div1 / div2 / 2.0 / div;

    int top = div, bottom = div;

    switch (dist) {
    case DIST_BUZZY_BASS:
        if (div1 == 114) {
            while (   top % 5 == 0) top++;
            while (bottom % 5 == 0) bottom--;
        } else {
            while (   top % 3 != 0 ||    top % 5 == 0) top++;
            while (bottom % 3 != 0 || bottom % 5 == 0) bottom--;
        }
        break;
    case DIST_GRITTY_BASS:
        while (   top % 3 == 0 ||    top % 5 == 0) top++;
        while (bottom % 3 == 0 || bottom % 5 == 0) bottom--;
        break;
    default:
        // shouldn't be here
        break;
    }

    float dtop    = fabs(target - pokey_frequency / div1 / div2 / 2.0 / top);
    float dbottom = fabs(target - pokey_frequency / div1 / div2 / 2.0 / bottom);

    // fixme?: calculate difference in cents instead of hertz
    return (dbottom > dtop ? top : bottom) - xcycles;
}
