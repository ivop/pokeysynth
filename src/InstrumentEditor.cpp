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
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <vector>
#include <set>
#include <array>

#include "lv2.h"
#include "fltk.h"
#include "uris.h"
#include "PokeyInstrument.h"
#include "InstrumentEditor.h"
#include "LoadSaveInstruments.h"

// ****************************************************************************
// SEND INSTRUMENT DATA TO DSP
//
void InstrumentEditor::SendInstrumentToDSP(unsigned int num) {
    if (num > 127) return;

    dirty = true;

    lv2_atom_forge_set_buffer(&forge, atom_buffer, sizeof(atom_buffer));

    LV2_Atom *msg = (LV2_Atom *) lv2_atom_forge_object(&forge,
                                                       &frame,
                                                       0,
                                                       uris.instrument_data);

    lv2_atom_forge_key(&forge, uris.program_number);
    lv2_atom_forge_int(&forge, num);

    lv2_atom_forge_key(&forge, uris.program_data);
    // unpacked struct should be padded to at least 32-bits
    int size = sizeof(struct pokey_instrument) / sizeof(uint32_t);
    lv2_atom_forge_vector(&forge,
                          sizeof(uint32_t),
                          uris.atom_Int,
                          size,
                          &instrdata[num]);

    lv2_atom_forge_pop(&forge, &frame);

    write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

// ****************************************************************************
// REQUEST INSTRUMENT DATA FROM DSP
//
void InstrumentEditor::RequestInstrumentFromDSP(unsigned int num) {
    if (num > 127) return;

    lv2_atom_forge_set_buffer(&forge, atom_buffer, sizeof(atom_buffer));

    LV2_Atom *msg = (LV2_Atom *) lv2_atom_forge_object(&forge,
                                                       &frame,
                                                       0,
                                                       uris.request_program);

    lv2_atom_forge_key(&forge, uris.program_number);
    lv2_atom_forge_int(&forge, num);

    lv2_atom_forge_pop(&forge, &frame);

    write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}


// ****************************************************************************
// SEND NEW BANK FILENAME TO DSP
//
void InstrumentEditor::SendNewPathnameToDSP(void) {
    lv2_atom_forge_set_buffer(&forge, atom_buffer, sizeof(atom_buffer));

    LV2_Atom *msg = (LV2_Atom *) lv2_atom_forge_object(&forge,
                                                       &frame,
                                                       0,
                                                       uris.filename_object);

    lv2_atom_forge_key(&forge, uris.bank_filename);
    lv2_atom_forge_path(&forge, bank_filename, strlen(bank_filename));

    lv2_atom_forge_pop(&forge, &frame);
    write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

// ****************************************************************************
// TELL DSP TO RELOAD BANK FROM FILENAME IT KNOWS
//
void InstrumentEditor::SendReloadFromFileToDSP(void) {
    lv2_atom_forge_set_buffer(&forge, atom_buffer, sizeof(atom_buffer));

    LV2_Atom *msg = (LV2_Atom *) lv2_atom_forge_object(&forge,
                                                       &frame,
                                                       0,
                                                       uris.reload_bank);
    lv2_atom_forge_pop(&forge, &frame);
    write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

// ****************************************************************************
// VOLUME ENVELOPE BOX WIDGET
//
void VolBoxCB(Fl_Widget *w, void *data) {
    InstrumentEditor *ie = (InstrumentEditor *) data;
    VolBox *vb = (VolBox *) w;
    struct pokey_instrument *p = &ie->instrdata[ie->program];

    p->volume[vb->myxpos()] = 15-vb->myypos();
    ie->SendInstrumentToDSP(ie->program);
    ie->DrawProgram();
}

VolBox::VolBox(int x, int y, const char *l) : Fl_Box(x,y,12,12,l) {
    box(FL_FLAT_BOX);
    color(FL_WHITE);
    labelsize(12);
}

int VolBox::handle(int event) {
    if (event == FL_PUSH) {
        do_callback();
        return 1;
    }
    return Fl_Box::handle(event);
}

void VolBox::black() { color(FL_BLACK); redraw(); }
void VolBox::white() { color(FL_WHITE); redraw(); }
int  VolBox::myxpos(void) { return xpos; }
int  VolBox::myypos(void) { return ypos; }
void VolBox::myposition(int x, int y) { xpos = x; ypos = y; }

// ****************************************************************************
// HEXBOX WIDGET - Box With Hexadecimal Values
//
HexBox::HexBox(int x, int y, const char *l) : Fl_Box(x,y,12,12,l) {
    box(FL_FLAT_BOX);
    labelsize(12);
    labelfont(FL_COURIER);
    normal();
}

void HexBox::normal(void) {
    color(FL_WHITE);
    labelcolor(FL_BLACK);
    redraw();
};

void HexBox::inverse(void) {
    color(FL_BLACK);
    labelcolor(FL_WHITE);
    redraw();
};

HexLine::HexLine(int x, int y, const char *l) : Fl_Group(x,y,64*12,12,nullptr){
    for (int q=0; q<64; q++) {
        boxes[q] = new HexBox(q*12+x,y,"");
    }
    Fl_Box *b = new Fl_Box(x-128, y,128, 16, l);
    b->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
    b->labelsize(12);
    end();
}

void HexLine::SetValue(int index, unsigned int v) {
    if (v > 15) v = 15;
    const char *hex[16] = {
        "0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"
    };
    boxes[index]->label(hex[v]);
}

void HexLine::SetValues(uint8_t *v) {
    for (int q=0; q<64; q++) {
        SetValue(q, v[q]);
    }
}

// ****************************************************************************
// LOOP/END POSITION SLIDER
//
PositionSlider::PositionSlider(int x, int y, const char *l) : Fl_Hor_Slider(x, y, 64*12, 16, nullptr) {
    box(FL_FLAT_BOX);
    bounds(0,63);
    precision(0);
    step(1);
    has_focus = false;
    Fl_Box *b = new Fl_Box(x-128, y, 128, 16, l);
    b->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
    b->labelsize(12);
    color(fl_rgb_color(0xb0, 0xb0, 0xb0));
}

int PositionSlider::handle(int event) {
    if (event == FL_ENTER) {
        this->take_focus();
        has_focus = true;
        return 1;
    }
    if (event == FL_LEAVE) {
        has_focus = false;
        return 1;
    }
    if (event == FL_MOUSEWHEEL && has_focus) {
        int v = this->value() - Fl::event_dy();
        if (v < this->minimum()) v = this->minimum();
        if (v > this->maximum()) v = this->maximum();
        this->value(v);
        this->do_callback();
        return 1;
    }
    return Fl_Hor_Slider::handle(event);
}

// ****************************************************************************
// KEYBOARD EDITOR GROUP FOR HEXLINE(S)
//
KeyboardEditor::KeyboardEditor(int x, int y, int w, int h, HexLine **lines, int nlines) : Fl_Group(x,y,w,h) {
    cursorX = cursorY = 0;
    this->lines = lines;
    this->nlines = nlines;
    showCursor = false;
}

void KeyboardEditor::show_cursor(void) {
    lines[cursorY]->boxes[cursorX]->inverse();
}

void KeyboardEditor::hide_cursor(void) {
    lines[cursorY]->boxes[cursorX]->normal();
}

void KeyboardEditor::advance_cursor(void) {
    hide_cursor();
    if (cursorY) {
        cursorY--;
    } else {
        cursorX++;
        if (cursorX >= 64) {
            cursorX = 63;
        }
    }
    show_cursor();
}

int KeyboardEditor::handle(int event) {
    switch (event) {
    case FL_FOCUS:
        has_focus = true;
        show_cursor();
        return 1;
    case FL_UNFOCUS:
        has_focus = false;
        hide_cursor();
        return 1;
    case FL_ENTER:
        focus(this);    // make it so we receive keydown/keyup events
        return 1;
    case FL_LEAVE:
        has_focus = false;
        hide_cursor();
        return 1;
    case FL_KEYDOWN:
        if (has_focus) {
            int key = Fl::event_key();
            switch (key) {
            case FL_Left:
                hide_cursor();
                cursorX--;
                if (cursorX < 0) cursorX = 0;
                show_cursor();
                break;
            case FL_Right:
                hide_cursor();
                cursorX++;
                if (cursorX >= 64) cursorX = 63;
                show_cursor();
                break;
            case FL_Up:
                hide_cursor();
                cursorY--;
                if (cursorY < 0) cursorY = 0;
                show_cursor();
                break;
            case FL_Down:
                hide_cursor();
                cursorY++;
                if (cursorY >= nlines) cursorY = nlines-1;
                show_cursor();
                break;
            default:
                break;
            }
            char text = Fl::event_text()[0];
            last_char = text;
            if (isxdigit(text) || text == '+' || text == '-' || text == '=') {
                if (text == '=') {
                    last_char = '+';
                }
                do_callback();
            }
            return 1;
        }
        break;
    default:
        break;
    }
    return Fl_Group::handle(event);
}

// ****************************************************************************
// PROGRAM NUMBER SPINNER
//
MouseWheelSpinner::MouseWheelSpinner(int x, int y, int w, int h, const char *l)
    : Fl_Spinner(x,y,w,h,l) {
    has_focus = false;
}

int MouseWheelSpinner::handle(int event) {
    if (event == FL_ENTER) {
        this->take_focus();
        has_focus = true;
        return 1;
    }
    if (event == FL_LEAVE) {
        has_focus = false;
        return 1;
    }
    if (event == FL_MOUSEWHEEL && has_focus) {
        int v = this->value() - Fl::event_dy();
        if (v < this->minimum()) v = this->minimum();
        if (v > this->maximum()) v = this->maximum();
        this->value(v);
        this->do_callback();
        return 1;
    }
    return Fl_Spinner::handle(event);
}

// ****************************************************************************
//
// InstrumentEditor Constructor
//
InstrumentEditor::InstrumentEditor(int width,
                                   int starty,
                                   LV2UI_Write_Function write_function,
                                   LV2UI_Controller controller,
                                   LV2_URID_Map *map,
                                   const char *bundle_path,
                                   struct pokey_instrument (&instrdata)[128]) :
    instrdata(instrdata),
    write_function(write_function),
    controller(controller),
    bundle_path(bundle_path),
    bank_filename(nullptr) {

    int cury = starty, curx = 0;

    lv2_atom_forge_init(&forge, map);
    sending_or_receiving = false;
    dirty = false;

    new Label(0, cury,width, "Instrument Editor");
    cury += 20;

    // ---------- Program number / name

    display128Check = new Fl_Check_Button(16, cury, 128, 20, "Display 1-128");
    display128Check->callback(HandleDisplay128_redirect, this);
    display128Check->labelsize(display128Check->labelsize()-1);
    display128Check->clear_visible_focus();
    program_base_1 = false;

    curx = (width -768) / 2;
    programSpinner = new MouseWheelSpinner(curx,cury, 64, 24);
    programSpinner->minimum(0);
    programSpinner->maximum(127);
    programSpinner->step(1);
    programSpinner->value(0);
    program = 0;
    programSpinner->textfont(FL_COURIER);

    programSpinner->callback(HandleProgramSpinner_redirect, this);

    programName = new InputField(curx+80, cury, 688, 24);
    programName->textfont(FL_COURIER);
    programName->callback(HandleProgramName_redirect, this);

    auto clrinstr = new Fl_Button(width-128-16, cury, 128, 20,
                                                        "Clear Instrument");
    clrinstr->labelsize(clrinstr->labelsize()-1);
    clrinstr->clear_visible_focus();
    clrinstr->callback(HandleClearInstrument_redirect, this);

    cury += 32;

    // ---------- Channels

    curx = (width - 768) / 2;
    Fl_Group *channelsGroup = new Fl_Group(curx, cury, 768, 24);
    channelsGroup->begin(); {
        const char *t[4] = {
            "8-bit Channel",
            "2CH Linked",
            "2CH Filter",
            "4CH Linked + Filter" };
        const char *u[4] = { "", "(1+2 or 3+4)", "(1+3 or 2+4)", "(1+2+3+4)" };
        Fl_Box *minitext;
        for (int c=0; c<4; c++) {
            channelsButtons[c] = new FlatRadioButton(curx+c*192, cury,
                                                            192, 24, t[c]);
            channelsButtons[c]->callback(HandleChannelsRadios_redirect, this);
            minitext = new Fl_Box(curx+c*192, cury+24, 192, 12, u[c]);
            minitext->labelsize(8);
            minitext->labelfont(FL_ITALIC);
        }
    };
    channelsGroup->end();
    cury += 24 + 8 + 8;

    // ---------- Clocks

    curx = (width - 384) / 2;
    Fl_Group *clocksGroup = new Fl_Group(curx, cury, 384, 24);
    clocksGroup->begin(); {
        const char *t[3] = { "15kHz", "64kHz", "1.8MHz" };
        const char *u[3] = { "", "", "1 or 3" };
        Fl_Box  *minitext;
        for (int c=0; c<3; c++) {
            clocksButtons[c] = new FlatRadioButton(curx+c*128, cury,
                                                            128, 24, t[c]);
            clocksButtons[c]->callback(HandleClocksRadios_redirect, this);
            minitext = new Fl_Box(curx+c*128, cury+24, 128, 12, u[c]);
            minitext->labelsize(8);
            minitext->labelfont(FL_ITALIC);
        }
    }
    clocksGroup->end();

    cury += 24 + 8 + 8;

    // ---------- Envelope

    curx = (width - (64*12)) / 2;

    for (int p=0; p<64; p++) {
        for (int q=0; q<16; q++) {
            envelopeBoxes[p][q] = new VolBox(curx+p*12, cury+q*12);
            envelopeBoxes[p][q]->callback(VolBoxCB, this);
            envelopeBoxes[p][q]->myposition(p, q);
        }
    }

    // ---------- ADSR

    attackSpin = new Fl_Spinner(32, cury, 40, 24, "A");
    attackSpin->minimum(1);
    attackSpin->maximum(30);
    attackSpin->step(1);
    attackSpin->value(4);
    decaySpin = new Fl_Spinner(32+64, cury, 40, 24, "D");
    decaySpin->minimum(1);
    decaySpin->maximum(30);
    decaySpin->step(1);
    decaySpin->value(8);
    sustainSpin = new Fl_Spinner(32, cury+32, 40, 24, "S");
    sustainSpin->minimum(0);
    sustainSpin->maximum(15);
    sustainSpin->step(1);
    sustainSpin->value(7);
    releaseSpin= new Fl_Spinner(32+64, cury+32, 40, 24, "R");
    releaseSpin->minimum(1);
    releaseSpin->maximum(30);
    releaseSpin->step(1);
    releaseSpin->value(14);

    int xx = 16;
    for (int i=0; i<3; i++) {
        const char *t[3] = { "AD", "ADR", "ADSR" };
        const int w[3] = { 30, 42, 56 };
        adsrButtons[i] = new Fl_Button(xx, cury+64, w[i], 20, t[i]);
        adsrButtons[i]->callback(HandleADSR_redirect, this);
        adsrButtons[i]->clear_visible_focus();
        xx += w[i];
    }

    // ---------- Distortions

    xx = curx + 64*12 + 16;

    for (int d=0; d<DIST_COUNT; d++) {
        const char *t[5] = {
            "0 - Pure",
            "1 - Noise",
            "2 - Buzzy Bass",
            "3 - Gritty Bass",
            "4 - Poly5 Square"
        };
        distButtons[d] = new Fl_Button(xx, cury+d*20, 128, 19, t[d]);
        distButtons[d]->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        distButtons[d]->callback(HandleDistButtons_redirect, this);
        distButtons[d]->clear_visible_focus();
    }

    // ---------- Volume / Distortion lines

    cury += 16*12;
    volumeValues = new HexLine(curx, cury, "Volume");
    editVolumeValues = new KeyboardEditor(curx, cury, 64*12, 12, &volumeValues, 1);
    editVolumeValues->add(volumeValues);
    editVolumeValues->callback(HandleKeyboardEditor_redirect, this);
    editVolumeValues->end();

    auto volclr = new Fl_Button(16, cury, 48, 16, "Clear");
    volclr->labelsize(volclr->labelsize()-1);
    volclr->clear_visible_focus();
    volclr->callback(HandleVolumeClear_redirect, this);

    cury += 12;
    distValues = new HexLine(curx, cury, "Distortion");
    editDistValues = new KeyboardEditor(curx, cury, 64*12, 12, &distValues, 1);
    editDistValues->add(distValues);
    editDistValues->callback(HandleKeyboardEditor_redirect, this);
    editDistValues->end();
    cury += 12;

    susLoopStart = new PositionSlider(curx, cury, "Sustain Start");
    susLoopStart->callback(HandleSusLoopStart_redirect, this);
    cury += 16;
    susLoopEnd = new PositionSlider(curx, cury, "Sustain End");
    susLoopEnd->callback(HandleSusLoopEnd_redirect, this);
    cury += 16;
    envEnd = new PositionSlider(curx, cury, "Release End");
    envEnd->callback(HandleEnvEnd_redirect, this);
    cury += 16;

    // ---------- Types and Values

    cury += 8;
    typesLine = new HexLine(curx, cury, "Type");
    editTypesLine = new KeyboardEditor(curx, cury, 64*12, 12, &typesLine, 1);
    editTypesLine->add(typesLine);
    editTypesLine->callback(HandleKeyboardEditor_redirect, this);
    editTypesLine->end();

    for (int t=0; t<8; t++) {
        const char *l[8] = { "LSB", ".", ".", ".", ".", ".", ".", "MSB" };
        typeValues[t] = new HexLine(curx, cury+14+t*12, l[t]);
    }
    editTypeValues = new KeyboardEditor(curx, cury+14, 64*12, 8*12, &typeValues[0], 8);
    for (int t=0; t<8; t++) {
        editTypeValues->add(typeValues[t]);
    }
    editTypeValues->callback(HandleKeyboardEditor_redirect, this);
    editTypeValues->end();

    // ---------- Types Helper

    Fl_Box *tbx;
    for (int t=0; t<4; t++) {
        const char *l[4] = {
            "0 - MIDI Note",
            "1 - MIDI +/- Note",
            "2 - MIDI +/- Cents",
            "3 - Fixed Divider"
        };
        tbx = new Fl_Box(xx, cury+t*12, 128, 12, l[t]);
        tbx->labelsize(12);
        tbx->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    }

    // ---------- Chord Helper

    Fl_Group *chordTypeGroup = new Fl_Group(16, cury, 96, 3*16);
    for (int c=0; c<6; c++) {
        const char *t[6] = { "Major", "Minor", "sus2", "sus4", "Aug", "Dim" };
        int x = 16 + (c&1)*48;
        int y = cury + (c>>1)*16;
        chordType[c] = new FlatRadioButton(x, y, 48, 16, t[c]);
    }
    chordType[0]->setonly();
    chordTypeGroup->end();

    Fl_Group *chordAddGroup = new Fl_Group(16, cury+4+3*16, 96, 4*16);
    for (int c=0; c<7; c++) {
        const char *t[7] = { "-", "7", "Maj7", "6", "add9", "add11", "add13" };
        int x = 16 + (c&1)*48;
        int y = cury + 4 + 3*16 + (c>>1)*16;
        chordAdd[c] = new FlatRadioButton(x, y, 48, 16, t[c]);
    }
    chordAdd[0]->setonly();
    chordAddGroup->end();

    Fl_Group *chordInversionGroup = new Fl_Group(16, cury+8+7*16, 96, 2*16);
    for (int c=0; c<4; c++) {
        const char *t[4] = { "Root", "1st", "2nd", "3rd" };
        int x = 16 + (c&1)*48;
        int y = cury + 8 + 7*16 + (c>>1)*16;
        chordInversion[c] = new FlatRadioButton(x, y, 48, 16, t[c]);
    }
    chordInversion[0]->setonly();
    chordInversionGroup->end();

    Fl_Group *chordArpDirGroup = new Fl_Group(16, cury+12+9*16, 96, 1*16);
    for (int c=0; c<2; c++) {
        const char *t[2] = { "Up", "Down" };
        int x = 16 + c*48;
        int y = cury + 12 + 9*16;
        chordArpDirection[c] = new FlatRadioButton(x, y, 48, 16, t[c]);
    }
    chordArpDirection[0]->setonly();
    chordArpDirGroup->end();

    Fl_Button *chordButton = new Fl_Button(16, cury+16+10*16, 96, 16, "Chord");
    chordButton->labelsize(chordButton->labelsize()-1);
    chordButton->callback(HandleChordsButton_redirect, this);
    chordButton->clear_visible_focus();

    // ---------- Types Loop

    cury += 14 + 8*12;
    typesLoopStart = new PositionSlider(curx, cury, "Loop");
    typesLoopStart->callback(HandleTypesLoopStart_redirect, this);

    typesLoopEnd = new PositionSlider(curx, cury+16, "End");
    typesLoopEnd->callback(HandleTypesLoopEnd_redirect, this);

    typesSpeed = new MouseWheelSlider(xx, cury, 128, 16, "Speed");
    typesSpeed->bounds(0,15);
    typesSpeed->step(1);
    typesSpeed->labelsize(12);
    typesSpeed->callback(HandleTypesLoopSpeed_redirect, this);

    auto typclr = new Fl_Button(width-16-48, cury-24, 48, 16, "Clear");
    typclr->labelsize(typclr->labelsize()-1);
    typclr->clear_visible_focus();
    typclr->callback(HandleTypesClear_redirect, this);

    cury += 3*16;

    // Filter Options

    filterDetune = new MouseWheelSlider(curx, cury,
                                    256-16, 20, "Filter Detune (cents)");
    filterDetune->bounds(-100,100);
    filterDetune->precision(0);
    filterDetune->step(1);
    filterDetune->value(0);
    filterDetune->labelsize(12);
    filterDetune->callback(HandleFilterDetune_redirect, this);

    filterVol2 = new MouseWheelSlider(curx+256, cury,
                                    256-16, 20, "Filter Detune Volume (%)");
    filterVol2->bounds(0,100);
    filterVol2->precision(0);
    filterVol2->step(1);
    filterVol2->value(50);
    filterVol2->labelsize(12);
    filterVol2->callback(HandleFilterVol2_redirect, this);

    filterTranspose = new Fl_Check_Button(curx+512, cury,
                                256-16, 20, "Filter Transpose Octave Down");
    filterTranspose->clear_visible_focus();
    filterTranspose->callback(HandleFilterTranspose_redirect, this);

    benderRange = new MouseWheelSlider(curx, cury+40,
                                256-16, 20, "Pitchwheel +/- Range (cents)");
    benderRange->bounds(0,1200);
    benderRange->precision(0);
    benderRange->step(100);
    benderRange->value(200);
    benderRange->labelsize(12);
    benderRange->callback(HandleBenderRange_redirect, this);

    modwheelDepth = new MouseWheelSlider(curx+256, cury+40,
                            256-16, 20, "Modwheel LFO Maximum Depth (cents)");
    modwheelDepth->bounds(0,200);
    modwheelDepth->precision(0);
    modwheelDepth->step(1);
    modwheelDepth->value(100);
    modwheelDepth->labelsize(12);
    modwheelDepth->callback(HandleModwheelDepth_redirect, this);

    modwheelSpeed = new MouseWheelSlider(curx+512, cury+40,
                            256, 20, "Modwheel LFO Speed (degrees/frame)");
    modwheelSpeed->bounds(0,360);
    modwheelSpeed->precision(0);
    modwheelSpeed->step(1);
    modwheelSpeed->value(90);
    modwheelSpeed->labelsize(12);
    modwheelSpeed->callback(HandleModwheelSpeed_redirect, this);

    auto botclr = new Fl_Button(width-16-48, cury+40, 48, 16, "Clear");
    botclr->clear_visible_focus();
    botclr->labelsize(botclr->labelsize()-1);
    botclr->callback(HandleBottomClear_redirect, this);

    cury += 80;

    // ---------- Buttons

    const int butwidth = 144;
    Fl_Button *tb;
    curx = (width - 16 - 5*(butwidth+8))/2;

#if 0
    progressBar = new Fl_Progress(curx, cury, 128, 24);
    progressBar->minimum(0);
    progressBar->maximum(127);
    progressBar->value(127);
    progressBar->color2(fl_rgb_color(0x20,0x40,0x80));
    curx += progressBar->w() + 8;

    tb = new Fl_Button(curx, cury, butwidth, 24, "Request All");
    tb->callback(RequestAllButtonCB_redirect, this);
    curx += butwidth + 8;
    tb = new Fl_Button(curx, cury, butwidth, 24, "Request Current");
    tb->callback(RequestCurButtonCB_redirect, this);
    curx += butwidth + 8;
#endif

    tb = new Fl_Button(curx, cury, butwidth, 24, "Export List");
    tb->clear_visible_focus();
    tb->callback(HandleExportList_redirect, this);
    curx += butwidth + 8;

    tb = new Fl_Button(curx, cury, butwidth, 24, "Load Instrument");
    tb->clear_visible_focus();
    tb->callback(HandleLoadInstrument_redirect, this);
    curx += butwidth + 8;
    tb = new Fl_Button(curx, cury, butwidth, 24, "Save Instrument");
    tb->clear_visible_focus();
    tb->callback(HandleSaveInstrument_redirect, this);
    curx += butwidth + 8;

    tb = new Fl_Button(curx, cury, butwidth, 24, "Load Bank");
    tb->clear_visible_focus();
    tb->callback(HandleLoadBank_redirect, this);
    curx += butwidth + 8;
    tb = new Fl_Button(curx, cury, butwidth, 24, "Save Bank");
    tb->clear_visible_focus();
    tb->callback(HandleSaveBank_redirect, this);
    curx += butwidth + 8;
}

// ****************************************************************************
// PROGRAM SELECTION SPINNER AND NAME
//
void InstrumentEditor::HandleDisplay128_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleDisplay128(w,data);
}

void InstrumentEditor::HandleDisplay128(Fl_Widget *w, void *data) {
    int v = programSpinner->value();
    if (display128Check->value()) {
        program_base_1 = true;
        programSpinner->minimum(1);
        programSpinner->maximum(128);
        programSpinner->value(v+1);
    } else {
        program_base_1 = false;
        programSpinner->minimum(0);
        programSpinner->maximum(127);
        programSpinner->value(v-1);
    }
    programSpinner->redraw();
}

void InstrumentEditor::HandleProgramSpinner_redirect(Fl_Widget *w, void *data){
    ((InstrumentEditor *) data)->HandleProgramSpinner((Fl_Spinner *)w,data);
}

void InstrumentEditor::HandleProgramSpinner(Fl_Spinner *w, void *data) {
    program = w->value() - program_base_1;
    DrawProgram();
}

void InstrumentEditor::HandleProgramName_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleProgramName(w,data);
}

void InstrumentEditor::HandleProgramName(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    char temp[64];
    memset(temp, 0, 64);
    snprintf(temp, 64, "%s", programName->value());
    memcpy(p->name, temp, 64);
    SendInstrumentToDSP(program);
}

// ****************************************************************************
// CHANNELS RADIO BUTTONS
//
void InstrumentEditor::HandleChannelsRadios_redirect(Fl_Widget *w, void *data){
    ((InstrumentEditor *) data)->HandleChannelsRadios(w,data);
}

void InstrumentEditor::HandleChannelsRadios(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];

    int which = 0;
    for (which=0; which<4; which++) if (channelsButtons[which]->value()) break;

    p->channels = (channels_type) which;
    SendInstrumentToDSP(program);
}

