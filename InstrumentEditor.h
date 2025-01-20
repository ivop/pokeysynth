#include "fltk.h"

class InstrumentEditor {
public:
    InstrumentEditor();
private:
    Fl_Double_Window *win;
    Fl_Spinner *programSpinner;
    Fl_Input *programName;

    int program;

    void DrawProgram(void);

    static void HandleProgramSpinner_redirect(Fl_Widget *w, void *data);
    void HandleProgramSpinner(Fl_Spinner *w, void *data);
};
