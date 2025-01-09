#include <math.h>
#include "Tuning.h"

Tuning::Tuning(void) :
    pokey_frequency(1773447.0) {
}

void Tuning::SetPokeyFrequency(float f) {
    pokey_frequency = f;
}

static float dividers[4] = { 114, 28, 1, 1 };

uint8_t Tuning::GetDividerSingle(enum distortions distortion,
                                 enum clocks clock,
                                 float frequency) {
    float div1 = dividers[clock],
          div2 = 1.0,
          extracycles = 1.0 + (clock == CLOCK_DIV1 ? 3.0 : 0.0);
    int pokdiv = 0;

    switch (distortion) {
    case DIST_PURE:
    case DIST_NOISE:
        pokdiv = round((pokey_frequency / div1 / div2 / 2.0 / frequency)
                                                            - extracycles);
        break;
    default:
        break;
    }

    if (pokdiv < 0) pokdiv = 0;

    return pokdiv > 255 ? 0xff : pokdiv;
}

uint16_t Tuning::GetDividerLinked(enum distortions distortion,
                                  enum clocks clock,
                                  float frequency) {
    return 0;
}
