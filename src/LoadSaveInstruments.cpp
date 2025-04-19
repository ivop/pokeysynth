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

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "PokeyInstrument.h"
#include "LoadSaveInstruments.h"

// Load and Save Instruments to our global instrument array
// Use the same code in DSP and GUI

#define MAGIC_LENGTH                16
#define POKEYSYNTH_INSTRUMENT_MAGIC "POKEYSYNTH_INSTR"
#define POKEYSYNTH_BANK_MAGIC       "POKEYSYNTH_BANK!"
#define FILE_FORMAT_VERSION         1

// ****************************************************************************

LoadSaveInstruments::LoadSaveInstruments(struct pokey_instrument (&instrdata)[128])
    : instrdata(instrdata) {
    error_message = "No errors";
}

bool LoadSaveInstruments::LoadInstrument(int program, const char *filename) {
    std::ifstream input(filename, std::ios::binary);

    if (!input.is_open()) {
        error_message = "Unable to open file!";
        return false;
    }

    if (!load_instrument(input, program)) {
        input.close();
        //error_message = "Loading instrument failed!";
        return false;
    }

    input.close();
    return true;
}

// ****************************************************************************

bool LoadSaveInstruments::SaveInstrument(int program, const char *filename) {
    std::ofstream output(filename, std::ios::binary);

    if (!output.is_open()) {
        error_message = "Unable to open file!";
        return false;
    }

    if (!save_instrument(output, program)) {
        output.close();
        error_message = "Saving instrument failed!";
        return false;
    }

    output.close();
    return true;
}

// ****************************************************************************

bool LoadSaveInstruments::LoadBank(const char *filename) {
    char magic[16];
    int file_format_version = 0;
    std::ifstream input(filename, std::ios::binary);

    if (!input.is_open()) {
        error_message = "Unable to open file!";
        return false;
    }

    input.read(magic, MAGIC_LENGTH);
    if (memcmp(magic, POKEYSYNTH_BANK_MAGIC, MAGIC_LENGTH)) {
        error_message = "Wrong file format!";
        return false;
    }
    file_format_version = input.get();
    if (file_format_version != FILE_FORMAT_VERSION) {
        error_message = "File format version mismatch!";
        return false;
    }

    for (int p=0; p<=127; p++) {
        if (!load_instrument(input, p)) {
            input.close();
            error_message = "Loading bank failed!";
            return false;
        }
    }

    input.close();
    return true;
}

// ****************************************************************************

bool LoadSaveInstruments::SaveBank(const char *filename) {
    std::ofstream output(filename, std::ios::binary);

    if (!output.is_open()) {
        error_message = "Unable to open file!";
        return false;
    }

    output.write(POKEYSYNTH_BANK_MAGIC, MAGIC_LENGTH);
    output << (uint8_t) FILE_FORMAT_VERSION;

    for (int p=0; p<=127; p++) {
        if (!save_instrument(output, p)) {
            output.close();
            error_message = "Saving bank failed!";
            return false;
        }
    }

    output.close();
    return true;
}

// ****************************************************************************

