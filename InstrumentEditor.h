#include "fltk.h"

class VolBox : public Fl_Box {
public:
    VolBox(int x, int y, const char *l = nullptr);
    int handle(int event) override;
    void black();
    void white();
    int myxpos();
    int myypos();
    void myposition(int x, int y);
private:
    int xpos, ypos;
};

class HexBox : public Fl_Box {
public:
    HexBox(int x, int y, const char *l = nullptr);
    void normal(void);
    void inverse(void);
private:
};

class HexLine : public Fl_Group {
public:
    HexLine(int x, int y);
private:
};

class InstrumentEditor {
public:
    InstrumentEditor(int width, int starty);
    void DrawProgram(void);
    int program;

private:
    Fl_Double_Window *win;
    Fl_Spinner *programSpinner;
    Fl_Input *programName;
    Fl_Radio_Button *channelsButtons[4];
    Fl_Radio_Button *clocksButtons[3];
    VolBox *envelopeBoxes[64][16];

    static void HandleProgramSpinner_redirect(Fl_Widget *w, void *data);
    void HandleProgramSpinner(Fl_Spinner *w, void *data);
};