// ****************************************************************************
// CLOCKS RADIO BUTTONS
//
void InstrumentEditor::HandleClocksRadios_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleClocksRadios(w,data);
}

void InstrumentEditor::HandleClocksRadios(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];

    int which = 0;
    for (which=0; which<3; which++) if (clocksButtons[which]->value()) break;

    p->clock = (enum clocks) which;
    SendInstrumentToDSP(program);
}

// ****************************************************************************
// SUSTAIN AND RELEASE SLIDERS
//
void InstrumentEditor::HandleSusLoopStart_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleSusLoopStart(w,data);
}

void InstrumentEditor::HandleSusLoopStart(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];

    if (susLoopStart->value() > susLoopEnd->value()) {
        susLoopStart->value(susLoopEnd->value());
    }
    p->sustain_loop_start = susLoopStart->value();
    SendInstrumentToDSP(program);
}

void InstrumentEditor::HandleSusLoopEnd_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleSusLoopEnd(w,data);
}

void InstrumentEditor::HandleSusLoopEnd(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];

    if (susLoopEnd->value() < susLoopStart->value()) {
        susLoopEnd->value(susLoopStart->value());
    }
    p->sustain_loop_end  = susLoopEnd->value();
    SendInstrumentToDSP(program);
}

void InstrumentEditor::HandleEnvEnd_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleEnvEnd(w,data);
}

