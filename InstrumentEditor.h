#pragma once
#include "lv2.h"
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
    HexBox *boxes[64];
    void SetValue(int index, unsigned int value);
    void SetValues(uint8_t *values);
private:
};

class InstrumentEditor {
public:
    InstrumentEditor(int width,
                     int starty,
                     LV2UI_Write_Function write_function,
                     LV2UI_Controller controller,
                     LV2_URID_Map *map);
    void DrawProgram(void);
    unsigned int program;

    void SendInstrumentToDSP(unsigned int num);
    void RequestInstrumentFromDSP(unsigned int num);

private:
    LV2UI_Write_Function write_function;
    LV2UI_Controller controller;
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Frame frame;
    uint8_t atom_buffer[1024*128];

    Fl_Double_Window *win;
    Fl_Spinner *programSpinner;
    Fl_Input *programName;
    Fl_Radio_Button *channelsButtons[4];
    Fl_Radio_Button *clocksButtons[3];
    VolBox *envelopeBoxes[64][16];
    HexLine *volumeValues;
    Fl_Spinner *attackSpin;
    Fl_Spinner *decaySpin;
    Fl_Spinner *sustainSpin;
    Fl_Spinner *releaseSpin;

    static void HandleProgramSpinner_redirect(Fl_Widget *w, void *data);
    void HandleProgramSpinner(Fl_Spinner *w, void *data);

    static void RequestAllButtonCB_redirect(Fl_Widget *w, void *data);
    void RequestAllButtonCB(Fl_Widget *w, void *data);
};
