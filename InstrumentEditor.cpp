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

    Fl_Box *temp = new Label(16,cury,128,24,"Channels");
    temp->align(FL_ALIGN_RIGHT);

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
}

