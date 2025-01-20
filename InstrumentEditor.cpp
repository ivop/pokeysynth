#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Table.H>
#include <FL/x.H>

#include "PokeyInstrument.h"
#include "InstrumentEditor.h"
#include "UiHelpers.h"

extern struct pokey_instrument (*instr)[128];

InstrumentEditor::InstrumentEditor(void) {
    win = new Fl_Double_Window(800,600,"Instrument Editor");
    win->set_modal();
    win->show();
}
