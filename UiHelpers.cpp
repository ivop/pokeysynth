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

class Label : public Fl_Box {
public:
    Label(int x, int y, int w, int h, const char *label = nullptr)
        : Fl_Box(x,y,w,h,label) {
        box(FL_FLAT_BOX);
        labelsize(14);
    }
    Label(int y, int w, const char *label = nullptr) : Fl_Box(0,y,w,20,label) {
        box(FL_FLAT_BOX);
        labelsize(14);
        labelfont(FL_BOLD);
    }
};

class Separator : public Fl_Box {
public:
    Separator(int y, int w, const char *label=nullptr) : Fl_Box(0,y,w,1,label) {
        color(FL_BLACK);
        box(FL_FLAT_BOX);
    }
};

class ArpSlider : public Fl_Hor_Value_Slider {
public:
    ArpSlider(int x, int y, int w, int h, const char *l=nullptr) :
        Fl_Hor_Value_Slider(x, y, w, h, l) {
        bounds(0,31);
        precision(0);
        step(1);
    }
};