void InstrumentEditor::HandleEnvEnd(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    p->release_end = envEnd->value();
    SendInstrumentToDSP(program);
}

// ****************************************************************************
// TYPES LOOP SLIDERS
//
void InstrumentEditor::HandleTypesLoopStart_redirect(Fl_Widget *w, void *data){
    ((InstrumentEditor *) data)->HandleTypesLoopStart(w,data);
}

void InstrumentEditor::HandleTypesLoopStart(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    if (typesLoopStart->value() > typesLoopEnd->value()) {
        typesLoopStart->value(typesLoopEnd->value());
    }
    p->types_loop = typesLoopStart->value();
    SendInstrumentToDSP(program);
}

void InstrumentEditor::HandleTypesLoopEnd_redirect(Fl_Widget *w, void *data){
    ((InstrumentEditor *) data)->HandleTypesLoopEnd(w,data);
}

void InstrumentEditor::HandleTypesLoopEnd(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    if (typesLoopEnd->value() < typesLoopStart->value()) {
        typesLoopEnd->value(typesLoopStart->value());
    }
    p->types_end = typesLoopEnd->value();
    SendInstrumentToDSP(program);
}

void InstrumentEditor::HandleTypesLoopSpeed_redirect(Fl_Widget *w, void *data){
    ((InstrumentEditor *) data)->HandleTypesLoopSpeed(w,data);
}

