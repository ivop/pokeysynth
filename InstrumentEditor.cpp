#include "fltk.h"
#include "PokeyInstrument.h"
#include "InstrumentEditor.h"
#include "UiHelpers.h"

extern struct pokey_instrument (*instr)[128];

InstrumentEditor::InstrumentEditor(void) {
    win = new Fl_Double_Window(800,600,"Instrument Editor");
    int cury = 8;

    new Label(cury,win->w(), "Instrument Editor");
    cury += 20;

    new Separator(cury, win->w());
    cury += 8;

    programSpinner = new Fl_Spinner(16,cury, 64, 24);
    programSpinner->minimum(0);
    programSpinner->maximum(127);
    programSpinner->step(1);
    programSpinner->value(0);
    program = 0;
    programSpinner->textfont(FL_COURIER);

    programSpinner->callback(HandleProgramSpinner_redirect, this);

    programName = new Fl_Input(96, cury, 608, 24);
    programName->textfont(FL_COURIER);

    cury += 32;

    Fl_Group *channelsGroup = new Fl_Group(64, cury, 768, 24);
    channelsGroup->begin(); {
        const char *t[4] = { "8-bit Channel", "2CH Linked", "2CH Filter", "4CH Linked + Filter" };
        for (int c=0; c<4; c++) {
            channelsButtons[c] = new Fl_Radio_Button(16+c*192,cury,192,24,t[c]);
        }
    };
    channelsGroup->end();

    DrawProgram();

    win->set_modal();
    win->show();
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
}

