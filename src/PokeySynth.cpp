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
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lv2.h"
#include "uris.h"
#include "mzpokey.h"
#include "PokeySynth.h"
#include "PokeyInstrument.h"
#include "LoadSaveInstruments.h"
#include "platform.h"

#include <map>

enum pokey_update_frequency {
    UPDATE_50HZ,
    UPDATE_100HZ,
    UPDATE_150HZ,
    UPDATE_200HZ
};

enum control_arp_enum {
    CONTROL_ARP_UP = 1,
    CONTROL_ARP_DOWN = 2
};

// ****************************************************************************

struct note {
    uint64_t time;
    uint8_t velocity;
    uint8_t program;
};

class PokeySynth {
public:
    PokeySynth(const double sample_rate, const char *bundle_path, const LV2_Feature *const *features);
    void connect_port(const uint32_t port, void *data);
    void run(const uint32_t sample_count);
    LV2_Worker_Status work(LV2_Worker_Respond_Function respond,
                           LV2_Worker_Respond_Handle handle,
                           uint32_t size,
                           const void *data);
    LV2_Worker_Status work_response(uint32_t size, const void *data);
    void SendFilenames();

private:
    // ports
    const LV2_Atom_Sequence *midi_in;
    float *audio_out;
    float *control_channels;        // react to MIDI chs 0-3,4-7,8-11,12-15
    float *control_mono_arp[4];     // pokey channels modes mono/auto-arp
    float *control_arp_speed[4];    // pokey channels arp speeds
    float *control_update_freq;     // update frequency 50/100/150/200
    float *control_overdrive_comp;  // chip overdrive compensation
    const LV2_Atom_Sequence *notify;

    // features
    LV2_URID_Map *map;
    LV2_Log_Logger logger;
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Frame notify_frame;
    LV2_Worker_Schedule *schedule;

    int sample_rate;
    int pokey_rate;
    uint64_t current_timestamp;

    struct mzpokey_context *mzp;

    // synth
    std::map<uint8_t,struct note> notes_on[4];
    uint8_t programs[4] = {};
    uint64_t last_note_times[4];

    PokeyInstrument instruments[4];

    int map_midi_to_pokey_channel(int channel);

    float ticks;
    float interval;
    void play(void);

    int auto_arp_count[4];
    int auto_arp_pos[4];

    char *bank_filename;
    char *sapr_filename;

    void StartSaprRecording(void);
    void StopSaprRecording(void);
    FILE *sapr_output;

    struct pokey_instrument instrdata[128];
};

static float intervals[4];

// ****************************************************************************

PokeySynth::PokeySynth(const double sample_rate,
                       const char *bundle_path,
                       const LV2_Feature *const *features) :
    midi_in(nullptr),
    audio_out(nullptr),
    control_channels(nullptr),
    control_mono_arp{nullptr},
    control_arp_speed{nullptr},
    control_update_freq(nullptr),
    current_timestamp(0),
    mzp(nullptr),
    last_note_times{0},
    instruments{instrdata,instrdata,instrdata,instrdata},
    ticks(0),
    interval(0),
    auto_arp_count{0},
    auto_arp_pos{0},
    sapr_output(nullptr) {

    const char *missing = lv2_features_query(
            features,
            LV2_LOG__log,           &logger.log, false,
            LV2_URID__map,          &map,        true,
            LV2_WORKER__schedule,   &schedule,   true,
            nullptr);

    lv2_log_logger_set_map(&logger, map);
    if (missing) {
        lv2_log_error(&logger, "Missing feature <%s>\n", missing);
        throw;
    }

    init_uris(map);

    pokey_rate = 1773447;
    for (unsigned int i=0; i<4; i++) {
        instruments[i].SetPokeyFrequency(pokey_rate);
    }

    this->sample_rate = sample_rate;

    mzp = mzpokey_create(pokey_rate, sample_rate, 1, 0);
    if (!mzp) throw;

    mzpokey_write_register(mzp, SKCTL, 3, 0);

    for (unsigned int i=0; i<4; i++) {
        intervals[i] = sample_rate / (50.0 + i * 50.0);
    }

    lv2_atom_forge_init(&forge, map);

    char path[4096];
    snprintf(path, 4096, "%s%c%s", bundle_path, PATH_SEPARATOR, "default.bnk");
    bank_filename = strdup(path);

    snprintf(path, 4096, "%s%c%s", bundle_path, PATH_SEPARATOR, "output.sapr");
    sapr_filename = strdup(path);

    LoadSaveInstruments io(instrdata);
    io.LoadBank(bank_filename);
}