void InstrumentEditor::HandleTypesLoopSpeed(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    p->types_speed = typesSpeed->value();
    SendInstrumentToDSP(program);
}

// ****************************************************************************
// FILTER OPTIONS
//
void InstrumentEditor::HandleFilterDetune_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleFilterDetune(w,data);
}

void InstrumentEditor::HandleFilterDetune(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    p->filtered_detune = filterDetune->value();
    SendInstrumentToDSP(program);
}

void InstrumentEditor::HandleFilterVol2_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleFilterVol2(w,data);
}

void InstrumentEditor::HandleFilterVol2(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    p->filtered_vol2  = filterVol2->value() / 100.0;
    SendInstrumentToDSP(program);
}

void InstrumentEditor::HandleFilterTranspose_redirect(Fl_Widget *w, void *data){
    ((InstrumentEditor *) data)->HandleFilterTranspose(w,data);
}

void InstrumentEditor::HandleFilterTranspose(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    p->filtered_transpose = filterTranspose->value();
    SendInstrumentToDSP(program);
}

// ****************************************************************************
// BENDER AND MODSHEEL PARAMETERS
//
void InstrumentEditor::HandleBenderRange_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleBenderRange(w,data);
}

void InstrumentEditor::HandleBenderRange(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    p->bender_range = benderRange->value();
    SendInstrumentToDSP(program);
}

