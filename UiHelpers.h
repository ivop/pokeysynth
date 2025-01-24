#pragma once
#include "fltk.h"

class Label : public Fl_Box {
public:
    Label(int x, int y, int w, int h, const char *label = nullptr)
        : Fl_Box(x,y,w,h,label) {
        labelsize(14);
    }
    Label(int x, int y, int w, const char *label = nullptr) : Fl_Box(x,y,w,20,label) {
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

class FlatRadioButton : public Fl_Radio_Button {
public:
    FlatRadioButton(int x, int y, int w, int h, const char *l=nullptr) :
        Fl_Radio_Button(x,y,w-1,h-1,l) {
        box(FL_FLAT_BOX);
        labelsize(labelsize()-1);
    }
    void draw(void) {
        if (value()) {
            selection_color(FL_BLACK);
        } else {
            color(fl_rgb_color(0xa0, 0xa0, 0xa0));
        }
        Fl_Radio_Button::draw();
    }
};