// ****************************************************************************

void PokeySynth::connect_port(uint32_t port, void *data) {
    switch ((PortIndex) port) {
    case POKEYSYNTH_MIDI_IN:
        midi_in = (const LV2_Atom_Sequence *) data;
        break;
    case POKEYSYNTH_AUDIO_OUT:
        audio_out = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_CHANNELS:
        control_channels = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP1:
        control_mono_arp[0] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP2:
        control_mono_arp[1] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP3:
        control_mono_arp[2] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_MONO_ARP4:
        control_mono_arp[3] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED1:
        control_arp_speed[0] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED2:
        control_arp_speed[1] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED3:
        control_arp_speed[2] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_ARP_SPEED4:
        control_arp_speed[3] = (float *) data;
        break;
    case POKEYSYNTH_CONTROL_UPDATE_FREQ:
        control_update_freq = (float *) data;
        break;
    case POKEYSYNTH_NOTIFY_GUI:
        notify = (const LV2_Atom_Sequence *) data;
        break;
    case POKEYSYNTH_CONTROL_OVERDRIVE_COMP:
        control_overdrive_comp = (float *) data;
    default:
        break;
    }
}

// ****************************************************************************

int PokeySynth::map_midi_to_pokey_channel(int channel) {
    unsigned int range = (int) *control_channels;

    if (range > 3) range = 3;

    int ranges[5]  = { 0, 4, 8, 12, 16 };

    if (channel >= ranges[range] && channel < ranges[range+1])
        return channel & 3;
    else
        return -1;
}

// ****************************************************************************

