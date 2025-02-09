#pragma once
#include "lv2.h"
#include "fltk.h"
#include "UiHelpers.h"
#include "PokeyInstrument.h"

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
    int handle(int event) override;
private:
    bool has_focus;
};

class KeyboardEditor : public Fl_Group {
public:
    KeyboardEditor(int x, int y, int w, int h, HexLine **lines, int nlines);
    int handle(int event) override;
    void show_cursor(void);
    void hide_cursor(void);
    void advance_cursor(void);
    char last_char;
    int cursorX;
    int cursorY;
private:
    HexLine **lines;
    int nlines;
    bool showCursor;
    bool has_focus;
};

class MouseWheelSpinner : public Fl_Spinner {
public:
    MouseWheelSpinner(int x, int y, int w, int h, const char *l = nullptr);
    int handle(int event) override;
private:
    bool has_focus;
};

class InstrumentEditor {
public:
    InstrumentEditor(int width,
                     int starty,
                     LV2UI_Write_Function write_function,
                     LV2UI_Controller controller,
                     LV2_URID_Map *map,
                     const char *bundle_path,
                     struct pokey_instrument (&instrdata)[128]);
    void DrawProgram(void);
    unsigned int program;

    void SendInstrumentToDSP(unsigned int num);
    void RequestInstrumentFromDSP(unsigned int num);

    bool LoadBank(const char *filename);
    bool SaveBank(void);

    bool is_dirty(void);

    void SendReloadFromFileToDSP(void);
    void SendNewPathnameToDSP(void);

    struct pokey_instrument (&instrdata)[128];

private:
    LV2UI_Write_Function write_function;
    LV2UI_Controller controller;
    const char *bundle_path;
    char *bank_filename;
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Frame frame;
    uint8_t atom_buffer[1024*128];

    bool dirty;
    bool program_base_1;
    bool sending_or_receiving;

    Fl_Check_Button *display128Check;

    MouseWheelSpinner *programSpinner;
    InputField *programName;
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
    Fl_Button *adsrButtons[3];

    HexLine *distValues;
    Fl_Button *distButtons[5];

    HexLine *typesLine;
    HexLine *typeValues[8];

    KeyboardEditor *editVolumeValues;
    KeyboardEditor *editDistValues;
    KeyboardEditor *editTypesLine;
    KeyboardEditor *editTypeValues;

    FlatRadioButton *chordType[6];
    FlatRadioButton *chordAdd[7];
    FlatRadioButton *chordInversion[4];
    FlatRadioButton *chordArpDirection[2];

    PositionSlider *typesLoopStart;
    PositionSlider *typesLoopEnd;

    MouseWheelSlider *typesSpeed;

    MouseWheelSlider *filterDetune;
    MouseWheelSlider *filterVol2;
    Fl_Check_Button *filterTranspose;

    MouseWheelSlider *benderRange;
    MouseWheelSlider *modwheelDepth;
    MouseWheelSlider *modwheelSpeed;

    //Fl_Progress *progressBar;

    static void HandleDisplay128_redirect(Fl_Widget *w, void *data);
    void HandleDisplay128(Fl_Widget *w, void *data);

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

    static void HandleChordsButton_redirect(Fl_Widget *w, void *data);
    void HandleChordsButton(Fl_Widget *w, void *data);

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

    //static void RequestAllButtonCB_redirect(Fl_Widget *w, void *data);
    //void RequestAllButtonCB(Fl_Widget *w, void *data);
    //static void RequestCurButtonCB_redirect(Fl_Widget *w, void *data);
    //void RequestCurButtonCB(Fl_Widget *w, void *data);

    static void HandleExportList_redirect(Fl_Widget *w, void *data);
    void HandleExportList(Fl_Widget *w, void *data);

    static void HandleLoadInstrument_redirect(Fl_Widget *w, void *data);
    void HandleLoadInstrument(Fl_Widget *w, void *data);
    static void HandleSaveInstrument_redirect(Fl_Widget *w, void *data);
    void HandleSaveInstrument(Fl_Widget *w, void *data);

    static void HandleLoadBank_redirect(Fl_Widget *w, void *data);
    void HandleLoadBank(Fl_Widget *w, void *data);
    static void HandleSaveBank_redirect(Fl_Widget *w, void *data);
    void HandleSaveBank(Fl_Widget *w, void *data);

    static void HandleVolumeClear_redirect(Fl_Widget *w, void *data);
    void HandleVolumeClear(Fl_Widget *w, void *data);
    static void HandleTypesClear_redirect(Fl_Widget *w, void *data);
    void HandleTypesClear(Fl_Widget *w, void *data);
    static void HandleBottomClear_redirect(Fl_Widget *w, void *data);
    void HandleBottomClear(Fl_Widget *w, void *data);
    static void HandleClearInstrument_redirect(Fl_Widget *w, void *data);
    void HandleClearInstrument(Fl_Widget *w, void *data);
};
