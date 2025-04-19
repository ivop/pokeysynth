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

#include <fstream>
#include <stdint.h>
#include "PokeyInstrument.h"

class LoadSaveInstruments {
public:
    LoadSaveInstruments(struct pokey_instrument (&instrdata)[128]);

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

    struct pokey_instrument (&instrdata)[128];
};