void PokeySynth::play(void) {
    uint32_t taudf, taudc;

    for (int c=0; c<4; c++) {
        instruments[c].SetOverdriveCompensation(*control_overdrive_comp);
        if (notes_on[c].empty()) {  // no note, release current note (if any)
            instruments[c].Release();
            continue;
        }
        if (*control_mono_arp[c] > 0 && notes_on[c].size() > 1) { // auto-arp
            if (auto_arp_count[c] == 0) {
                int idx = auto_arp_pos[c];
                if (idx > (int) notes_on[c].size() - 1) {
                    auto_arp_pos[c] = idx = 0;
                }
                if (*control_mono_arp[c] == CONTROL_ARP_DOWN) {
                    idx = notes_on[c].size() - idx - 1;
                }
                auto i = notes_on[c].begin();
                while (--idx >= 0) i++;
                instruments[c].Start(i->first, i->second.velocity,
                                                        i->second.program);
            }
        }
        else { // Monophonic, check top key that is pressed
            auto i = notes_on[c].rbegin();  // pick top note
            if (last_note_times[c] != i->second.time) {     // different from last
                instruments[c].Start(i->first, i->second.velocity,
                                                        i->second.program);
                last_note_times[c] = i->second.time;
            }
        }
    }

    // Determine AUDCTL (channel layout, clocks, muted channels, etc...),
    // retrieve AUDF/AUDC values, and write to pokey

    uint8_t registers[9] = {};

    registers[AUDCTL] = AUDCTL_CLOCK_15KHZ;      // default to 15kHz

    enum clocks clocks[4];
    enum channels_type channels[4];

    for (unsigned int c=0; c<4; c++) {
        clocks[c]   = instruments[c].GetClock();
        if (clocks[c] == CLOCK_DIV28) { // override if one or more are 64kHz
            registers[AUDCTL] &= ~AUDCTL_CLOCK_15KHZ;
        }
        channels[c] = instruments[c].GetChannel();
    }
    // todo: mute single and linked channels that use "wrong" base clock

    // 4CH LINKED FILTERED
    //
    if (channels[0] == CHANNELS_4CH_LINKED_FILTERED ||
        channels[1] == CHANNELS_4CH_LINKED_FILTERED ||
        channels[2] == CHANNELS_4CH_LINKED_FILTERED ||
        channels[3] == CHANNELS_4CH_LINKED_FILTERED) {

        registers[AUDCTL] |= AUDCTL_LINK_12 | AUDCTL_LINK_34 |
                             AUDCTL_HIPASS_13 | AUDCTL_HIPASS_24;
        for (unsigned int c=0; c<4; c++) {
            if (channels[c] == CHANNELS_4CH_LINKED_FILTERED &&
                  clocks[c] == CLOCK_DIV1) {
                registers[AUDCTL] |= AUDCTL_CH1_HIFRQ | AUDCTL_CH3_HIFRQ;
            }
        }

        // get 32-bit audf and audc and store in registers

        int c = 0;
             if (channels[0] == CHANNELS_4CH_LINKED_FILTERED) c = 0;
        else if (channels[1] == CHANNELS_4CH_LINKED_FILTERED) c = 1;
        else if (channels[2] == CHANNELS_4CH_LINKED_FILTERED) c = 2;
        else if (channels[3] == CHANNELS_4CH_LINKED_FILTERED) c = 3;

        taudf = instruments[c].GetAudf();
        taudc = instruments[c].GetAudc();

        registers[AUDF1] = taudf;
        registers[AUDF2] = taudf >> 8;
        registers[AUDF3] = taudf >> 16;
        registers[AUDF4] = taudf >> 24;
        registers[AUDC2] = taudc;
        registers[AUDC4] = taudc >> 8;
    }

    // 2CH FILTERED
    //
    else if (channels[0] == CHANNELS_2CH_FILTERED ||
               channels[2] == CHANNELS_2CH_FILTERED) {

        registers[AUDCTL] |= AUDCTL_HIPASS_13;

        if ((channels[0] == CHANNELS_2CH_FILTERED && clocks[0] == CLOCK_DIV1) ||
            (channels[2] == CHANNELS_2CH_FILTERED && clocks[2] == CLOCK_DIV1)) {

            registers[AUDCTL] |= AUDCTL_CH1_HIFRQ | AUDCTL_CH3_HIFRQ;
        }

        // get 16-bit audf and audc and store in registers

        if (channels[0] == CHANNELS_2CH_FILTERED) {
            taudf = instruments[0].GetAudf();
            taudc = instruments[0].GetAudc();
        } else {
            taudf = instruments[2].GetAudf();
            taudc = instruments[2].GetAudc();
        }
        registers[AUDF1] = taudf;
        registers[AUDC1] = taudc;
        registers[AUDF3] = taudf >> 8;
        registers[AUDC3] = taudc >> 8;

        // we are left with channel 2 and 4, which are either filtered, or
        // single channels

        // 2+4 filtered
        if (channels[1] == CHANNELS_2CH_FILTERED ||
            channels[3] == CHANNELS_2CH_FILTERED) {

            registers[AUDCTL] |= AUDCTL_HIPASS_24;

            // get 16-bit audf and audc and store in registers

            if (channels[1] == CHANNELS_2CH_FILTERED) {
                taudf = instruments[1].GetAudf();
                taudc = instruments[1].GetAudc();
            } else {
                taudf = instruments[3].GetAudf();
                taudc = instruments[3].GetAudc();
            }
            registers[AUDF2] = taudf;
            registers[AUDC2] = taudc;
            registers[AUDF4] = taudf >> 8;
            registers[AUDC4] = taudc >> 8;
        }

        // normal
        else {
            if (channels[1] == CHANNELS_1CH) {
                registers[AUDF2] = instruments[1].GetAudf();
                registers[AUDC2] = instruments[1].GetAudc();
            }
            if (channels[3] == CHANNELS_1CH) {
                registers[AUDF4] = instruments[3].GetAudf();
                registers[AUDC4] = instruments[3].GetAudc();
            }
        }
    }

    // 2CH LINKED
    //
    else if (channels[0] == CHANNELS_2CH_LINKED ||
               channels[1] == CHANNELS_2CH_LINKED) {
        registers[AUDCTL] |= AUDCTL_LINK_12;

        if ((channels[0] == CHANNELS_2CH_LINKED && clocks[0] == CLOCK_DIV1) ||
            (channels[1] == CHANNELS_2CH_LINKED && clocks[1] == CLOCK_DIV1)) {

            registers[AUDCTL] |= AUDCTL_CH1_HIFRQ;
        }

        // get 16-bit audf and audc and store in registers

        if (channels[0] == CHANNELS_2CH_LINKED) {
            taudf = instruments[0].GetAudf();
            taudc = instruments[0].GetAudc();
        } else {
            taudf = instruments[1].GetAudf();
            taudc = instruments[1].GetAudc();
        }
        registers[AUDF1] = taudf;
        registers[AUDC1] = taudc;
        registers[AUDF2] = taudf >> 8;
        registers[AUDC2] = taudc >> 8;

        // we are left with channel 3 and 4, which are either linked or normal

        // 3+4 linked
        if (channels[2] == CHANNELS_2CH_LINKED ||
            channels[3] == CHANNELS_2CH_LINKED) {

            registers[AUDCTL] |= AUDCTL_LINK_34;

            if ((channels[2] == CHANNELS_2CH_LINKED &&
                                    clocks[2] == CLOCK_DIV1) ||
                (channels[3] == CHANNELS_2CH_LINKED &&
                                    clocks[3] == CLOCK_DIV1)) {
                registers[AUDCTL] |= AUDCTL_CH3_HIFRQ;
            }

            // get 16-bit audf and audc and store in registers

            if (channels[2] == CHANNELS_2CH_LINKED) {
                taudf = instruments[2].GetAudf();
                taudc = instruments[2].GetAudc();
            } else {
                taudf = instruments[3].GetAudf();
                taudc = instruments[3].GetAudc();
            }
            registers[AUDF3] = taudf;
            registers[AUDC3] = taudc;
            registers[AUDF4] = taudf >> 8;
            registers[AUDC4] = taudc >> 8;
        }

        // 3 and 4 normal
        else {
            if (channels[2] == CHANNELS_1CH) {
                if (clocks[2] == CLOCK_DIV1) {
                    registers[AUDCTL] |= AUDCTL_CH3_HIFRQ;
                }
                registers[AUDF3] = instruments[2].GetAudf();
                registers[AUDC3] = instruments[2].GetAudc();
            }
            if (channels[3] == CHANNELS_1CH) {
                registers[AUDF4] = instruments[3].GetAudf();
                registers[AUDC4] = instruments[3].GetAudc();
            }
        }
    }

    // 1CH NORMAL
    //
    else {
        if (clocks[0] == CLOCK_DIV1) {
            registers[AUDCTL] |= AUDCTL_CH1_HIFRQ;
        }

        registers[AUDF1] = instruments[0].GetAudf();
        registers[AUDC1] = instruments[0].GetAudc();

        // we are left with channel 2,3, and 4, which can be:

        // 2+4 filtered, 3 normal
        if (channels[1] == CHANNELS_2CH_FILTERED ||
            channels[3] == CHANNELS_2CH_FILTERED) {

            registers[AUDCTL] |= AUDCTL_HIPASS_24;

            // get 16-bit audf and audc and store in registers

            if (channels[1] == CHANNELS_2CH_FILTERED) {
                taudf = instruments[1].GetAudf();
                taudc = instruments[1].GetAudc();
            } else {
                taudf = instruments[3].GetAudf();
                taudc = instruments[3].GetAudc();
            }
            registers[AUDF2] = taudf;
            registers[AUDC2] = taudc;
            registers[AUDF4] = taudf >> 8;
            registers[AUDC4] = taudc >> 8;

            // channel 3 is normal

            if (channels[2] == CHANNELS_1CH) {
                if (clocks[2] == CLOCK_DIV1) {
                    registers[AUDCTL] |= AUDCTL_CH3_HIFRQ;
                }
                registers[AUDF3] = instruments[2].GetAudf();
                registers[AUDC3] = instruments[2].GetAudc();
            }
        }

        // 2 normal, 3+4 linked
        else if (channels[2] == CHANNELS_2CH_LINKED ||
                 channels[3] == CHANNELS_2CH_LINKED) {

            registers[AUDCTL] |= AUDCTL_LINK_34;

            if ((channels[2] == CHANNELS_2CH_LINKED &&
                                    clocks[2] == CLOCK_DIV1) ||
                (channels[3] == CHANNELS_2CH_LINKED &&
                                    clocks[3] == CLOCK_DIV1)) {
                registers[AUDCTL] |= AUDCTL_CH3_HIFRQ;
            }

            // get 16-bit audf and audc and store in registers

            if (channels[2] == CHANNELS_2CH_LINKED) {
                taudf = instruments[2].GetAudf();
                taudc = instruments[2].GetAudc();
            } else {
                taudf = instruments[3].GetAudf();
                taudc = instruments[3].GetAudc();
            }
            registers[AUDF3] = taudf;
            registers[AUDC3] = taudc;
            registers[AUDF4] = taudf >> 8;
            registers[AUDC4] = taudc >> 8;

            // channel 2 is normal
            if (channels[1] == CHANNELS_1CH) {
                registers[AUDF2] = instruments[1].GetAudf();
                registers[AUDC2] = instruments[1].GetAudc();
            }
        }

        // 2,3, and 4 normal
        else {
            if (channels[1] == CHANNELS_1CH) {
                registers[AUDF2] = instruments[1].GetAudf();
                registers[AUDC2] = instruments[1].GetAudc();
            }
            if (channels[2] == CHANNELS_1CH) {
                if (clocks[2] == CLOCK_DIV1) {
                    registers[AUDCTL] |= AUDCTL_CH3_HIFRQ;
                }
                registers[AUDF3] = instruments[2].GetAudf();
                registers[AUDC3] = instruments[2].GetAudc();
            }
            if (channels[3] == CHANNELS_1CH) {
                registers[AUDF4] = instruments[3].GetAudf();
                registers[AUDC4] = instruments[3].GetAudc();
            }
        }
    }

    for (unsigned int r=0; r<9; r++) {
        mzpokey_write_register(mzp, (enum pokey_register) r, registers[r], 0);
    }

    if (sapr_output) {
        fwrite(registers, 9, 1, sapr_output);
    }

    for (unsigned int c=0; c<4; c++) {
        instruments[c].Next();
    }

    for (unsigned int c=0; c<4; c++) {
        auto_arp_count[c]--;
        if (auto_arp_count[c] < 0) {
            auto_arp_count[c] = *control_arp_speed[c];
            auto_arp_pos[c]++;
        }
    }
}