void InstrumentEditor::HandleModwheelDepth_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleModwheelDepth(w,data);
}

void InstrumentEditor::HandleModwheelDepth(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    p->mod_lfo_maxdepth = modwheelDepth->value();
    SendInstrumentToDSP(program);
}

void InstrumentEditor::HandleModwheelSpeed_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleModwheelSpeed(w,data);
}

void InstrumentEditor::HandleModwheelSpeed(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    p->mod_lfo_speed = modwheelSpeed->value() / 360.0 * 2 * M_PI;
    SendInstrumentToDSP(program);
}

// ****************************************************************************
// ADSR BUTTON - GENRATE ENVELOPE
//
void InstrumentEditor::HandleADSR_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleADSR(w,data);
}

void InstrumentEditor::HandleADSR(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    float A = attackSpin->value();
    float D = decaySpin->value();
    float S = sustainSpin->value();
    float R = releaseSpin->value();

    int which, i, pos = 0;

    for (which=0; which<3; which++) {
        if (adsrButtons[which] == w) break;
    }

    for (i=0; i<(int)A; i++) {
        p->volume[pos+i] = round(15.0 / A * (i + 1));
    }
    pos += i;

    if (which == 1 || which == 2) {     // ADR and ADSR
        for (i=0; i<(int)D; i++) {
            p->volume[pos+i] = round(15.0 - ((15.0 - S) / D * (i + 1)));
        }
        pos += i;

        if (which == 2) {   // ADSR
            p->sustain_loop_start = p->sustain_loop_end = pos - 1;
        }

        for (i=0; i<(int)R; i++) {
            if ((pos + i) >= 64) break;
            p->volume[pos+i] = round(S - (S / R * (i + 1)));
        }
        pos += i - 1;

        if (which == 1) {   // ADR
            p->sustain_loop_start = p->sustain_loop_end = pos;
        }

        p->release_end = pos;
    } else {                            // AD
        for (i=0; i<(int)D; i++) {
            if ((pos + i) >= 64) break;
            p->volume[pos+i] = round(15.0 - (15.0 / D * (i + 1)));
        }
        pos += i - 1;
        p->release_end = p->sustain_loop_start = p->sustain_loop_end = pos;
    }

    for (; pos<64; pos++) p->volume[pos] = 0;

    SendInstrumentToDSP(program);
    DrawProgram();
}

