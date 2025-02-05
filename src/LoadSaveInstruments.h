#pragma once
#include <fstream>
#include <stdint.h>
#include "PokeyInstrument.h"

class LoadSaveInstruments {
public:
    LoadSaveInstruments(void);

    bool LoadInstrument(int program, const char *filename);
    bool SaveInstrument(int program, const char *filename);

    bool LoadBank(const char *filename);
    bool SaveBank(const char *filename);

    bool ExportInstrumentList(const char *filename);

    const char *error_message;

private:
    bool load_instrument(std::ifstream &input, int program);
    bool save_instrument(std::ofstream &output, int program);
    uint32_t read_uint32(std::ifstream &input);
    void write_uint32(std::ofstream &output, uint32_t value);
    float read_float(std::ifstream &input);
    void write_float(std::ofstream &output, float value);
};