// ****************************************************************************

void PokeySynth::run(uint32_t sample_count) {

    // Handle all MIDI events

    LV2_ATOM_SEQUENCE_FOREACH (midi_in, ev) {
        if (ev->body.type == uris.midi_MidiEvent) {
            const uint8_t *const msg = (const uint8_t *) (ev + 1);
            const uint8_t type = lv2_midi_message_type(msg);
            int channel = msg[0] & 0x0f;

            switch (type) {
            case LV2_MIDI_MSG_NOTE_ON:
                channel = map_midi_to_pokey_channel(channel);
                if (channel < 0) continue;
                notes_on[channel][msg[1]] =
                            { current_timestamp, msg[2], programs[channel] };
                break;
            case LV2_MIDI_MSG_NOTE_OFF:
                channel = map_midi_to_pokey_channel(channel);
                if (channel < 0) continue;
                notes_on[channel].erase(msg[1]);
                break;
            case LV2_MIDI_MSG_PGM_CHANGE:
                channel = map_midi_to_pokey_channel(channel);
                if (channel < 0) continue;
                programs[channel] = msg[1];
                break;
            case LV2_MIDI_MSG_BENDER:
                channel = map_midi_to_pokey_channel(channel);
                if (channel < 0) continue;
                instruments[channel].SetPitchShift((msg[2]<<7) | msg[1]);
                break;
            case LV2_MIDI_MSG_CONTROLLER:
                if (msg[1] == 0x01) {       // ModWheel CC1
                    channel = map_midi_to_pokey_channel(channel);
                    if (channel < 0) continue;
                    instruments[channel].SetModWheel(msg[2]);
                } else if (msg[1] == 0x07) {    // Volume CC7
                    channel = map_midi_to_pokey_channel(channel);
                    if (channel < 0) continue;
                    instruments[channel].SetVolumeCC(msg[2]);
                } else if (msg[1] == 120 || msg[1] == 121) { // All Notes Off
                    channel = map_midi_to_pokey_channel(channel);
                    if (channel < 0) continue;
                    notes_on[channel].clear();
                } else if (msg[1] == 14) {      // CC 14 start/stop
                    channel = map_midi_to_pokey_channel(channel);
                    if (channel < 0) continue;
                    if (msg[2] >= 64) {
                        StartSaprRecording();
                    } else {
                        StopSaprRecording();
                    }
                }
//                else {
//                    printf("CC%d\n", msg[1]);
//                }
                break;
            default:
                break;
            }
        } else if (lv2_atom_forge_is_object_type(&forge, ev->body.type)) { 

            const LV2_Atom_Object* obj =
                (const LV2_Atom_Object*) (void *) &ev->body;
            if (obj->body.otype == uris.request_filenames) {
                SendFilenames();
            } else if (obj->body.otype == uris.start_sapr) {
                StartSaprRecording();
            } else if (obj->body.otype == uris.stop_sapr) {
                StopSaprRecording();
            } else {
                schedule->schedule_work(schedule->handle,
                                        lv2_atom_total_size(&ev->body),
                                        &ev->body);
            }
        }
    }

    interval = intervals[(int)*control_update_freq];
    if (ticks > interval) { // may be up to 64 samples off, but ok for now
        ticks -= interval;
        play();
    }

    mzpokey_process_float(mzp, audio_out, sample_count);
    current_timestamp += sample_count;
    ticks += sample_count;
}

