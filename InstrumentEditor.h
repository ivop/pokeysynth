#pragma once
#include "lv2.h"
#include "fltk.h"
#include "UiHelpers.h"

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
    HexLine(int x, int y, const char *l = nullptr);
    HexBox *boxes[64];
    void SetValue(int index, unsigned int value);
    void SetValues(uint8_t *values);
private:
};

class PositionSlider : public Fl_Hor_Slider {
public:
    PositionSlider(int x, int y, const char *l = nullptr);
private:
};

class KeyboardEditor : public Fl_Group {
public:
    KeyboardEditor(int x, int y, int w, int h, HexLine **lines, int nlines);
    int handle(int event);
    void show_cursor(void);
    void hide_cursor(void);
    char last_char;
    int cursorX;
    int cursorY;
private:
    HexLine **lines;
    int nlines;
    bool showCursor;
    bool has_focus;
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

    bool sending_or_receiving;

    Fl_Double_Window *win;

    Fl_Spinner *programSpinner;
    Fl_Input *programName;
    FlatRadioButton *channelsButtons[4];
    FlatRadioButton *clocksButtons[3];

    VolBox *envelopeBoxes[64][16];
    HexLine *volumeValues;
    PositionSlider *susLoopStart;
    PositionSlider *susLoopEnd;
    PositionSlider *envEnd;
    Fl_Spinner *attackSpin;
    Fl_Spinner *decaySpin;
    Fl_Spinner *sustainSpin;
    Fl_Spinner *releaseSpin;

    HexLine *distValues;
    Fl_Button *distButtons[5];

    HexLine *typesLine;
    HexLine *typeValues[8];

    KeyboardEditor *editVolumeValues;
    KeyboardEditor *editDistValues;
    KeyboardEditor *editTypesLine;
    KeyboardEditor *editTypeValues;

    PositionSlider *typesLoopStart;
    PositionSlider *typesLoopEnd;

    Fl_Hor_Value_Slider *typesSpeed;

    Fl_Hor_Value_Slider *filterDetune;
    Fl_Hor_Value_Slider *filterVol2;
    Fl_Check_Button *filterTranspose;

    Fl_Hor_Value_Slider *benderRange;
    Fl_Hor_Value_Slider *modwheelDepth;
    Fl_Hor_Value_Slider *modwheelSpeed;

    Fl_Progress *progressBar;

    static void HandleProgramSpinner_redirect(Fl_Widget *w, void *data);
    void HandleProgramSpinner(Fl_Spinner *w, void *data);
    static void HandleProgramName_redirect(Fl_Widget *w, void *data);
    void HandleProgramName(Fl_Widget *w, void *data);

    static void HandleChannelsRadios_redirect(Fl_Widget *w, void *data);
    void HandleChannelsRadios(Fl_Widget *w, void *data);

    static void HandleClocksRadios_redirect(Fl_Widget *w, void *data);
    void HandleClocksRadios(Fl_Widget *w, void *data);

    static void HandleSusLoopStart_redirect(Fl_Widget *w, void *data);
    void HandleSusLoopStart(Fl_Widget *w, void *data);
    static void HandleSusLoopEnd_redirect(Fl_Widget *w, void *data);
    void HandleSusLoopEnd(Fl_Widget *w, void *data);
    static void HandleEnvEnd_redirect(Fl_Widget *w, void *data);
    void HandleEnvEnd(Fl_Widget *w, void *data);

    static void HandleADSR_redirect(Fl_Widget *w, void *data);
    void HandleADSR(Fl_Widget *w, void *data);

    static void HandleDistButtons_redirect(Fl_Widget *w, void *data);
    void HandleDistButtons(Fl_Widget *w, void *data);

    static void HandleTypesLoopStart_redirect(Fl_Widget *w, void *data);
    void HandleTypesLoopStart(Fl_Widget *w, void *data);
    static void HandleTypesLoopEnd_redirect(Fl_Widget *w, void *data);
    void HandleTypesLoopEnd(Fl_Widget *w, void *data);
    static void HandleTypesLoopSpeed_redirect(Fl_Widget *w, void *data);
    void HandleTypesLoopSpeed(Fl_Widget *w, void *data);

    static void HandleFilterDetune_redirect(Fl_Widget *w, void *data);
    void HandleFilterDetune(Fl_Widget *w, void *data);
    static void HandleFilterVol2_redirect(Fl_Widget *w, void *data);
    void HandleFilterVol2(Fl_Widget *w, void *data);
    static void HandleFilterTranspose_redirect(Fl_Widget *w, void *data);
    void HandleFilterTranspose(Fl_Widget *w, void *data);

    static void HandleBenderRange_redirect(Fl_Widget *w, void *data);
    void HandleBenderRange(Fl_Widget *w, void *data);
    static void HandleModwheelDepth_redirect(Fl_Widget *w, void *data);
    void HandleModwheelDepth(Fl_Widget *w, void *data);
    static void HandleModwheelSpeed_redirect(Fl_Widget *w, void *data);
    void HandleModwheelSpeed(Fl_Widget *w, void *data);

    static void HandleKeyboardEditor_redirect(Fl_Widget *w, void *data);
    void HandleKeyboardEditor(KeyboardEditor *w, void *data);

    static void RequestAllButtonCB_redirect(Fl_Widget *w, void *data);
    void RequestAllButtonCB(Fl_Widget *w, void *data);

    static void RequestCurButtonCB_redirect(Fl_Widget *w, void *data);
    void RequestCurButtonCB(Fl_Widget *w, void *data);
};
