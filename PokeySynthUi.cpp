#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/x.H>

#include "FL/Fl_Exception.h"
#include "FL/Fl_State.h"
#include "FL/Fl_Transform.h"
#include "FL/Fl_Instruction.h"
#include "FL/Fl_Helper.h"
#include "FL/Fl_Flow.h"

#include <X11/Xlib.h>

class PokeySynthUi {
public:
    PokeySynthUi(LV2UI_Write_Function write_function,
                 LV2UI_Controller controller,
                 void *parentWindow);
    void portEvent(uint32_t port_index,
                   uint32_t buffer_size,
                   uint32_t format,
                   const void *buffer);

    Fl_Window *window;
    void *parentWindow;

private:
    LV2UI_Write_Function write_function;
    LV2UI_Controller controller;
};

PokeySynthUi::PokeySynthUi(LV2UI_Write_Function write_function,
                           LV2UI_Controller controller,
                           void *parentWindow) :
    parentWindow(parentWindow),
    write_function(write_function),
    controller(controller) {

    Fl::visual(FL_DOUBLE|FL_INDEX);

    window = new Fl_Double_Window(640,480);
    Fl_Flow *flow = new Fl_Flow(0,0, window->w(), window->h());
    Fl_Box *title = new Fl_Box(0,0,100,48,"PokeySynth");
    title->box(FL_FLAT_BOX);
    title->labelsize(40);
    title->labelfont(FL_BOLD|FL_ITALIC);
    title->labeltype(FL_SHADOW_LABEL);
    flow->rule(title, "=<^");

    Fl_Box *copyright = new Fl_Box(0,0,100,16,
            "Version 0.9.0 -- Copyright © 2025 by Ivo van poorten");
    copyright->box(FL_FLAT_BOX);
    copyright->labelsize(14);
    copyright->labelfont(FL_ITALIC);
    flow->rule(copyright, "=<^");

    Fl_Box *sep1 = new Fl_Box(0,0,1,1);
    sep1->color(FL_BLACK);
    sep1->box(FL_FLAT_BOX);
    flow->rule(sep1, "=<^");

    Fl_Group *flg = new Fl_Group(0,0,512,24);
    flg->begin();
    Fl_Radio_Button *rb1 = new Fl_Radio_Button(0,0,128,24, "Channel 1-4");
    new Fl_Radio_Button(128,0,128,24, "Channel 5-8");
    new Fl_Radio_Button(256,0,128,24, "Channel 9-12");
    new Fl_Radio_Button(384,0,128,24, "Channel 13-16");
    rb1->setonly();
    flg->end();
    flow->rule(flg, "^/<");

    Fl_Box *sep2 = new Fl_Box(0,0,1,1);
    sep2->color(FL_BLACK);
    sep2->box(FL_FLAT_BOX);
    flow->rule(sep2, "=<^");

    Fl_Group *flg2 = new Fl_Group(0,0,512,96);
    flg2->begin();
    new Fl_Box(0,0,128,24, "Voice 1:");
        Fl_Group *v1g = new Fl_Group(128,0,384,24);
        v1g->begin();
            Fl_Radio_Button *v1grb1 = new Fl_Radio_Button(128,0,128,24,"Monophonic");
            new Fl_Radio_Button(256,0,128,24,"Arpeggio Up");
            new Fl_Radio_Button(384,0,128,24,"Arpeggio Down");
            v1grb1->setonly();
        v1g->end();
    new Fl_Box(0,24,128,24, "Voice 2:");
        Fl_Group *v2g = new Fl_Group(128,24,384,24);
        v2g->begin();
            Fl_Radio_Button *v2grb1 = new Fl_Radio_Button(128,24,128,24,"Monophonic");
            new Fl_Radio_Button(256,24,128,24,"Arpeggio Up");
            new Fl_Radio_Button(384,24,128,24,"Arpeggio Down");
            v2grb1->setonly();
        v2g->end();
    new Fl_Box(0,48,128,24, "Voice 3:");
        Fl_Group *v3g = new Fl_Group(128,48,384,24);
        v3g->begin();
            Fl_Radio_Button *v3grb1 = new Fl_Radio_Button(128,48,128,24,"Monophonic");
            new Fl_Radio_Button(256,48,128,24,"Arpeggio Up");
            new Fl_Radio_Button(384,48,128,24,"Arpeggio Down");
            v3grb1->setonly();
        v3g->end();
    new Fl_Box(0,72,128,24, "Voice 4:");
        Fl_Group *v4g = new Fl_Group(128,72,384,24);
        v4g->begin();
            Fl_Radio_Button *v4grb1 = new Fl_Radio_Button(128,72,128,24,"Monophonic");
            new Fl_Radio_Button(256,72,128,24,"Arpeggio Up");
            new Fl_Radio_Button(384,72,128,24,"Arpeggio Down");
            v4grb1->setonly();
        v4g->end();
    flg2->end();
    flow->rule(flg2, "^/<");

    window->resizable(flow);
    window->size_range(640,480);
    window->end();
    window->show();

    Window w = fl_xid(window);

    printf("debug: window = %llx\n", (unsigned long long ) w);
    printf("debug: parentWindow = %llx\n", (unsigned long long) parentWindow);

#ifdef __linux__
    XSync(fl_display, False);
    Fl::check();
    Fl::flush();
    usleep(100000);

    XUnmapWindow(fl_display, w);

    XSync(fl_display, False);
    Fl::check();
    Fl::flush();
    usleep(100000);

    XReparentWindow(fl_display, w, (Window) parentWindow, 0,0);
    XMapWindow(fl_display, w);
    XSync(fl_display, False);
#elif _WIN32
    // windows code goes here, reparent HWND
    #error "WIN32 not implemented yet"
#else
    #error "Unsupported platform"
#endif
}