// ****************************************************************************

LV2_Worker_Status PokeySynth::work(LV2_Worker_Respond_Function respond,
                                   LV2_Worker_Respond_Handle handle,
                                   uint32_t size,
                                   const void *data) {

    const LV2_Atom_Object* obj = (const LV2_Atom_Object*) data;
    if (obj->body.otype == uris.instrument_data) {
        uint32_t program_number;
        const LV2_Atom *pgm = nullptr;
        const LV2_Atom *pgmdata = nullptr;

        lv2_atom_object_get(obj, uris.program_number, &pgm,
                                 uris.program_data, &pgmdata,
                                 0);
        if (pgm && pgmdata) {
            program_number = ((const LV2_Atom_Int *)pgm)->body;
            const LV2_Atom_Vector *vec = (const LV2_Atom_Vector *)pgmdata;
            if (vec->body.child_type == uris.atom_Int) {
                printf("dsp: received program number %d\n", program_number);
                uint8_t *data = (uint8_t *)(&vec->body + 1);
                memcpy(&instrdata[program_number], data, sizeof(struct pokey_instrument));
            }
        }
    } else if (obj->body.otype == uris.request_program) {
        uint32_t program_number;
        const LV2_Atom *pgm = nullptr;

        lv2_atom_object_get(obj, uris.program_number, &pgm,
                                 0);

        if (pgm) {
            program_number = ((const LV2_Atom_Int *)pgm)->body;
            printf("dsp: requested program number %d\n", program_number);
            respond(handle, sizeof(uint32_t), &program_number);
        }
    } else if (obj->body.otype == uris.filename_object) {
        puts("dsp: received new filename");
        const LV2_Atom_String *bpath = nullptr;
        const LV2_Atom_String *spath = nullptr;
        lv2_atom_object_get(obj, uris.bank_filename, &bpath,
                                 uris.sapr_filename, &spath,
                                 0);
        if (bpath) {
            const char *f = (const char *)LV2_ATOM_BODY(bpath);
            printf("dsp: set bank_filename to '%s'\n", f);
            if (bank_filename) {
                free(bank_filename);
            }
            bank_filename = strdup(f);
        }
        if (spath) {
            const char *f = (const char *)LV2_ATOM_BODY(spath);
            printf("dsp: set sapr_filename to '%s'\n", f);
            if (sapr_filename) {
                free(sapr_filename);
            }
            sapr_filename = strdup(f);
        }
    } else if (obj->body.otype == uris.reload_bank) {
        puts("dsp: received reload bank command");
        LoadSaveInstruments io(instrdata);
        io.LoadBank(bank_filename);
    }

    return LV2_WORKER_SUCCESS;
}