// ****************************************************************************
// DISTORTION BUTTONS, FILL DIST TABLE
//
void InstrumentEditor::HandleDistButtons_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleDistButtons(w, data);
}

void InstrumentEditor::HandleDistButtons(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    int d, i;
    for (d=0; d<DIST_COUNT; d++) {
        if (w == distButtons[d]) break;
    }
    for (i=0; i<=p->release_end; i++) {
        p->distortion[i] = (enum distortions) d;
    }
    for (; i<64; i++) {
        p->distortion[i] = (enum distortions) 0;
    }
    SendInstrumentToDSP(program);
    DrawProgram();
}

// ****************************************************************************
// KEYBOARD EDITOR HANDLER
// Just use one callback for all edits
//
void InstrumentEditor::HandleKeyboardEditor_redirect(Fl_Widget *w, void *data){
    ((InstrumentEditor *)data)->HandleKeyboardEditor((KeyboardEditor *)w, data);
}

void InstrumentEditor::HandleKeyboardEditor(KeyboardEditor *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    char key = tolower(w->last_char);
    uint32_t v;

    if (isxdigit(key)) {
        v = key > '9' ? key - 'a' + 10 : key - '0';
        if (w == editVolumeValues) {
            p->volume[w->cursorX] = v;
        } else if (w == editDistValues) {
            if (v >= DIST_COUNT) return;
            p->distortion[w->cursorX] = (enum distortions) v;
        } else if (w == editTypesLine) {
            if (v >= TYPES_COUNT) return;
            p->types[w->cursorX] = v;
        } else if (w == editTypeValues) {
            int shift = w->cursorY * 4;
            uint32_t mask = 0xf << shift;
            v <<= shift;
            printf("debug: mask = %08x, v=%08x\n", ~mask, v);
            p->values[w->cursorX] &= ~mask;
            p->values[w->cursorX] |= v;
        }
        w->advance_cursor();
    } else {
        if (w == editVolumeValues) {
            v = p->volume[w->cursorX];
            v += key == '+' ? 1 : -1;
            v &= 0x0f;
            p->volume[w->cursorX] = v;
        } else if (w == editDistValues) {
            v = p->distortion[w->cursorX];
            v += key == '+' ? 1 : -1;
            v &= 0x0f;
            if (v >= DIST_COUNT) return;
            p->distortion[w->cursorX] = (enum distortions) v;
        } else if (w == editTypesLine) {
            v = p->types[w->cursorX];
            v += key == '+' ? 1 : -1;
            v &= 0x0f;
            if (v >= TYPES_COUNT) return;
            p->types[w->cursorX] = v;
        } else if (w == editTypeValues) {
            v = p->values[w->cursorX];
            v += key == '+' ? 1 : -1;
            p->values[w->cursorX] = v;
        }
    }
    SendInstrumentToDSP(program);
    DrawProgram();
}

