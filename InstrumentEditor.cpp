#include "fltk.h"
#include "PokeyInstrument.h"
#include "InstrumentEditor.h"
#include "UiHelpers.h"

extern struct pokey_instrument (*instr)[128];

void testCB(Fl_Widget *w, void *data) {
    puts("test");
}

// ****************************************************************************

void VolBoxCB(Fl_Widget *w, void *data) {
    InstrumentEditor *ie = (InstrumentEditor *) data;
    VolBox *vb = (VolBox *) w;
    struct pokey_instrument *p = &(*instr)[ie->program];

    p->volume[vb->myxpos()] = 15-vb->myypos();
    ie->DrawProgram();
}

VolBox::VolBox(int x, int y, const char *l) : Fl_Box(x,y,10,10,l) {
    box(FL_FLAT_BOX);
    color(FL_WHITE);
    labelsize(10);
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

HexBox::HexBox(int x, int y, const char *l) : Fl_Box(x,y,10,10,l) {
    box(FL_FLAT_BOX);
    labelsize(10);
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

HexLine::HexLine(int x, int y) : Fl_Group(x,y,64*10,10,nullptr) {
    HexBox *hb = new HexBox(x,y,"F");
    end();
}

// ****************************************************************************

InstrumentEditor::InstrumentEditor(int width, int starty) {
    int cury = starty, curx = 0;

    new Label(0, cury,width, "Instrument Editor");
    cury += 20;

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

    cury += 32;

    curx = (width - 768) / 2;
    Fl_Group *channelsGroup = new Fl_Group(curx, cury, 768, 24);
    channelsGroup->begin(); {
        const char *t[4] = {
            "8-bit Channel",
            "2CH Linked",
            "2CH Filter",
            "4CH Linked + Filter" };
        for (int c=0; c<4; c++) {
            channelsButtons[c] = new Fl_Radio_Button(curx+c*192, cury,
                                                            192, 24, t[c]);
        }
    };
    channelsGroup->end();
    cury += 24 + 8;

    curx = (width - 384) / 2;
    Fl_Group *clocksGroup = new Fl_Group(curx, cury, 384, 24);
    clocksGroup->begin(); {
        const char *t[3] = { "15kHz", "64kHz", "1.8MHz" };
        for (int c=0; c<3; c++) {
            clocksButtons[c] = new Fl_Radio_Button(curx+c*128, cury,
                                                            128, 24, t[c]);
        }
    }
    clocksGroup->end();

    cury += 24 + 8;
    curx = (width - 640) / 2;

    for (int p=0; p<64; p++) {
        for (int q=0; q<16; q++) {
            envelopeBoxes[p][q] = new VolBox(curx+p*10, cury+q*10);
            envelopeBoxes[p][q]->callback(VolBoxCB, this);
            envelopeBoxes[p][q]->myposition(p, q);
        }
    }

    cury += 16*10;

    HexLine *hl = new HexLine(curx, cury);
}

void InstrumentEditor::HandleProgramSpinner_redirect(Fl_Widget *w, void *data){
    ((InstrumentEditor *) data)->HandleProgramSpinner((Fl_Spinner *)w,data);
}

void InstrumentEditor::HandleProgramSpinner(Fl_Spinner *w, void *data) {
    program = w->value();
    DrawProgram();
}

void InstrumentEditor::DrawProgram(void) {
    struct pokey_instrument *p = &(*instr)[program];

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
}