// work_response is only used for sending program data to the GUI, so we
// simply use the data pointer as a pointer to an int which is the pogram
// number.
//
LV2_Worker_Status PokeySynth::work_response(uint32_t size, const void *data) {

    uint32_t program_number = *((uint32_t *)data);
    printf("dsp: send response for pogram number %d\n", program_number);

    const uint32_t notify_capacity = notify->atom.size;
    lv2_atom_forge_set_buffer(&forge, (uint8_t *)notify,
                                                    notify_capacity);
    lv2_atom_forge_sequence_head(&forge, &notify_frame, 0);
    lv2_atom_forge_frame_time(&forge, 0);

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(&forge, &frame, 0, uris.instrument_data);
        
    lv2_atom_forge_key(&forge, uris.program_number);
    lv2_atom_forge_int(&forge, program_number);
        
    lv2_atom_forge_key(&forge, uris.program_data);
    // unpacked struct should be padded to at least 32-bits
    int xsize = sizeof(struct pokey_instrument) / sizeof(uint32_t);
    lv2_atom_forge_vector(&forge,
                          sizeof(uint32_t),
                          uris.atom_Int,
                          xsize,
                          &instrdata[program_number]);

    lv2_atom_forge_pop(&forge, &frame);
    lv2_atom_forge_pop(&forge, &notify_frame);

    return LV2_WORKER_SUCCESS;
}

