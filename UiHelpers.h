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
    void draw(void) override {
        if (value()) {
            selection_color(FL_BLACK);
        } else {
            color(fl_rgb_color(0xa0, 0xa0, 0xa0));
        }
        Fl_Radio_Button::draw();
    }
};

class HorizontalLayout : public Fl_Group {
public:
    HorizontalLayout(int x, int y, int w, int h) : Fl_Group(x,y,w,h) {
    }
    void draw(void) {
        int n = children();
        int nvis = 0;
        for (int c=0; c<n; c++) {
            if (child(c)->visible()) {
                nvis++;
            }
        }
        float ww = (float) w() / nvis;
        for (int c=0; c<n; c++) {
            if (child(c)->visible()) {
                child(c)->resize(x()+c*ww, y(), ww, h());
            }
        }
        Fl_Group::draw();
    }
};

class VerticalLayout : public Fl_Group {
public:
    VerticalLayout(int x, int y, int w, int h) : Fl_Group(x,y,w,h) {
    }
    void draw(void) {
        int n = children();
        int nvis = 0;
        for (int c=0; c<n; c++) {
            if (child(c)->visible()) {
                nvis++;
            }
        }
        float hh = (float) h() / nvis;
        for (int c=0; c<n; c++) {
            if (child(c)->visible()) {
                child(c)->resize(x(), y()+c*hh, w(), hh);
            }
        }
        Fl_Group::draw();
    }
};
