#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "lv2.h"
#include "fltk.h"
#include "uris.h"
#include "PokeyInstrument.h"
#include "InstrumentEditor.h"

extern struct pokey_instrument instrdata[128];

#if 0
void testCB(Fl_Widget *w, void *data) {
    puts("test");
    /*char *result = */ fl_file_chooser("Choose a file", "*.*", NULL);
}
#endif

// ****************************************************************************
// SEND INSTRUMENT DATA TO DSP
//
void InstrumentEditor::SendInstrumentToDSP(unsigned int num) {
    if (num > 127) return;

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
// VOLUME ENVELOPE BOX WIDGET
//
void VolBoxCB(Fl_Widget *w, void *data) {
    InstrumentEditor *ie = (InstrumentEditor *) data;
    VolBox *vb = (VolBox *) w;
    struct pokey_instrument *p = &instrdata[ie->program];

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
    Fl_Box *b = new Fl_Box(x-128, y, 128, 16, l);
    b->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
    b->labelsize(12);
}

// ****************************************************************************
//
// InstrumentEditor Constructor
//
InstrumentEditor::InstrumentEditor(int width,
                                   int starty,
                                   LV2UI_Write_Function write_function,
                                   LV2UI_Controller controller,
                                   LV2_URID_Map *map) {
    int cury = starty, curx = 0;

    this->write_function = write_function;
    this->controller = controller;
    lv2_atom_forge_init(&forge, map);
    sending_or_receiving = false;

    new Label(0, cury,width, "Instrument Editor");
    cury += 20;

    // ---------- Program number / name

    curx = (width -768) / 2;
    programSpinner = new Fl_Spinner(curx,cury, 64, 24);
    programSpinner->minimum(0);
    programSpinner->maximum(127);
    programSpinner->step(1);
    programSpinner->value(0);
    program = 0;
    programSpinner->textfont(FL_COURIER);

    programSpinner->callback(HandleProgramSpinner_redirect, this);

    programName = new Fl_Input(curx+80, cury, 688, 24);
    programName->textfont(FL_COURIER);
    // todo: add edit callback

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
        for (int c=0; c<4; c++) {
            channelsButtons[c] = new FlatRadioButton(curx+c*192, cury,
                                                            192, 24, t[c]);
            channelsButtons[c]->callback(HandleChannelsRadios_redirect, this);
        }
    };
    channelsGroup->end();
    cury += 24 + 8;

    // ---------- Clocks

    curx = (width - 384) / 2;
    Fl_Group *clocksGroup = new Fl_Group(curx, cury, 384, 24);
    clocksGroup->begin(); {
        const char *t[3] = { "15kHz", "64kHz", "1.8MHz" };
        for (int c=0; c<3; c++) {
            clocksButtons[c] = new FlatRadioButton(curx+c*128, cury,
                                                            128, 24, t[c]);
            clocksButtons[c]->callback(HandleClocksRadios_redirect, this);
        }
    }
    clocksGroup->end();

    cury += 24 + 8;

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

    Fl_Button *adsrButton = new Fl_Button(16, cury+64, 128, 24, "ADSR");
    adsrButton->callback(HandleADSR_redirect, this);

    // ---------- Distortions

    int xx = curx + 64*12 + 16;

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
    }

    // ---------- Volume / Distortion lines

    cury += 16*12;
    volumeValues = new HexLine(curx, cury, "Volume");
    cury += 12;
    distValues = new HexLine(curx, cury, "Distortion");
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

    // ---------- Progress bar, test buttons

    cury += 8;
    curx = 16;
    progressBar = new Fl_Progress(curx, cury, 128, 24);
    progressBar->minimum(0);
    progressBar->maximum(127);
    progressBar->value(0);
    progressBar->color2(fl_rgb_color(0x20,0x40,0x80));
    curx += progressBar->w() + 8;

    const int butwidth = 144;
    Fl_Button *tb;
    tb = new Fl_Button(curx, cury, butwidth, 24, "Request All");
    tb->callback(RequestAllButtonCB_redirect, this);
    curx += butwidth + 8;
    tb = new Fl_Button(curx, cury, butwidth, 24, "Request Current");
    tb->callback(RequestCurButtonCB_redirect, this);
    curx += butwidth + 8;
#if 0
    tb = new Fl_Button(curx, cury, butwidth, 24, "Load Instrument");
    curx += butwidth + 8;
    tb = new Fl_Button(curx, cury, butwidth, 24, "Save Instrument");
    curx += butwidth + 8;
    tb = new Fl_Button(curx, cury, butwidth, 24, "Load Bank");
    curx += butwidth + 8;
    tb = new Fl_Button(curx, cury, butwidth, 24, "Save Bank");
    curx += butwidth + 8;
#endif

    DrawProgram();
}

// ****************************************************************************
// PROGRAM SELECTION SPINNER
//
void InstrumentEditor::HandleProgramSpinner_redirect(Fl_Widget *w, void *data){
    ((InstrumentEditor *) data)->HandleProgramSpinner((Fl_Spinner *)w,data);
}

void InstrumentEditor::HandleProgramSpinner(Fl_Spinner *w, void *data) {
    program = w->value();
    DrawProgram();
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

    int i, pos = 0;

    for (i=0; i<(int)A; i++) {
        p->volume[pos+i] = round(15.0 / A * (i + 1));
    }
    pos += i;

    for (i=0; i<(int)D; i++) {
        p->volume[pos+i] = round(15.0 - ((15.0 - S) / D * (i + 1)));
    }
    pos += i;

    p->sustain_loop_start = p->sustain_loop_end = pos - 1;

    for (i=0; i<(int)R; i++) {
        if ((pos + i) >= 64) break;
        p->volume[pos+i] = round(S - (S / R * (i + 1)));
    }
    pos += i - 1;

    p->release_end = pos;

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
}

