#pragma once
// ****************************************************************************
//
// This file is part of PokeySynth.
//
// Copyright © 2024, 2025, by Ivo van Poorten
//
// Licensed under the terms of the General Public License, version 2.
// See the LICENSE file in the root of the prohect directory for the full text.
//
// ****************************************************************************

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

class MouseWheelSlider : public Fl_Hor_Value_Slider {
public:
    MouseWheelSlider(int x, int y, int w, int h, const char *l=nullptr) :
        Fl_Hor_Value_Slider(x, y, w, h, l) {
        bounds(0,31);
        precision(0);
        step(1);
        has_focus = false;
    }
    int handle(int event) override {
        if (event == FL_ENTER) {
            this->take_focus();
            has_focus = true;
            return 1;
        }
        if (event == FL_LEAVE) {
            has_focus = false;
            return 1;
        }
        if (event == FL_MOUSEWHEEL && has_focus) {
            int v = this->value();
            v -= Fl::event_dy();
            if (v < this->minimum()) v = this->minimum();
            if (v > this->maximum()) v = this->maximum();
            this->value(v);
            this->do_callback();
            return 1;
        }
        return Fl_Hor_Value_Slider::handle(event);
    }
private:
    bool has_focus;
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

class InputField : public Fl_Input {
public:
    InputField(int x, int y, int w, int h, const char *l=nullptr)
        : Fl_Input(x,y,w,h,l) {
    }
    int handle(int event) override {
        if (event == FL_ENTER) {
            this->take_focus();
        }
        return Fl_Input::handle(event);
    }
private:
};