void PokeySynth::SendFilenames() {
    puts("dsp: send filenames");

    const uint32_t notify_capacity = notify->atom.size;
    lv2_atom_forge_set_buffer(&forge, (uint8_t *)notify,
                                                    notify_capacity);
    lv2_atom_forge_sequence_head(&forge, &notify_frame, 0);
    lv2_atom_forge_frame_time(&forge, 0);

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(&forge, &frame, 0, uris.filename_object);

    lv2_atom_forge_key(&forge, uris.bank_filename);
    lv2_atom_forge_path(&forge, bank_filename, strlen(bank_filename));

    lv2_atom_forge_key(&forge, uris.sapr_filename);
    lv2_atom_forge_path(&forge, sapr_filename, strlen(sapr_filename));

    lv2_atom_forge_pop(&forge, &frame);
    lv2_atom_forge_pop(&forge, &notify_frame);
}

// ****************************************************************************

#define SAPR_HEADER "SAP\r\nAUTHOR \"\"\r\nNAME \"\"\r\nDATE \"\"\r\nTYPE R\r\n\r\n"

void PokeySynth::StartSaprRecording(void) {
    sapr_output = fopen(sapr_filename, "wb");
    if (sapr_output) {
        int r = fwrite(SAPR_HEADER, strlen(SAPR_HEADER), 1, sapr_output);
        if (r != 1) {
            fclose(sapr_output);
            sapr_output = nullptr;
        } else {
            const uint32_t notify_capacity = notify->atom.size;
            lv2_atom_forge_set_buffer(&forge, (uint8_t *)notify,
                                                            notify_capacity);
            lv2_atom_forge_sequence_head(&forge, &notify_frame, 0);
            lv2_atom_forge_frame_time(&forge, 0);

            LV2_Atom_Forge_Frame frame;
            lv2_atom_forge_object(&forge, &frame, 0, uris.start_sapr);

            lv2_atom_forge_pop(&forge, &frame);
            lv2_atom_forge_pop(&forge, &notify_frame);
        }
    }
}