// ****************************************************************************
// CHORD ARPEGGIATOR BUTTON
//
void InstrumentEditor::HandleChordsButton_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleChordsButton(w, data);
}

void InstrumentEditor::HandleChordsButton(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    static std::vector<int> chord[6] = {
        { 0, 4, 7 },    // Major
        { 0, 3, 7 },    // Minor
        { 0, 2, 7 },    // Suspended 2nd
        { 0, 5, 7 },    // Suspended 4th
        { 0, 4, 8 },    // Augmented
        { 0, 3, 6 }     // Diminished
    };
    static std::array<int,7> addNote = {{
        0,  // Root
        10, // 7th
        11, // Major 7th
        9,  // 6th
        14, // 9th
        17, // 11th
        21  // 13th
    }};
    static std::vector<int> inversion[4] = {
        { 0,   0,   0,   0 },   // Root position
        { 0, -12, -12, -12 },   // 1st Inversion
        { 0,   0, -12, -12 },   // 2nd Inversion
        { 0,   0,   0, -12 }    // 3rd Inversion
    };

    int whichChord = 0;
    int whichAdd = 0;
    int whichInversion = 0;

    for (int c=0; c<6; c++) {
        if (chordType[c]->value()) {
            whichChord = c;
            break;
        }
    }
    for (int a=0; a<7; a++) {
        if (chordAdd[a]->value()) {
            whichAdd = a;
            break;
        }
    }
    for (int i=0; i<4; i++) {
        if (chordInversion[i]->value()) {
            whichInversion = i;
            break;
        }
    }

    // clear all types data
    for (int i=0; i<64; i++) {
        p->types[i] = 0;
        p->values[i] = 0;
    }

    std::vector<int> input = chord[whichChord];
    if (whichAdd) {
        input.push_back(addNote[whichAdd]);
    }

    std::set<int> result;

    for (auto it1 = input.begin(), it2 = inversion[whichInversion].begin();
            it1 != input.end(); it1++, it2++) {
        result.insert(*it1 + *it2);
    }

    int i, d;
    if (chordArpDirection[0]->value()) {    // Up
        i = 0;
        d = 1;
    } else {                                // Down
        i = result.size() - 1;
        d = -1;
    }
    for (auto x : result) {
        p->types[i] = TYPE_NOTE_PLUS_NOTE;
        p->values[i] = x;
        i += d;
    }
    p->types_loop = 0;
    p->types_end = i - 1;

    SendInstrumentToDSP(program);
    DrawProgram();
}

// ****************************************************************************
// CLEAR VOLUME, TYPES, ETC...
//
void InstrumentEditor::HandleVolumeClear_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleVolumeClear(w, data);
}

void InstrumentEditor::HandleVolumeClear(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    for (auto &v : p->volume) { v = 0; }
    for (auto &d : p->distortion) { d = DIST_PURE; }
    p->sustain_loop_start = p->sustain_loop_end = p->release_end = 0;
    SendInstrumentToDSP(program);
    DrawProgram();
}

void InstrumentEditor::HandleTypesClear_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleTypesClear(w, data);
}

void InstrumentEditor::HandleTypesClear(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    for (auto &t : p->types) { t = 0; }
    for (auto &v : p->values) { v = 0; }
    p->types_loop = p->types_end = p->types_speed = 0;
    SendInstrumentToDSP(program);
    DrawProgram();
}

void InstrumentEditor::HandleBottomClear_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleBottomClear(w, data);
}

void InstrumentEditor::HandleBottomClear(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    p->filtered_detune = p->filtered_vol2 = p->filtered_transpose = 0;
    p->bender_range = p->mod_lfo_speed = p->mod_lfo_maxdepth = 0;
    SendInstrumentToDSP(program);
    DrawProgram();
}

void InstrumentEditor::HandleClearInstrument_redirect(Fl_Widget *w, void *data){
    ((InstrumentEditor *) data)->HandleClearInstrument(w, data);
}

void InstrumentEditor::HandleClearInstrument(Fl_Widget *w, void *data) {
    struct pokey_instrument *p = &instrdata[program];
    for (auto &v : p->volume) { v = 0; }
    for (auto &d : p->distortion) { d = DIST_PURE; }
    p->sustain_loop_start = p->sustain_loop_end = p->release_end = 0;
    for (auto &t : p->types) { t = 0; }
    for (auto &v : p->values) { v = 0; }
    p->types_loop = p->types_end = p->types_speed = 0;
    p->filtered_detune = p->filtered_vol2 = p->filtered_transpose = 0;
    p->bender_range = p->mod_lfo_speed = p->mod_lfo_maxdepth = 0;
    p->channels = CHANNELS_1CH;
    p->clock = CLOCK_DIV114;
    memset(p->name, 0, sizeof(p->name));
    SendInstrumentToDSP(program);
    DrawProgram();
}

#if 0
// ****************************************************************************
// REQUEST ALL INSTRUMENTS BUTTON
//
void InstrumentEditor::RequestAllButtonCB_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->RequestAllButtonCB(w, data);
}

void InstrumentEditor::RequestAllButtonCB(Fl_Widget *w, void *data) {
    if (sending_or_receiving) return;

    sending_or_receiving = true;

    for (int i=0; i<128; i++) {
        progressBar->value(i);
        RequestInstrumentFromDSP(i);
        Fl::check();
        usleep(50000);
    }

    sending_or_receiving = false;
}

// ****************************************************************************
// REQUEST CURRENT INSTRUMENT BUTTON
//
void InstrumentEditor::RequestCurButtonCB_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->RequestCurButtonCB(w, data);
}

