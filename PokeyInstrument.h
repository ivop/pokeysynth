#pragma once
#include <stdint.h>

class PokeyInstrument {
public:
    PokeyInstrument(const uint8_t program);
    void Start(const uint8_t midi_note, const uint8_t velocity, const uint8_t program);
    void Restart(void);
    void Next(void);
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