void PokeySynth::StopSaprRecording(void) {
    if (sapr_output) {
        fclose(sapr_output);
        sapr_output = nullptr;

        const uint32_t notify_capacity = notify->atom.size;
        lv2_atom_forge_set_buffer(&forge, (uint8_t *)notify,
                                                        notify_capacity);
        lv2_atom_forge_sequence_head(&forge, &notify_frame, 0);
        lv2_atom_forge_frame_time(&forge, 0);

        LV2_Atom_Forge_Frame frame;
        lv2_atom_forge_object(&forge, &frame, 0, uris.stop_sapr);

        lv2_atom_forge_pop(&forge, &frame);
        lv2_atom_forge_pop(&forge, &notify_frame);
    }
}

// ****************************************************************************
//
// C API
//
static LV2_Handle instantiate(const LV2_Descriptor *descriptor,
                              double sample_rate,
                              const char *bundle_path,
                              const LV2_Feature *const *features) {
    PokeySynth *ps = new PokeySynth(sample_rate, bundle_path, features);
    return ps;
}

static void connect_port(LV2_Handle instance, uint32_t port, void *data) {
    PokeySynth *ps = (PokeySynth *) instance;
    if (ps) ps->connect_port(port, data);
}

static void activate(LV2_Handle instance) {
}

static void run(LV2_Handle instance, uint32_t sample_count) {
    PokeySynth *ps = (PokeySynth *) instance;
    if (ps) ps->run(sample_count);
}

static void deactivate(LV2_Handle instance) {
}

static void cleanup(LV2_Handle instance) {
    free(instance);
}

static LV2_Worker_Status work(LV2_Handle instance,
                              LV2_Worker_Respond_Function respond,
                              LV2_Worker_Respond_Handle handle,
                              uint32_t size,
                              const void *data) {
    PokeySynth *ps = (PokeySynth *) instance;
    return ps->work(respond, handle, size, data);
}

static LV2_Worker_Status work_response(LV2_Handle instance,
                                       uint32_t size,
                                       const void *data) {
    PokeySynth *ps = (PokeySynth *) instance;
    return ps->work_response(size, data);
}

static const void *extension_data(const char *uri) {
    static const LV2_Worker_Interface worker = { work, work_response, nullptr };

    if (!strcmp(uri, LV2_WORKER__interface)) {
        return &worker;
    }
    return nullptr;
}

static const LV2_Descriptor descriptor = {
    POKEYSYNTH_URI,
    instantiate,
    connect_port,
    activate,
    run,
    deactivate,
    cleanup,
    extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor(uint32_t index) {
    return !index ? &descriptor : nullptr;
}