void InstrumentEditor::RequestCurButtonCB(Fl_Widget *w, void *data) {
    if (sending_or_receiving) return;
    sending_or_receiving = true;
    RequestInstrumentFromDSP(program);
    progressBar->value(127);
    sending_or_receiving = false;
}
#endif

// ****************************************************************************
// DRAW CURRENT PROGRAM
//
void InstrumentEditor::DrawProgram(void) {
    struct pokey_instrument *p = &instrdata[program];

    programName->value(p->name);
    for (int c=0; c<4; c++) {
        if (c == p->channels) {
            channelsButtons[c]->setonly();
        } else {
            channelsButtons[c]->clear();
        }
    }
    for (int c=0; c<3; c++) {
        if (c == p->clock) {
            clocksButtons[c]->setonly();
        } else {
            clocksButtons[c]->clear();
        }
    }
    for (int x=0; x<64; x++) {
        int y = 0;
        for (; y<=p->volume[x]; y++) {
            envelopeBoxes[x][15-y]->black();
        }
        for (; y<16; y++) {
            envelopeBoxes[x][15-y]->white();
        }
    }
    volumeValues->SetValues(&p->volume[0]);
    distValues->SetValues((uint8_t *)&p->distortion[0]);
    susLoopStart->value(p->sustain_loop_start);
    susLoopEnd->value(p->sustain_loop_end);
    envEnd->value(p->release_end);

    typesLine->SetValues(p->types);
    for (int n=0; n<8; n++) {
        uint8_t values[64] = {0};
        for (int x=0; x<64; x++) {
            values[x] = (p->values[x] >> (n*4)) & 0xf;
        }
        typeValues[n]->SetValues(values);
    }

    typesLoopStart->value(p->types_loop);
    typesLoopEnd->value(p->types_end);
    typesSpeed->value(p->types_speed);

    filterDetune->value(p->filtered_detune);
    filterVol2->value(p->filtered_vol2 * 100);
    filterTranspose->value(p->filtered_transpose);

    benderRange->value(p->bender_range);
    modwheelDepth->value(p->mod_lfo_maxdepth);
    modwheelSpeed->value(p->mod_lfo_speed / (2*M_PI) * 360);
}

// ****************************************************************************
// LOAD/SAVE INSTRUMENT(S)
//
void InstrumentEditor::HandleLoadInstrument_redirect(Fl_Widget *w, void *data){
    ((InstrumentEditor *) data)->HandleLoadInstrument(w, data);
}

void InstrumentEditor::HandleLoadInstrument(Fl_Widget *w, void *data) {
    LoadSaveInstruments io(instrdata);
    char *filename = fl_file_chooser("Load Instrument", "*.ins", bundle_path);

    if (filename) {
        if (!io.LoadInstrument(program, filename)) {
            fl_message("Error: %s", io.error_message);
        }
    }
    dirty = true;
    SendInstrumentToDSP(program);
    DrawProgram();
}

// ----------------------------------------------------------------------------

void InstrumentEditor::HandleSaveInstrument_redirect(Fl_Widget *w, void *data){
    ((InstrumentEditor *) data)->HandleSaveInstrument(w, data);
}

void InstrumentEditor::HandleSaveInstrument(Fl_Widget *w, void *data) {
    LoadSaveInstruments io(instrdata);
    char *filename = fl_file_chooser("Save Instrument", "*.ins", bundle_path);

    if (filename) {
        if (!io.SaveInstrument(program, filename)) {
            fl_message("Error: %s", io.error_message);
        }
    }
}

// ----------------------------------------------------------------------------

void InstrumentEditor::HandleLoadBank_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleLoadBank(w, data);
}

void InstrumentEditor::HandleLoadBank(Fl_Widget *w, void *data) {
    LoadSaveInstruments io(instrdata);
    char *filename = fl_file_chooser("Load Bank", "*.bnk", bundle_path);

    if (filename) {
        if (!io.LoadBank(filename)) {
            fl_message("Error: %s", io.error_message);
            return;
        }
        if (bank_filename) {
            free(bank_filename);
        }
        bank_filename = strdup(filename);
        SendNewPathnameToDSP();
        SendReloadFromFileToDSP();
        DrawProgram();
        dirty = false;
    }
}

// ----------------------------------------------------------------------------

void InstrumentEditor::HandleSaveBank_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleSaveBank(w, data);
}

void InstrumentEditor::HandleSaveBank(Fl_Widget *w, void *data) {
    LoadSaveInstruments io(instrdata);
    char *filename = fl_file_chooser("Save Bank", "*.bnk", bank_filename);

    if (filename) {
        if (!io.SaveBank(filename)) {
            fl_message("Error: %s", io.error_message);
            return;
        }
        if (bank_filename) {
            free(bank_filename);
        }
        bank_filename = strdup(filename);
        dirty = false;
        SendNewPathnameToDSP();
    }
}

// ----------------------------------------------------------------------------

void InstrumentEditor::HandleExportList_redirect(Fl_Widget *w, void *data) {
    ((InstrumentEditor *) data)->HandleExportList(w, data);
}

void InstrumentEditor::HandleExportList(Fl_Widget *w, void *data) {
    LoadSaveInstruments io(instrdata);
    char *filename = fl_file_chooser("Export Instrument List", "*.txt",
                                                                bundle_path);
    if (filename) {
        if (!io.ExportInstrumentList(filename)) {
            fl_message("Error: %s", io.error_message);
        }
    }
}

// ----------------------------------------------------------------------------

bool InstrumentEditor::LoadBank(const char *filename) {
    LoadSaveInstruments io(instrdata);
    if (!io.LoadBank(filename)) {
        fl_message("Loading bank. Error: %s\n", io.error_message);
        return false;
    }
    if (bank_filename) {
        free(bank_filename);
    }
    bank_filename = strdup(filename);
    DrawProgram();
    return true;
}

bool InstrumentEditor::SaveBank(void) {
    LoadSaveInstruments io(instrdata);
    if (bank_filename) {
        if (!io.SaveBank(bank_filename)) {
            fl_message("Saving bank. Error: %s\n", io.error_message);
            return false;
        }
    } else {
        fl_message("Saving bank failed. No filename\n");
        return false;
    }
    return true;
}

bool InstrumentEditor::is_dirty() {
    return dirty;
}