bool LoadSaveInstruments::load_instrument(std::ifstream &input, int program) {
    struct pokey_instrument *p = &instrdata[program];
    char magic[16];
    int file_format_version = 0;
    struct pokey_instrument temp = {}; // read here first, copy when correct

    input.read(magic, MAGIC_LENGTH);
    if (memcmp(magic, POKEYSYNTH_INSTRUMENT_MAGIC, MAGIC_LENGTH)) {
        error_message = "Wrong file format!";
        return false;
    }
    file_format_version = input.get();
    if (file_format_version != FILE_FORMAT_VERSION) {
        error_message = "File format version mismatch!";
        return false;
    }

    input.read(temp.name, 64);

    temp.channels = (enum channels_type) input.get();
    temp.clock = (enum clocks) input.get();

    input.read((char *) temp.volume, INSTRUMENT_LENGTH);
    input.read((char *) temp.distortion, INSTRUMENT_LENGTH);

    temp.sustain_loop_start = input.get();
    temp.sustain_loop_end = input.get();
    temp.release_end = input.get();

    input.read((char *) temp.types, INSTRUMENT_LENGTH);

    for (int i=0; i<INSTRUMENT_LENGTH; i++) {
        temp.values[i] = read_uint32(input);
    }

    temp.types_end = input.get();
    temp.types_loop = input.get();
    temp.types_speed = input.get();

    temp.filtered_detune = read_float(input);
    temp.filtered_vol2 = read_float(input);

    temp.filtered_transpose = input.get();

    temp.bender_range = read_float(input);

    temp.mod_lfo_maxdepth = read_float(input);
    temp.mod_lfo_speed = read_float(input);

    if (input.bad()) {
        error_message = "Error while reading file!";
        return false;
    }

    memcpy(p, &temp, sizeof(struct pokey_instrument));

    return true;
}

// ****************************************************************************

bool LoadSaveInstruments::save_instrument(std::ofstream &output, int program) {
    struct pokey_instrument *p = &instrdata[program];

    output.write(POKEYSYNTH_INSTRUMENT_MAGIC, MAGIC_LENGTH);
    output << (uint8_t) FILE_FORMAT_VERSION;

    output.write(p->name, 64);

    output << (uint8_t) p->channels;
    output << (uint8_t) p->clock;

    output.write((const char *) p->volume, INSTRUMENT_LENGTH);
    output.write((const char *) p->distortion, INSTRUMENT_LENGTH);

    output << p->sustain_loop_start;
    output << p->sustain_loop_end;
    output << p->release_end;

    output.write((const char *) p->types, INSTRUMENT_LENGTH);

    for (int i=0; i<INSTRUMENT_LENGTH; i++) {
        write_uint32(output, p->values[i]);
    }

    output << p->types_end;
    output << p->types_loop;
    output << p->types_speed;

    write_float(output, p->filtered_detune);
    write_float(output, p->filtered_vol2);

    output << (uint8_t) p->filtered_transpose;

    write_float(output, p->bender_range);

    write_float(output, p->mod_lfo_maxdepth);
    write_float(output, p->mod_lfo_speed);

    output.flush();
    return output.good();
}

// ****************************************************************************
// Endian Safe reading and writing of (u)int32_t
//
uint32_t LoadSaveInstruments::read_uint32(std::ifstream &input) {
    uint32_t value;
    value  = input.get();
    value |= ((uint32_t) input.get()) <<  8;
    value |= ((uint32_t) input.get()) << 16;
    value |= ((uint32_t) input.get()) << 24;
    return value;
}

void LoadSaveInstruments::write_uint32(std::ofstream &output, uint32_t value) {
    output.put((value >>  0) & 0xff);
    output.put((value >>  8) & 0xff);
    output.put((value >> 16) & 0xff);
    output.put((value >> 24) & 0xff);
}

float LoadSaveInstruments::read_float(std::ifstream &input) {
    static_assert(sizeof(float) == sizeof(uint32_t), "Weird");
    union {
        uint32_t intval;
        float floatval;
    } invalue;
    invalue.intval = read_uint32(input);
    return invalue.floatval;
}

void LoadSaveInstruments::write_float(std::ofstream &output, float value) {
    static_assert(sizeof(float) == sizeof(uint32_t), "Weird");
    union {
        uint32_t intval;
        float floatval;
    } outvalue;
    outvalue.floatval = value;
    write_uint32(output, outvalue.intval);
}

// ****************************************************************************

bool LoadSaveInstruments::ExportInstrumentList(const char *filename) {
    std::ofstream output(filename, std::ios::binary);

    if (!output.is_open()) {
        error_message = "Unable to open file!";
        return false;
    }

    for (int p=0; p<=127; p++) {
        struct pokey_instrument *d = &instrdata[p];
        output << p << ": " << d->name << std::endl;
    }

    output.close();
    return true;
}