void PokeySynthUi::portEvent(uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer) {
}

// ****************************************************************************
//
// C API
//
static LV2UI_Handle instantiate(const struct LV2UI_Descriptor *descriptor,
                                const char *plugin_uri,
                                const char *bundle_path,
                                LV2UI_Write_Function write_function,
                                LV2UI_Controller controller,
                                LV2UI_Widget *widget,
                                const LV2_Feature *const *features) {

    if (strcmp (plugin_uri, "https://github.com/ivop/pokeysynth") != 0) {
        return nullptr;
    }

    void* parentWindow = nullptr;
    for (int i = 0; features[i]; ++i) {
        if (strcmp (features[i]->URI, LV2_UI__parent) == 0)
            parentWindow = features[i]->data;
    }

    if (!parentWindow) {
        fprintf(stderr, "Required feature LV2_UI__parent not provided\n");
        return nullptr;
    }

    PokeySynthUi *ui;
    try {
        ui = new PokeySynthUi(write_function, controller, parentWindow);
    }
    catch (std::exception& exc) {
        fprintf(stderr, "UI instantiation failed.\n");
        return nullptr;
    }

    *widget = reinterpret_cast<LV2UI_Widget>(fl_xid(ui->window));

    return (LV2UI_Handle) ui;
}

static void cleanup (LV2UI_Handle ui) {
    PokeySynthUi *psui = static_cast<PokeySynthUi *>(ui);
    if (psui) delete psui;
}

static void port_event(LV2UI_Handle ui,
                       uint32_t port_index,
                       uint32_t buffer_size,
                       uint32_t format,
                       const void *buffer) {
    PokeySynthUi* psui = static_cast<PokeySynthUi *>(ui);
    if (psui) psui->portEvent(port_index, buffer_size, format, buffer);
}

static int ui_idle(LV2UI_Handle ui) {
//    PokeySynthUi *psui = static_cast<PokeySynthUi *>(ui);
    Fl::check();
    Fl::flush();
    return 0;
}

static const void * extension_data (const char *uri) {
    static const LV2UI_Idle_Interface idle = { ui_idle };
    if (strcmp (uri, LV2_UI__idleInterface) == 0) return &idle;
    return nullptr;
}

static const LV2UI_Descriptor ui_descriptor = {
    "https://github.com/ivop/pokeysynth#ui",
    instantiate,
    cleanup,
    port_event,
    extension_data
};

LV2_SYMBOL_EXPORT const LV2UI_Descriptor * 	lv2ui_descriptor (uint32_t index) {
    switch (index) {
        case 0:     return &ui_descriptor;
        default:    return 0;
    }
}
